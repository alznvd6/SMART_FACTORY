import serial
import os
from datetime import datetime
import re
import time

PORT = "COM2"
BAUD_RATE = 9600

def trim_string(str_val):
    clean_str = re.sub(r'[\x00-\x1f\x7f-\x9f]', '', str_val)
    return clean_str.strip()

# =========================================================
# مسیرهای پایگاه داده (بر اساس مسیر دقیق سیستم شما)
# =========================================================
BASE_DIR = r"C:\Users\ashka\Desktop\Embdded System PRoject\SMART_FACTORY\infrastructure\database"
WORKER_FILE_PATH = r"C:\Users\ashka\Desktop\Embdded System PRoject\SMART_FACTORY\infrastructure\database\worker_info\worker_ID.txt"
BIRTH_DATE_PATH = r"C:\Users\ashka\Desktop\Embdded System PRoject\SMART_FACTORY\infrastructure\database\worker_info\date\birth_date.txt"

ATTENDANCE_DIR = os.path.join(BASE_DIR, 'worker_info', 'date', 'attendance')
VACATION_DIR = os.path.join(BASE_DIR, 'worker_info', 'date', 'vacation')
ABSENCE_DIR = os.path.join(BASE_DIR, 'worker_info', 'date', 'absence')
REQUEST_DIR = os.path.join(BASE_DIR, 'worker_info', 'date', 'worker_demands')

REPORTS_PATH = os.path.join(BASE_DIR, 'foreman_info', 'reports', 'reports.txt')
DEMANDS_PATH = os.path.join(BASE_DIR, 'foreman_info', 'demands', 'demands.txt')
LIVE_STATUS_PATH = os.path.join(BASE_DIR, 'foreman_info', 'reports', 'live_workers_status.txt')

def get_today():
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S")

def update_worker_file(file_path, new_log_line):
    os.makedirs(os.path.dirname(file_path), exist_ok=True)
    lines = []
    if os.path.exists(file_path):
        with open(file_path, 'r', encoding='utf-8') as f:
            lines = f.read().splitlines()
    count = 0
    if lines and lines[0].isdigit():
        count = int(lines[0])
        logs = lines[1:]
    else:
        logs = lines
    count += 1
    new_content = [str(count)] + logs + [new_log_line]
    with open(file_path, 'w', encoding='utf-8') as f:
        f.write('\n'.join(new_content) + '\n')

# =========================================================
# تابع خواندن داینامیک کارگران از فایل متنی
# =========================================================
workers_db = {}
active_sessions = {}

def load_workers_from_file():
    global workers_db, active_sessions
    workers_db.clear()
    
    try:
        # خواندن اطلاعات پایه و تلفن
        with open(WORKER_FILE_PATH, 'r', encoding='utf-8') as f:
            for line in f:
                parts = line.strip().split(',')
                if len(parts) >= 3:
                    wid = parts[0].strip()
                    full_name = f"{parts[1].strip()} {parts[2].strip()}"
                    phone = parts[3].strip() if len(parts) >= 4 else "N/A"
                    workers_db[wid] = {"name": full_name, "phone": phone, "birth": "N/A"}
        
        # خواندن تاریخ تولد و ادغام آن
        if os.path.exists(BIRTH_DATE_PATH):
            with open(BIRTH_DATE_PATH, 'r', encoding='utf-8') as f:
                for line in f:
                    parts = line.strip().split('-', 1)
                    if len(parts) == 2:
                        wid = parts[0].strip()
                        birth = parts[1].strip()
                        if wid in workers_db:
                            workers_db[wid]['birth'] = birth

        print(f"✅ Loaded {len(workers_db)} workers from database with phone & birth data.")
        
        active_sessions = {
            wid: {'status': 'OUT', 'in_time': 'N/A', 'out_time': 'N/A', 'out_by': 'N/A'}
            for wid in workers_db.keys()
        }
    except Exception as e:
        print(f"❌ Error loading files: {e}")

