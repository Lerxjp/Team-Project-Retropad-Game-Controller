/*
 * ArcadeStick.c
 *
 * Created: 16/03/2022 8:58:10 AM
 */ 

/* Defines and Includes */
#define F_CPU 1000000L // ATmega328P clock cycles every second 
// ( internal clock is 8MHZ and the prescaler is set to 8) so the clock speed. 
#define BAUD 9600 // expected baud rate for pc serial communication
#define UBRR 12 // USART Baud Rate Register tells the internal clock to count this number before sending or reading
// Normal speed hardware samples each bit 16 times so UBRR = (Fcpu/(16xBaud))-1 ~ 6
// Double speed hardware samples each bit 8 times UBRR = (Fcpu/(8xBaud))-1 ~ 12 (error of 0.16%)
#define joyModeAddress 0x00 
// Following spaced by 4 bits each to fit into a 16-bit word length or 2-Bytes
#define redAddress (uint16_t*) 0x04
#define greenAddress (uint16_t*) 0x08
#define blueAddress (uint16_t*) 0x0C
#define audioAddress (uint16_t*) 0x10

// ATmega328P Libraries
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

// Standard C libraies
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <util/delay.h>

/* global variables */
uint8_t joyMode;
uint8_t modeSwitched;
uint16_t* red;
uint16_t* green;
uint16_t* blue;
uint16_t volume;

/* UART functions*/
/**
* @brief 
* @retval
**/
void UART_init(uint16_t ubrr)
{
	UBRR0L = (uint8_t)(ubrr & 0xFF);
	UBRR0H = (uint8_t)(ubrr >> 8);

	UCSR0B |= (1 << RXEN0) | (1 << TXEN0) | (1<<RXCIE0);
	UCSR0A |= (1 << U2X0);
}

/**
* @brief 
* @retval
**/
void UART_putc(uint8_t data)
{
	while(!(UCSR0A & (1 << UDRE0))){};
	UDR0 = data;
}

/**
* @brief 
* @retval
**/
void UART_puts(char* s)
{
	while(*s > 0)
	{
		UART_putc(*s++);
	}
}

/**
* @brief 
* @retval
**/
char UART_getc(void)
{
	while(!(UCSR0A & (1 << RXC0))){};
	return UDR0;
}

/* SPI functions*/
/**
* @brief 
* @retval
**/
void spi_init(void)
{
	DDRB |= (1<<PINB2) | (1<<PINB3) | (1<<PINB5);
	DDRC |= (1<<PINC5);
	SPCR |= (1<<SPE)|(1<<MSTR)|(0<<CPOL)|(0<<CPHA)|(1<<SPR0)|(1<<SPR1)|(0<<SPI2X);

}

/**
* @brief 
* @retval
**/
void spi_transmit_buttons(uint8_t registerValue)
{
	PORTB &= ~(1<<PINB2);
	SPDR=0x00;
	while(!(SPSR&(1<<SPIF))){};
	SPDR=registerValue;
	while(!(SPSR&(1<<SPIF))){};
	PORTB |= (1<<PINB2);
	_delay_ms(2);
	
	PORTB &= ~(1<<PINB2);
	SPDR=0xFF;
	while(!(SPSR&(1<<SPIF))){};
	SPDR=0x00;
	while(!(SPSR&(1<<SPIF))){};
	PORTB |= (1<<PINB2);
	_delay_ms(2);
}

/**
* @brief 
* @retval
**/
void spi_transmit_dpad(uint8_t registerValue)
{
	PORTB &= ~(1<<PINB2);
	SPDR=0x07;
	while(!(SPSR&(1<<SPIF))){};
	SPDR=registerValue;
	while(!(SPSR&(1<<SPIF))){};
	PORTB |= (1<<PINB2);
	_delay_ms(2);
	
	PORTB &= ~(1<<PINB2);
	SPDR=0xFF;
	while(!(SPSR&(1<<SPIF))){};
	SPDR=0x00;
	while(!(SPSR&(1<<SPIF))){};
	PORTB |= (1<<PINB2);
	_delay_ms(2);
}

