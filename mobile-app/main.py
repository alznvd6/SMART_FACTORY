import serial
import os
import sys
import time

PORT = 'COM8' 
BAUDRATE = 9600  

def clear_screen():
    os.system('cls' if os.name == 'nt' else 'clear')

def parse_and_display(raw_line):
    try:
        decoded_line = raw_line.decode('utf-8').strip()
        if not decoded_line.startswith('@'):
            return #
        
        data_payload = decoded_line[1:]
        tokens = data_payload.split('|')
        
        # Initialize variables
        temp, avg, fire, lock = "N/A", "N/A", "N/A", "N/A"
        
        for token in tokens:
            if '=' in token:
                key, val = token.split('=', 1)
                if key == "TEMP":
                    temp = val
                elif key == "AVG":
                    avg = val
                elif key == "FIRE":
                    fire = val
                elif key == "LOCK":
                    lock = "!!! LOCKED OUT !!!" if val == "1" else "NORMAL"

        clear_screen()
        print("==================================================")
        print("         PYTHON ENVIRONMENTAL MONITOR APP         ")
        print("==================================================")
        print(f"  [+] Status:         CONNECTED ({PORT})")
        print(f"  [+] Live Temp:      {temp} °C")
        print(f"  [+] 5-Sec Average:  {avg} °C")
        print(f"  [+] Fire Status:    {fire}")
        print(f"  [+] System Lock:    {lock}")
        print("==================================================")
        print(" Press Ctrl+C to safely exit.")
        
    except Exception as e:
        pass

def main():
    print(f"Initializing connection on {PORT}...")
    try:
        ser = serial.Serial(PORT, BAUDRATE, timeout=1)
        ser.reset_input_buffer() 
        clear_screen()
        print(f"Waiting for telemetry data from Proteus on {PORT}...")
        
        while True:
            if ser.in_waiting > 0:
                line = ser.readline()
                if line:
                    parse_and_display(line)
            time.sleep(0.05) 

    except serial.SerialException:
        print(f"\n[Error] Could not open serial port {PORT}.")
        print("Verify that your virtual serial port pair is active and Proteus is running.")
    except KeyboardInterrupt:
        print("\nExiting Python Monitor. Goodbye!")
        sys.exit(0)

if __name__ == "__main__":
    main()