import os
import sys
import time
import serial
import threading
from datetime import datetime
import tkinter as tk
import customtkinter as ctk

PORT = "COM8"          
BAUDRATE = 9600        
TIMEOUT = 0.5          

telemetry_data = {
    "time": "00:00:00",
    "date": "00-00-00",
    "cur_temp": 0.0,
    "avg_temp": 0.0,
    "target_temp": 25.0,
    "fire_triggered": False,
    "system_locked": False
}
telemetry_lock = threading.Lock()
running = True

ctk.set_appearance_mode("Dark")
ctk.set_default_color_theme("blue")

def parse_telemetry(line: str):
    """Parses raw @TIME=...|DATE=... packet from the microcontrollers."""
    if not line.startswith("@"):
        return

    global telemetry_data
    try:
        parts = line.strip().split('|')
        temp_data = {}
        for part in parts:
            if '=' not in part:
                continue
            key, val = part.split('=', 1)
            if key == "@TIME":
                temp_data["time"] = val
            elif key == "DATE":
                temp_data["date"] = val
            elif key == "CUR":
                temp_data["cur_temp"] = float(val)
            elif key == "AVG":
                temp_data["avg_temp"] = float(val)
            elif key == "TARG":
                temp_data["target_temp"] = float(val)
            elif key == "FIRE":
                temp_data["fire_triggered"] = (val == '1')
            elif key == "LOCK":
                temp_data["system_locked"] = (val == '1')
        
        with telemetry_lock:
            telemetry_data.update(temp_data)
    except Exception:
        pass  

def serial_rx_thread(port_name: str, baud: int):
    """Background thread to capture incoming serial data."""
    global running
    try:
        ser = serial.Serial(port_name, baud, timeout=TIMEOUT)
    except Exception as e:
        print(f"Serial Connection Error: {e}")
        return

    buffer = ""
    while running:
        try:
            if ser.in_waiting > 0:
                data = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
                buffer += data
                while '\n' in buffer:
                    line, buffer = buffer.split('\n', 1)
                    parse_telemetry(line)
            else:
                time.sleep(0.05)
        except Exception as e:
            print(f"Error in serial reading loop: {e}")
            break
    
    ser.close()