def update_live_status():
    os.makedirs(os.path.dirname(LIVE_STATUS_PATH), exist_ok=True)
    with open(LIVE_STATUS_PATH, "w", encoding="utf-8") as f:
        f.write("=== SMART FACTORY: LIVE WORKER STATUS ===\n")
        f.write(f"Last Updated: {get_today()}\n\n")
        for wid, info in workers_db.items():
            session = active_sessions.get(wid, {'status': 'OUT', 'out_time': 'N/A', 'out_by': 'N/A'})
            if session['status'] == 'IN':
                f.write(f"🟢 ID: {wid} | Name: {info['name']} | Phone: {info['phone']}\n")
                f.write(f"   Status: IN (Entered at {session['in_time']})\n")
                f.write("-" * 50 + "\n")
            else:
                f.write(f"🔴 ID: {wid} | Name: {info['name']} | Phone: {info['phone']}\n")
                f.write(f"   Status: OUT (Last Out: {session.get('out_time', 'N/A')} | Logged by: {session.get('out_by', 'N/A')})\n")
                f.write("-" * 50 + "\n")

# اجرای توابع اولیه
load_workers_from_file()
update_live_status()

# =========================================================
# توابع ارتباط با میکروکنترلر
# =========================================================
def send_to_mcu(msg):
    try:
        ser.write((msg + "\r\n").encode('utf-8'))
        time.sleep(0.4)
    except Exception as e:
        print(f"Error sending to MCU: {e}")

def lcd_update(line1, line2):
    send_to_mcu(f"@LCD1:{line1}")
    send_to_mcu(f"@LCD2:{line2}")

print(f"Target Database Path: {BASE_DIR}")
print(f"[{PORT}] Initializing Python IoT Server at {BAUD_RATE} baud...")

