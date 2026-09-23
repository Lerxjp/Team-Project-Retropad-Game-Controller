import tkinter as tk
from tkinter import colorchooser
import serial

# Default COM port; adjust as required.
SERIAL_PORT = 'COM3'
BAUD_RATE = 9600

class ArcadeControllerGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Arcade Controller Interface")
        
        try:
            self.device = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)
        except serial.SerialException as e:
            print(f"Serial port error: {e}")
            self.device = None

        self.setup_ui()
        
        # Local state tracking for parameters controlled by PC
        self.current_mode = "Unknown"
        self.current_vol = 0
        self.current_rgb = "#000000"
        
        # Begin 10Hz (100ms) update loop to satisfy >2Hz requirement
        self.root.after(100, self.update_telemetry)

    def setup_ui(self):
        # Mode Display & Control
        mode_frame = tk.LabelFrame(self.root, text="Emulation Mode")
        mode_frame.pack(fill="x", padx=10, pady=5)
        
        self.mode_label = tk.Label(mode_frame, text="Current Mode: Unknown", font=("Arial", 12, "bold"))
        self.mode_label.pack(pady=5)
        
        tk.Button(mode_frame, text="Set Joystick Mode", command=lambda: self.send_cmd('j')).pack(side="left", padx=5, pady=5)
        tk.Button(mode_frame, text="Set D-Pad Mode", command=lambda: self.send_cmd('d')).pack(side="right", padx=5, pady=5)

        # 2. Joystick Position Display
        joy_frame = tk.LabelFrame(self.root, text="Joystick Position")
        joy_frame.pack(fill="x", padx=10, pady=5)
        self.joy_pos_label = tk.Label(joy_frame, text="Center", font=("Arial", 12))
        self.joy_pos_label.pack(pady=5)

        # 3. Button Status Display
        btn_frame = tk.LabelFrame(self.root, text="Button Status")
        btn_frame.pack(fill="x", padx=10, pady=5)
        self.button_indicators = []
        for i in range(7):
            # Icons represented as colored frames acting as visual indicators
            indicator = tk.Frame(btn_frame, width=30, height=30, bg="grey", relief="raised", bd=2)
            indicator.grid(row=0, column=i, padx=5, pady=5)
            tk.Label(btn_frame, text=f"B{i}").grid(row=1, column=i)
            self.button_indicators.append(indicator)

        # 4. Volume Control & Display
        vol_frame = tk.LabelFrame(self.root, text="Amplifier Volume")
        vol_frame.pack(fill="x", padx=10, pady=5)
        self.vol_label = tk.Label(vol_frame, text="Current Volume: 0")
        self.vol_label.pack()
        
        self.vol_slider = tk.Scale(vol_frame, from_=0, to=255, orient='horizontal', command=self.send_volume)
        self.vol_slider.pack(fill="x", padx=5, pady=5)

        # 5. RGB LED Control & Display
        rgb_frame = tk.LabelFrame(self.root, text="RGB LED")
        rgb_frame.pack(fill="x", padx=10, pady=5)
        self.rgb_label = tk.Label(rgb_frame, text="Current Color: #000000")
        self.rgb_label.pack()
        
        self.color_preview = tk.Frame(rgb_frame, width=50, height=20, bg="#000000", relief="sunken", bd=2)
        self.color_preview.pack(pady=5)
        
        tk.Button(rgb_frame, text="Open Color Picker", command=self.choose_color).pack(pady=5)

    def send_cmd(self, char):
        if self.device:
            self.device.write(char.encode('utf-8'))

    def send_volume(self, val):
        if self.device:
            cmd = f"V{int(val):03d}"
            self.device.write(cmd.encode('utf-8'))

    def choose_color(self):
        color_code = colorchooser.askcolor(title="Choose LED Color")[1]
        if color_code and self.device:
            hex_color = color_code.lstrip('#').upper()
            cmd = f"#{hex_color}"
            self.device.write(cmd.encode('utf-8'))

    def update_telemetry(self):
        if self.device and self.device.in_waiting > 0:
            try:
                line = self.device.readline().decode('utf-8').strip()
                # Parse expected telemetry format: "MODE,JOY_STATE,B0,B1,B2,B3,B4,B5,B6,VOL,HEX_RGB"
                # Example: "j,Up-Left,1,0,0,0,1,0,0,128,#FF00FF"
                self.parse_telemetry(line)
            except Exception as e:
                pass
                
            # main.c compatibility fallback for mode indication:
            # main.c transmits 'd' or 'j' continuously depending on the mode
            try:
                raw = self.device.read_all().decode('utf-8')
                if 'j' in raw:
                    self.mode_label.config(text="Current Mode: Joystick", fg="blue")
                elif 'd' in raw:
                    self.mode_label.config(text="Current Mode: D-Pad", fg="green")
            except:
                pass

        self.root.after(100, self.update_telemetry)

    def parse_telemetry(self, data):
        parts = data.split(',')
        if len(parts) == 11:
            mode, joy, b0, b1, b2, b3, b4, b5, b6, vol, rgb = parts
            
            self.mode_label.config(text=f"Current Mode: {'Joystick' if mode == 'j' else 'D-Pad'}")
            self.joy_pos_label.config(text=joy)
            
            buttons = [b0, b1, b2, b3, b4, b5, b6]
            for i, state in enumerate(buttons):
                color = "green" if state == "1" else "grey"
                self.button_indicators[i].config(bg=color)
                
            self.vol_label.config(text=f"Current Volume: {vol}")
            self.rgb_label.config(text=f"Current Color: {rgb}")
            self.color_preview.config(bg=rgb)

if __name__ == "__main__":
    root = tk.Tk()
    app = ArcadeControllerGUI(root)
    root.mainloop()