class SmartFactoryDashboard(ctk.CTk):
    def __init__(self):
        super().__init__()

        self.title("SMART FACTORY DISTRIBUTED DASHBOARD")
        self.geometry("1100x750")
        self.minsize(1000, 650)

        self.grid_rowconfigure(0, weight=0)  
        self.grid_rowconfigure(1, weight=1)  
        self.grid_rowconfigure(2, weight=1)  
        self.grid_columnconfigure(0, weight=1) 
        self.grid_columnconfigure(1, weight=1) 

        self.create_header()
        self.create_quadrant_1_fire_system()
        self.create_quadrant_2_electrical()
        self.create_quadrant_3_assembly()
        self.create_quadrant_4_hvac()

        self.update_gui_values()

    def create_header(self):
        """Top Status and Navigation bar."""
        header_frame = ctk.CTkFrame(self, height=70, corner_radius=0, fg_color="#1E1E24")
        header_frame.grid(row=0, column=0, columnspan=2, sticky="nsew", padx=0, pady=(0, 10))
        
        lbl_title = ctk.CTkLabel(
            header_frame, 
            text="🏭 SMART FACTORY CENTRAL OPERATIONAL MONITOR", 
            font=ctk.CTkFont(size=20, weight="bold"),
            text_color="#FFFFFF"
        )
        lbl_title.pack(side="left", padx=20, pady=15)

        self.lbl_sys_time = ctk.CTkLabel(
            header_frame, 
            text="SYS TIME: --:--:--", 
            font=ctk.CTkFont(family="Consolas", size=15),
            text_color="#A5A5A5"
        )
        self.lbl_sys_time.pack(side="right", padx=20, pady=15)

    def create_quadrant_1_fire_system(self):
        """[SECTION 1/4] Real-time Fire & Interlock Safety Monitoring."""
        frame = ctk.CTkFrame(self, corner_radius=12, border_width=2, border_color="#2B2B30")
        frame.grid(row=1, column=0, padx=10, pady=10, sticky="nsew")

        frame.columnconfigure(0, weight=1)
        frame.columnconfigure(1, weight=1)

        title = ctk.CTkLabel(
            frame, text="🔥 SECTION 1: FIRE & THERMAL SAFETY", 
            font=ctk.CTkFont(size=16, weight="bold"), text_color="#FF4C4C"
        )
        title.grid(row=0, column=0, columnspan=2, pady=(15, 10), padx=15, sticky="w")

        self.val_cur_temp = ctk.CTkLabel(frame, text="0.0 °C", font=ctk.CTkFont(size=28, weight="bold"))
        self.val_cur_temp.grid(row=1, column=0, pady=10)
        lbl_cur = ctk.CTkLabel(frame, text="Current Temp", font=ctk.CTkFont(size=12), text_color="#A5A5A5")
        lbl_cur.grid(row=2, column=0, pady=(0, 10))

        self.val_avg_temp = ctk.CTkLabel(frame, text="0.0 °C", font=ctk.CTkFont(size=28, weight="bold"))
        self.val_avg_temp.grid(row=1, column=1, pady=10)
        lbl_avg = ctk.CTkLabel(frame, text="Average Temp (24h)", font=ctk.CTkFont(size=12), text_color="#A5A5A5")
        lbl_avg.grid(row=2, column=1, pady=(0, 10))

        self.lbl_target = ctk.CTkLabel(frame, text="Target Safety Temp: 25.0°C", font=ctk.CTkFont(size=13))
        self.lbl_target.grid(row=3, column=0, columnspan=2, pady=5)

        self.badge_fire = ctk.CTkLabel(
            frame, text="🔥 FIRE DETECTED", font=ctk.CTkFont(size=13, weight="bold"),
            fg_color="#3A1A1A", text_color="#FF4C4C", corner_radius=8, height=30
        )
        self.badge_fire.grid(row=4, column=0, padx=15, pady=15, sticky="ew")

        self.badge_lock = ctk.CTkLabel(
            frame, text="🔒 SAFETY INTERLOCK", font=ctk.CTkFont(size=13, weight="bold"),
            fg_color="#2B2B30", text_color="#A5A5A5", corner_radius=8, height=30
        )
        self.badge_lock.grid(row=4, column=1, padx=15, pady=15, sticky="ew")

    def create_quadrant_2_electrical(self):
        """[SECTION 2/4] Machine Monitoring"""

        frame = ctk.CTkFrame(self, corner_radius=12, fg_color="#18181C")
        frame.grid(row=1, column=1, padx=10, pady=10, sticky="nsew")

        title = ctk.CTkLabel(
            frame,
            text="⚙️ SECTION 2: MACHINE MONITORING",
            font=ctk.CTkFont(size=16, weight="bold"),
            text_color="#FFD700"
        )
        title.pack(anchor="w", padx=15, pady=15)

        ctk.CTkLabel(
            frame,
            text="📈 Machine Vibration : 0.28 g",
            font=ctk.CTkFont(size=15)
        ).pack(anchor="w", padx=25, pady=5)

        ctk.CTkLabel(
            frame,
            text="🌡 Motor Temperature : 41.6 °C",
            font=ctk.CTkFont(size=15)
        ).pack(anchor="w", padx=25, pady=5)

        ctk.CTkLabel(
            frame,
            text="⚡ Motor Current : 5.2 A",
            font=ctk.CTkFont(size=15)
        ).pack(anchor="w", padx=25, pady=5)

        ctk.CTkLabel(
            frame,
            text="🛠 Machine Runtime : 186 hrs",
            font=ctk.CTkFont(size=15)
        ).pack(anchor="w", padx=25, pady=5)

        ctk.CTkLabel(
            frame,
            text="MACHINE STATUS : HEALTHY",
            font=ctk.CTkFont(size=12, weight="bold"),
            fg_color="#1F3A1E",
            text_color="#4CAF50",
            corner_radius=6,
            padx=10,
            pady=5
        ).pack(anchor="w", padx=25, pady=15)

    def create_quadrant_3_assembly(self):
        """[SECTION 3/4] Workforce Statistics"""

        frame = ctk.CTkFrame(self, corner_radius=12, fg_color="#18181C")
        frame.grid(row=2, column=0, padx=10, pady=10, sticky="nsew")

        title = ctk.CTkLabel(
            frame,
            text="👷 SECTION 3: WORKFORCE STATISTICS",
            font=ctk.CTkFont(size=16, weight="bold"),
            text_color="#00D2FF"
        )
        title.pack(anchor="w", padx=15, pady=15)

        ctk.CTkLabel(
            frame,
            text="👤 Present Workers : 18",
            font=ctk.CTkFont(size=15)
        ).pack(anchor="w", padx=25, pady=5)

        ctk.CTkLabel(
            frame,
            text="🦺 Safety Helmets Detected : 18",
            font=ctk.CTkFont(size=15)
        ).pack(anchor="w", padx=25, pady=5)

        ctk.CTkLabel(
            frame,
            text="🚪 Absent Workers : 2",
            font=ctk.CTkFont(size=15)
        ).pack(anchor="w", padx=25, pady=5)

        ctk.CTkLabel(
            frame,
            text="⏱ Shift Duration : 04:36:15",
            font=ctk.CTkFont(size=15)
        ).pack(anchor="w", padx=25, pady=5)

        ctk.CTkLabel(
            frame,
            text="WORKFORCE STATUS : NORMAL",
            font=ctk.CTkFont(size=12, weight="bold"),
            fg_color="#1F3A1E",
            text_color="#4CAF50",
            corner_radius=6,
            padx=10,
            pady=5
        ).pack(anchor="w", padx=25, pady=15)
        
    def create_quadrant_4_hvac(self):
        """[SECTION 4/4] Production Line & Quality Control"""

        frame = ctk.CTkFrame(self, corner_radius=12, fg_color="#18181C")
        frame.grid(row=2, column=1, padx=10, pady=10, sticky="nsew")

        title = ctk.CTkLabel(
            frame,
            text="🏭 SECTION 4: PRODUCTION LINE & QUALITY CONTROL",
            font=ctk.CTkFont(size=16, weight="bold"),
            text_color="#A020F0"
        )
        title.pack(anchor="w", padx=15, pady=15)

        ctk.CTkLabel(
            frame,
            text="📦 Total Products : 2,540",
            font=ctk.CTkFont(size=15)
        ).pack(anchor="w", padx=25, pady=5)

        ctk.CTkLabel(
            frame,
            text="✅ Healthy Products : 2,501",
            font=ctk.CTkFont(size=15)
        ).pack(anchor="w", padx=25, pady=5)

        ctk.CTkLabel(
            frame,
            text="❌ Defective Products : 39",
            font=ctk.CTkFont(size=15)
        ).pack(anchor="w", padx=25, pady=5)

        ctk.CTkLabel(
            frame,
            text="🎯 Quality Rate : 98.46 %",
            font=ctk.CTkFont(size=15)
        ).pack(anchor="w", padx=25, pady=5)

        ctk.CTkLabel(
            frame,
            text="QUALITY STATUS : EXCELLENT",
            font=ctk.CTkFont(size=12, weight="bold"),
            fg_color="#1F3A1E",
            text_color="#4CAF50",
            corner_radius=6,
            padx=10,
            pady=5
        ).pack(anchor="w", padx=25, pady=15)

    def update_gui_values(self):
        """Cyclical UI refresh callback (runs in main thread thread-safely)."""
        global telemetry_data
        
        with telemetry_lock:
            cur = telemetry_data["cur_temp"]
            avg = telemetry_data["avg_temp"]
            targ = telemetry_data["target_temp"]
            fire = telemetry_data["fire_triggered"]
            locked = telemetry_data["system_locked"]
            sys_time = telemetry_data["time"]
            sys_date = telemetry_data["date"]

        self.lbl_sys_time.configure(text=f"SYS DATE: {sys_date}  |  SYS TIME: {sys_time}")

        self.val_cur_temp.configure(text=f"{cur:.1f} °C")
        self.val_avg_temp.configure(text=f"{avg:.1f} °C")
        self.lbl_target.configure(text=f"Target Trigger Threshold: {targ:.1f}°C")

        if fire:
            self.badge_fire.configure(
                text="⚠️ FIRE ALARM ACTIVE", 
                fg_color="#FF4C4C", 
                text_color="#FFFFFF"
            )
        else:
            self.badge_fire.configure(
                text="✔️ SYSTEM SECURE", 
                fg_color="#1F3A1E", 
                text_color="#4CAF50"
            )

        if locked:
            self.badge_lock.configure(
                text="🔒 SHUTDOWN ACTIVATED", 
                fg_color="#FF9900", 
                text_color="#000000"
            )
        else:
            self.badge_lock.configure(
                text="🔓 MOTOR LOOP ACTIVE", 
                fg_color="#2B2B30", 
                text_color="#A5A5A5"
            )

        self.after(150, self.update_gui_values)

if __name__ == "__main__":
    threading.Thread(target=serial_rx_thread, args=(PORT, BAUDRATE), daemon=True).start()

    app = SmartFactoryDashboard()
    try:
        app.mainloop()
    finally:
        running = False  