/**
* @brief 
* @retval
**/
void spi_transmit_audio(uint8_t volume)
{
	PORTC &= ~(1<<PINC5);
	SPDR=0x00;
	while(!(SPSR&(1<<SPIF))){};
	SPDR=volume;
	while(!(SPSR&(1<<SPIF))){};
	PORTC |= (1<<PINC5);
	_delay_ms(2);
}

/**
* @brief 
* @retval
**/
void spi_transmit_joystick(uint8_t registerValueX, uint8_t registerValueY)
{
	PORTB &= ~(1<<PINB2);
	SPDR=0x02;
	while(!(SPSR&(1<<SPIF))){};
	SPDR=registerValueX;
	while(!(SPSR&(1<<SPIF))){};
	PORTB |= (1<<PINB2);
	_delay_ms(2);
	
	PORTB &= ~(1<<PINB2);
	SPDR=0x03;
	while(!(SPSR&(1<<SPIF))){};
	SPDR=registerValueY;
	while(!(SPSR&(1<<SPIF))){};
	PORTB |= (1<<PINB2);
	_delay_ms(2);
	
	PORTB &= ~(1<<PINB2);
	SPDR=0xFF;
	while(!(SPSR&(1<<SPIF))){};
	SPDR=0x00;
	while(!(SPSR&(1<<SPIF))){};
	PORTB |= (1<<PINB2);
	_delay_ms(2);
}

/**
* @brief 
* @retval
**/
uint16_t joy_mode(void)
{
	uint16_t readJoy;
	readJoy = eeprom_read_word(joyModeAddress);
	if ((readJoy != 0) & (readJoy != 1))
	{
		eeprom_write_word(joyModeAddress, 0);	
		readJoy = 0;
	}
	return readJoy;
}

/* ISR functions*/
/**
* @brief 
* @retval
**/
ISR(USART_RX_vect)
{
	char redLed[3];
	char greenLed[3];
	char blueLed[3];
	char audioLevel[4];
	
	char data = UDR0;
	
	if (data == 'j')
	{
		eeprom_write_word(joyModeAddress, 1);
		joyMode = 1;
		modeSwitched = 1;
	}
	else if (data == 'd')
	{
		eeprom_write_word(joyModeAddress, 0);
		joyMode = 0;
		modeSwitched = 1;
	}
	else if (data == 'V')
	{
		audioLevel[0] = UART_getc();
		_delay_ms(1);
		audioLevel[1] = UART_getc();
		_delay_ms(1);
		audioLevel[2] = UART_getc();
		_delay_ms(1);
		audioLevel[3] = '\0';
		volume = (uint16_t) strtol(audioLevel,'\0',10);
	}
	
	else if (data == '#')
	{
		redLed[0] = UART_getc();
		_delay_ms(1);
		redLed[1] = UART_getc();
		_delay_ms(1);
		redLed[2] = '\0';
		
		greenLed[0] = UART_getc();
		_delay_ms(1);
		greenLed[1] = UART_getc();
		_delay_ms(1);
		greenLed[2] = '\0';
		
		blueLed[0] = UART_getc();
		_delay_ms(1);
		blueLed[1] = UART_getc();
		_delay_ms(1);
		blueLed[2] = '\0';
		
		red[0] = (uint16_t) strtol(redLed, '\0', 16) ^ 0xFF;
		green[0] = (uint16_t) strtol(greenLed, '\0', 16) ^ 0xFF;
		blue[0] = (uint16_t) strtol(blueLed, '\0', 16) ^ 0xFF;
		
	}
}

