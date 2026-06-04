import serial
import time

PORT = "COM6"
BAUD_RATE = 9600

print(f"Initializing connection on {PORT}...")

try:
    # Open the serial port matching your STM32 settings (9600, 8N1)
    ser = serial.Serial(
        port=PORT,
        baudrate=BAUD_RATE,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        timeout=1
    )
    print(f"Successfully connected to {PORT}! Waiting for factory line data...\n")
    
    # Initialize our dashboard counters
    total_count = 0
    healthy_count = 0
    defect_count = 0

    while True:
        # Read incoming data up to the newline character (\n) sent by STM32
        raw_data = ser.readline()
        
        if raw_data:
            # Decode binary bytes to a clean text string and strip spaces/newlines
            incoming_msg = raw_data.decode(errors="ignore").strip().lower()
            
            # Match the exact words sent by your HAL_UART_Transmit functions
            if "healthy" in incoming_msg:
                total_count += 1
                healthy_count += 1
                latest_status = "SYSTEM NOMINAL (HEALTHY)"
                
            elif "problem" in incoming_msg:
                total_count += 1
                defect_count += 1
                latest_status = "ALERT: DEFECT SPOTTED (PROBLEM)"
                
            else:
                # Catch-all if any junk initialization bits arrive
                latest_status = f"UNKNOWN DATA RECEIVED: {incoming_msg}"
                continue

            # Print out a clean, live terminal dashboard panel
            print("=========================================")
            print("      LIVE PRODUCTION LINE MONITOR       ")
            print("=========================================")
            print(f" Total Products Scanned : {total_count}")
            print(f" Healthy Units Passed   : {healthy_count}")
            print(f" Defected Units Found   : {defect_count}")
            print(f" Current Status         : [{latest_status}]")
            print("=========================================\n")
            
except serial.SerialException as e:
    print(f"\n[ERROR]: Could not open {PORT}.")
    print("1. Make sure no other program (like another Python instance) is holding the port open.")
    print("2. Ensure your Virtual Serial Port Driver (VSPD) is running and COM10/COM11 are created.")
    print(f"Details: {e}")
except KeyboardInterrupt:
    print("\nMonitoring stopped by operator. Exiting.")