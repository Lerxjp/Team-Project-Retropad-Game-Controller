# Retropad — Custom USB Arcade Controller \& Companion Software

Retropad is a custom-built arcade controller designed to bring an authentic, rugged arcade experience to modern PCs and entertainment systems. Built with standard microswitch arcade hardware, custom PCB electronics, RGB underglow lighting, digitally controlled audio amplification, and an accompanying desktop GUI, Retropad merges classic 80s/90s arcade controls with modern USB HID compatibility.


## Features

### Physical Interface \& Arcade Controls

* **Joystick:** Zippy-style microswitch joystick supporting both 8-way directional input and D-Pad digital mapping.
* **Buttons:** 7 Happ-style arcade pushbuttons with horizontal microswitches:

  * 5 primary action buttons (Green and Purple)
  * 1 Coin simulation button (White)
  * 1 Player 1 Start button (White with icon)
* **Underglow Lighting:** 5 high-brightness RGB LEDs casting light outward from the custom PCB. Unified color control saved directly to non-volatile memory.
* **Enclosure:** Precision-cut custom acrylic enclosure housing a custom-designed PCB.

### Integrated Audio System

* **Pass-through Audio:** Standard 3.5mm TRS stereo line-in connector.
* **Software Volume Control:** Audio routed through an onboard amplifier powering an 8 $\\Omega$ speaker.
* **128 Volume Levels:** Volume (0–127) is controlled exclusively via the desktop software (no hardware knobs/potentiometers) and preserved across power cycles.

### Connectivity \& Protocol

* **USB HID Standard:** Plug-and-play operation as a standard USB game controller on Windows, Linux, macOS, and console systems.
* **Dual Emulation Modes:**

  * **Joystick Emulation Mode:** Maps physical movements to $X$ and $Y$ axis registers (`JOYSTICK\_X` and `JOYSTICK\_Y` values from $-127$ to $127$).
  * **DPad Emulation Mode:** Maps physical movements directly to the standard digital `DPAD` register.
  * Auto-resets opposing registers to neutral/released states upon mode switching.
* **UART Side-Channel:** Dedicated secondary serial interface over USB for real-time telemetry and desktop control without interfering with game inputs.
* **Firmware Platform:** Powered by a Seeeduino XIAO SAMD21 microcontroller running custom USB interface firmware (`playful-turtle-xiao`).

### Desktop Companion GUI Software

* **Real-time Input Monitoring ($\\ge$ 2 Hz refresh rate):**

  * Dynamic visual button status icons that change color when pressed.
  * Real-time joystick position indicator.
  * Current volume level and active RGB LED color display.
  * Current active mode display (Joystick vs. D-Pad).
* **Device Configuration:**

  * Interactive color picker window for single-click RGB lighting customization.
  * Slider control for setting amplifier gain/volume across 128 discrete levels.
  * Toggle switch between Joystick and D-Pad emulation modes.
* **Standalone Operation:** Controller retains all saved configuration profiles (LED color, volume, mode) in persistent memory and functions fully even when the companion app is closed.

### Key Hardware Components

* **Primary Microcontroller:** Microchip ATmega328P (or equivalent onboard MCU)
* **USB Interface Board:** Seeeduino XIAO (SAMD21 Cortex-M0+)
* **Audio Amplifier:** Adjustable gain circuit driving an 8 $\\Omega$ loudspeaker
* **Wiring:** Microswitch COM terminals routed to common ground chain; NO/NC terminals connected to dedicated micro digital input pins.

## System Requirements \& Compatibility

* **Supported OS:** Windows 10/11, Linux, macOS
* **Game Compatibility:** Fully compatible with any standard USB HID Gamepad-supported game or emulator (e.g., arcade games such as *Push-Push Penguin* / *Pengo*).
* **Configuration App Requirements:** Desktop GUI supporting serial port communication and modern UI rendering.

## Usage
* **TO DO**



## Hardware Specification Summary

|Item|Specification|
|-|-|
|**Input Interface**|1x Zippy Microswitch Joystick, 7x Happ Pushbuttons|
|**USB Protocol**|USB HID Gamepad + USB CDC Serial (UART)|
|**Audio Interface**|3.5mm TRS Line In, 8 $\\Omega$ Speaker Out|
|**Volume Control**|Digital gain control, 128 discrete steps ($0-127$)|
|**Lighting**|5x Bright RGB LEDs (Unified color, non-volatile memory)|
|**Telemetry Rate**|$\\ge 2\\text{ Hz}$ GUI update rate|
|**Power Source**|USB Powered ($5\\text{V}$) via Seeeduino XIAO|

## License

This project is open-hardware and open-source under the MIT License. See `LICENSE` for details.