/* Button and Joystick Functions*/
/**
* @brief 
* @retval
**/
uint8_t button_check(uint8_t buttonRegister)
{
	//Turn on/off button 0
	if(!(PINC & (1<<PINC3)))
	{
		_delay_ms(1);
		if(!(PINC & (1<<PINC3)))
		{
			buttonRegister |= 1UL << 0;
		}
	}
	else
	{
		buttonRegister &= ~(1UL << 0);
	}
			
	//Turn on/off button 1
	if(!(PINC & (1<<PINC2)))
	{
		_delay_ms(1);
		if(!(PINC & (1<<PINC2)))
		{
			buttonRegister |= 1UL << 6;
		}
	}
	else
	{
		buttonRegister &= ~(1UL << 6);
	}
			
	//Turn on/off button 2
	if(!(PINC & (1<<PINC1)))
	{
		_delay_ms(1);
		if(!(PINC & (1<<PINC1)))
		{
			buttonRegister |= 1UL << 1;
		}
	}
	else
	{
		buttonRegister &= ~(1UL << 1);
	}
			
	//Turn on/off button 3
	if(!(PINC & (1<<PINC0)))
	{
		_delay_ms(1);
		if(!(PINC & (1<<PINC0)))
		{
			buttonRegister |= 1UL << 5;
		}
	}
	else
	{
		buttonRegister &= ~(1UL << 5);
	}
			
	//Turn on/off button 4
	if(!(PIND & (1<<PIND5)))
	{
		_delay_ms(1);
		if(!(PIND & (1<<PIND5)))
		{
			buttonRegister |= 1UL << 2;
		}
	}
	else
	{
		buttonRegister &= ~(1UL << 2);
	}
			
	//Turn on/off button 5
	if(!(PIND & (1<<PIND7)))
	{
		_delay_ms(1);
		if(!(PIND & (1<<PIND7)))
		{
			buttonRegister |= 1UL << 4;
		}
	}
	else
	{
		buttonRegister &= ~(1UL << 4);
	}
			
	//Turn on/off button 6
	if(!(PINB & (1<<PINB0)))
	{
		_delay_ms(1);
		if(!(PINB & (1<<PINB0)))
		{
			buttonRegister |= 1UL << 3;
		}
	}
	else
	{
		buttonRegister &= ~(1UL << 3);
	}
	spi_transmit_buttons(buttonRegister);
	return buttonRegister;
}

/**
* @brief 
* @retval
**/
uint8_t dpad_check(uint8_t dpadRegister)
{
	//Turn on/off d-pad up
	if(!(PIND & (1<<PIND2)))
	{
		_delay_ms(1);
		if(!(PIND & (1<<PIND2)))
		{
			dpadRegister |= 1UL << 1;
		}
	}
	else
	{
		dpadRegister &= ~(1UL << 1);
	}
			
	//Turn on/off d-pad down
	if(!(PINB & (1<<PINB7)))
	{
		_delay_ms(1);
		if(!(PINB & (1<<PINB7)))
		{
			dpadRegister |= 1UL << 2;
		}
	}
	else
	{
		dpadRegister &= ~(1UL << 2);
	}
			
	//Turn on/off d-pad left
	if(!(PIND & (1<<PIND4)))
	{
		_delay_ms(1);
		if(!(PIND & (1<<PIND4)))
		{
			dpadRegister |= 1UL << 0;
		}
	}
	else
	{
		dpadRegister &= ~(1UL << 0);
	}
			
	//Turn on/off d-pad right
	if(!(PINB & (1<<PINB6)))
	{
		_delay_ms(1);
		if(!(PINB & (1<<PINB6)))
		{
			dpadRegister |= 1UL << 3;
		}
	}
	else
	{
		dpadRegister &= ~(1UL << 3);
	}
	spi_transmit_dpad(dpadRegister);
	UART_putc('d');
	return dpadRegister;	
}

/**
* @brief 
* @retval
**/
uint8_t * joystick_check(uint8_t * joyRegister)
{
	//Joystick Y-Axis
	if(!(PIND & (1<<PIND4)))
	{
		_delay_ms(1);
		if(!(PIND & (1<<PIND4)))
		{
			joyRegister[1] = 0b01111111;
		}
	}
	else if(!(PIND & (1<<PIND2)))
	{
		_delay_ms(1);
		if(!(PIND & (1<<PIND2)))
		{
			joyRegister[1] = 0b10000001;
		}
	}
	else
	{
		joyRegister[1] = 0b00000000;
	}
	
			
	//Joystick X-Axis
	if(!(PINB & (1<<PINB7)))
	{
		_delay_ms(1);
		if(!(PINB & (1<<PINB7)))
		{
			joyRegister[0] = 0b01111111;
		}
	}
	else if(!(PINB & (1<<PINB6)))
	{
		_delay_ms(1);
		if(!(PINB & (1<<PINB6)))
		{
			joyRegister[0] = 0b10000001;
		}
	}
	else
	{
		joyRegister[0] = 0b00000000;
	}
	spi_transmit_joystick(joyRegister[0], joyRegister[1]);	
	UART_putc('j');
	return joyRegister;
}