try:
    ser = serial.Serial(PORT, BAUD_RATE, timeout=1)
    print(f"✅ Successfully connected to {PORT}! Waiting for Smart Factory data...\n")
    
    while True:
        raw_data = ser.readline()
        if raw_data:
            incoming_msg = raw_data.decode(errors="ignore").strip()
            if not incoming_msg: continue
            
            print(f"\n📥 [STM32 COMMAND]: {incoming_msg}")

            # 0. احراز هویت کارگر (درخواست نام از دیتابیس)
            if incoming_msg.startswith("REQ_NAME:"):
                finger_id = incoming_msg.split(":")[1].strip()
                if finger_id in workers_db:
                    send_to_mcu(f"@NAME:{workers_db[finger_id]['name']}")
                    print(f"✅ Auth Success: Sent name '{workers_db[finger_id]['name']}' to MCU.")
                else:
                    send_to_mcu("@MSG:Error! Unknown ID.")
                    lcd_update("Login Failed!", "Unknown ID")
                    print(f"❌ Auth Failed: Unknown ID {finger_id}")

            # 1. ورود کارگر
            elif incoming_msg.startswith("CHECKIN:"):
                finger_id = incoming_msg.split(":")[1].strip()
                if finger_id in workers_db:
                    if active_sessions[finger_id]['status'] == 'IN':
                        print(f"❌ [ATTENDANCE] Worker {finger_id} is already IN! Ignored.")
                        send_to_mcu("@MSG:Error! Worker is already Clocked IN.")
                        lcd_update("Check-in Failed", "Already IN!")
                    else:
                        active_sessions[finger_id]['status'] = 'IN'
                        active_sessions[finger_id]['in_time'] = get_today()
                        update_live_status()
                        file_path = os.path.join(ATTENDANCE_DIR, f"{finger_id}.txt")
                        update_worker_file(file_path, f"IN at: {get_today()}")
                        
                        send_to_mcu(f"@MSG:Check-in OK for {workers_db[finger_id]['name']}")
                        lcd_update("Check-in OK:", workers_db[finger_id]['name'])
                        print(f"✅ [ATTENDANCE] {workers_db[finger_id]['name']} checked IN.")

            # 2. خروج کارگر
            elif incoming_msg.startswith("CHECKOUT:"):
                finger_id = incoming_msg.split(":")[1].strip()
                if finger_id in workers_db:
                    if active_sessions[finger_id]['status'] == 'OUT':
                        send_to_mcu("@MSG:Error! Worker is already Clocked OUT.")
                        lcd_update("Check-out Fail", "Already OUT!")
                        print(f"❌ [ATTENDANCE] Worker {finger_id} is already OUT! Ignored.")
                    else:
                        active_sessions[finger_id]['status'] = 'OUT'
                        active_sessions[finger_id]['out_time'] = get_today()
                        active_sessions[finger_id]['out_by'] = 'Worker (Self)'
                        update_live_status()
                        file_path = os.path.join(ATTENDANCE_DIR, f"{finger_id}.txt")
                        update_worker_file(file_path, f"OUT at: {get_today()} (By: Self)")
                        
                        send_to_mcu(f"@MSG:Check-out OK for {workers_db[finger_id]['name']}")
                        lcd_update("Check-out OK:", workers_db[finger_id]['name'])
                        print(f"✅ [ATTENDANCE] {workers_db[finger_id]['name']} checked OUT.")

            # 3. خروج اجباری توسط مدیر
            elif incoming_msg.startswith("FORCE_OUT:"):
                finger_id = incoming_msg.split(":")[1].strip()
                if finger_id in workers_db:
                    if active_sessions[finger_id]['status'] == 'OUT':
                        send_to_mcu(f"@MSG:Worker {finger_id} is already OUT.")
                        lcd_update("Force-Out Fail", "Already OUT!")
                    else:
                        active_sessions[finger_id]['status'] = 'OUT'
                        active_sessions[finger_id]['out_time'] = get_today()
                        active_sessions[finger_id]['out_by'] = 'Foreman'
                        update_live_status()
                        file_path = os.path.join(ATTENDANCE_DIR, f"{finger_id}.txt")
                        update_worker_file(file_path, f"OUT at: {get_today()} (By: Foreman)")
                        
                        send_to_mcu(f"@MSG:Forced checkout successful for ID {finger_id}.")
                        lcd_update("Forced Out OK:", workers_db[finger_id]['name'])
            # ۴. مرخصی
            elif incoming_msg.startswith("VACATION:"):
                parts = incoming_msg.split(":")
                if len(parts) >= 3:
                    finger_id = parts[1].strip()
                    status = parts[2].strip()
                    file_path = os.path.join(VACATION_DIR, f"{finger_id}.txt")
                    update_worker_file(file_path, f"Date: {get_today()} - Status: {status}")
                    print(f"🌴 [VACATION] Worker {finger_id} vacation is {status}")

            # ۵. وضعیت نامه‌ها
            elif incoming_msg.startswith("LETTER:"):
                parts = incoming_msg.split(":", 2)
                if len(parts) >= 3:
                    finger_id = parts[1].strip()
                    custom_message = parts[2].strip()
                    file_path = os.path.join(REQUEST_DIR, f"{finger_id}.txt")
                    update_worker_file(file_path, f"[{get_today()}] Message: {custom_message}")
                    print(f"📩 [LETTER] New message saved for worker {finger_id}")

            # ۶. ثبت غیبت کارگر توسط مدیر
            elif incoming_msg.startswith("ABSENCE:"):
                parts = incoming_msg.split(":")
                if len(parts) >= 2:
                    finger_id = parts[1].strip()
                    file_path = os.path.join(ABSENCE_DIR, f"{finger_id}.txt")
                    update_worker_file(file_path, f"Date: {get_today()} - Status: ABSENT (Reported by Manager)")
                    print(f"⚠️ [MANAGER] Absence recorded for worker {finger_id}.")

            # ۷. گزارش خرابی
            elif incoming_msg.startswith("MANAGER:REPORT_FAIL:"):
                parts = incoming_msg.split(":", 2)
                if len(parts) >= 3:
                    report_text = parts[2].strip()
                    os.makedirs(os.path.dirname(REPORTS_PATH), exist_ok=True)
                    with open(REPORTS_PATH, 'a+', encoding='utf-8') as f:
                        f.write(f"[{get_today()}] SYSTEM FAILURE: {report_text}\n")
                    print("🚨 [MANAGER] Failure report saved.")

            # ۸. ارسال دستور مدیر
            elif incoming_msg.startswith("MANAGER:DEMAND:"):
                parts = incoming_msg.split(":", 2)
                if len(parts) >= 3:
                    demand_text = parts[2].strip()
                    os.makedirs(os.path.dirname(DEMANDS_PATH), exist_ok=True)
                    with open(DEMANDS_PATH, 'a+', encoding='utf-8') as f:
                        f.write(f"[{get_today()}] Demand/Task: {demand_text}\n")
                    print("📨 [MANAGER] Foreman demand saved.")
            
            # ۹. توقف خط تولید
            elif incoming_msg.startswith("MANAGER:STOP_LINE"):
                print("🛑 [ALARM] Production Line STOPPED by Manager!")  
                          
            # ۱۰. دریافت اطلاعات کامل کارگر توسط سرکارگر
            elif incoming_msg.startswith("MANAGER:INFO:"):
                finger_id = incoming_msg.split(":")[2].strip()
                if finger_id in workers_db:
                    info = workers_db[finger_id]
                    session = active_sessions.get(finger_id, {})
                    status = session.get('status', 'OUT')
                    
                    # بررسی آخرین وضعیت مرخصی کارگر
                    vac_status = "None"
                    vac_file = os.path.join(VACATION_DIR, f"{finger_id}.txt")
                    if os.path.exists(vac_file):
                        with open(vac_file, 'r', encoding='utf-8') as f:
                            lines = f.read().splitlines()
                            if lines and len(lines) > 1: # نادیده گرفتن خط شمارنده
                                vac_status = lines[-1].split(" - ")[-1] # گرفتن کلمه آخر

                    # بررسی ارسال نامه/درخواست توسط کارگر
                    letter_status = "No"
                    let_file = os.path.join(REQUEST_DIR, f"{finger_id}.txt")
                    if os.path.exists(let_file):
                        with open(let_file, 'r', encoding='utf-8') as f:
                            lines = f.read().splitlines()
                            if lines and len(lines) > 1:
                                letter_status = "Yes (Check DB)"

                    # ارسال لاین به لاین داشبورد برای ترمینال
                    send_to_mcu(f"@PRT:[SERVER]: --- WORKER {finger_id} INFO ---")
                    send_to_mcu(f"@PRT:[SERVER]: Name: {info['name']} | Status: {status}")
                    send_to_mcu(f"@PRT:[SERVER]: Phone: {info['phone']}")
                    send_to_mcu(f"@PRT:[SERVER]: Birth: {info['birth']}")
                    send_to_mcu(f"@PRT:[SERVER]: Vacation Req: {vac_status}")
                    send_to_mcu(f"@PRT:[SERVER]: Sent Letter: {letter_status}")
                    send_to_mcu(f"@PRT:[SERVER]: -----------------------------")
                    
                    send_to_mcu("@MSG:Worker data fetch complete.")
                    
                    lcd_update("Info Retrieved!", info['name'])
                    print(f"📊 [MANAGER] Dashboard sent for worker {finger_id}.")
                else:
                    send_to_mcu("@MSG:Error! Worker ID not found.")
                    lcd_update("Error!", "ID Not Found")
                    
except serial.SerialException as e:
    print(f"\n[ERROR]: Could not open {PORT}.")
except KeyboardInterrupt:
    print("\nMonitoring stopped by operator. Exiting.")