/* Setup section*/
/**
* @brief 
* @retval
**/
void setup(void)
{
	//Output pin
	DDRB |= (1<<6) | (1<<7) | (1<<0);
	DDRC |= (1<<3) | (1<<2) | (1<<1) | (1<<0);
	DDRD |= (1<<5) | (1<<7) | (1<<4) | (1<<2);
	PORTB = (1<<PINB6) | (1<<PINB7) | (1<<PINB0); 
	PORTC = (1<<PINC3) | (1<<PINC2) | (1<<PINC1) | (1<<PINC0);
	PORTD = (1<<PIND5) | (1<<PIND7) | (1<<PIND4) | (1<<PIND2);
		
	/*red*/
	DDRD |= (1<<6);
	OCR0A = 0xFF; //red
	OCR0B = 0xFF;
	TCCR0A = (1<<COM0A1) | (1<<WGM01) | (1<<WGM00);
	TCCR0B = (1<<CS00);
	
	/*blue*/
	DDRB |= (1<<1);
	OCR1A = 0xFF; //blue
	ICR1 = 0xFF;
	TCCR1A = (1<<COM1A1) | (1<<WGM11);
	TCCR1B = (1<<WGM12) | (1<<WGM13) | (1<<CS10);
	
	/*green*/
	DDRD |= (1<<3);
	OCR2B = 0xFF; //green
	OCR2A = 0xFF;
	TCCR2A = (1<<COM2B1) | (1<<WGM21) | (1<<WGM20);
	TCCR2B = (1<<CS20);
	
	sei();
}

/* main */
int main(void)
{
	UART_init(UBRR);
	spi_init();
	setup();

	uint8_t buttonRegister;
	uint8_t dpadRegister;
	uint8_t * joyRegister;
	uint16_t currentVolume;
	
	//Analogue or Digital mode
	//1=Analogue 0=D-Pad	
	joyMode = joy_mode();
	
	modeSwitched = 0;
	currentVolume = 0;

	//Variable containing current button inputs
	buttonRegister = 0b0000000;
	//variable containing current d-pad inputs
	dpadRegister = 0b000;
	
	//Analogue mode variables
	joyRegister = (uint8_t*) calloc(2,sizeof(uint8_t)+2);
	red = (uint16_t*) calloc(2,sizeof(uint16_t));
	green = (uint16_t*) calloc(2,sizeof(uint16_t));
	blue = (uint16_t*) calloc(2,sizeof(uint16_t));
	joyRegister[0] = 0b00000000;
	joyRegister[1] = 0b00000000;

	red[0] = eeprom_read_word(redAddress);
	green[0] = eeprom_read_word(greenAddress);
	blue[0] = eeprom_read_word(blueAddress);
	volume = eeprom_read_word(audioAddress);
	while (1) 
    {
		buttonRegister = button_check(buttonRegister);		
		if(joyMode==0)
		{
			dpadRegister = dpad_check(dpadRegister);
		}
		if (modeSwitched ==  1)
		{
			spi_transmit_dpad(0x00);
			spi_transmit_joystick(0x00, 0x00);
			modeSwitched = 0;
		}
		else if(joyMode==1)
		{
			joyRegister = joystick_check(joyRegister);
		}
		
	    if (OCR0A != red[0])
	    {
		    OCR0A = red[0];
			eeprom_write_word(redAddress, red[0]);	
	    }
	    if (OCR1A != blue[0])
	    {
		    OCR1A = blue[0];
			eeprom_write_word(blueAddress, blue[0]);	
	    }
	    if (OCR2B != green[0])
	    {
		    OCR2B = green[0];
			eeprom_write_word(greenAddress, green[0]);	
	    }	
		if (currentVolume != volume)
		{
			currentVolume = volume;
			spi_transmit_audio(volume);
			eeprom_write_word(audioAddress,currentVolume);
		}
	}
}

