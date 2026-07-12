import serial
import os
from datetime import datetime
import re

PORT = "COM2"
BAUD_RATE = 9600

def trim_string(str_val):
    # این خط تمام کاراکترهای کنترلی مثل بک‌اسپیس یا نویزهای اولیه را کاملاً پاک می‌کند
    clean_str = re.sub(r'[\x00-\x1f\x7f-\x9f]', '', str_val)
    return clean_str.strip()

# مسیر پایه دیتابیس کارخانه شما
BASE_DIR = r"C:\Users\ashka\Desktop\Embdded System PRoject\SMART_FACTORY\infrastructure\database"

# مسیر پوشه‌های زیرمجموعه
ATTENDANCE_DIR = os.path.join(BASE_DIR, 'worker_info', 'date', 'attendance')
VACATION_DIR = os.path.join(BASE_DIR, 'worker_info', 'date', 'vacation')
ABSENCE_DIR = os.path.join(BASE_DIR, 'worker_info', 'date', 'absence')
REQUEST_DIR = os.path.join(BASE_DIR, 'worker_info', 'date', 'worker_demands')

REPORTS_PATH = os.path.join(BASE_DIR, 'foreman_info', 'reports', 'reports.txt')
DEMANDS_PATH = os.path.join(BASE_DIR, 'foreman_info', 'demands', 'demands.txt')

def get_today():
    # اضافه شدن ساعت و دقیقه برای دقت بیشتر در لاگ‌ها
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S")

# =========================================================
# تابع هوشمند برای آپدیت فایل‌ها (مدیریت شمارنده و خطوط جدید)
# =========================================================
def update_worker_file(file_path, new_log_line):

    os.makedirs(os.path.dirname(file_path), exist_ok=True)
    
    lines = []
    # اگر فایل از قبل وجود داشت، اطلاعاتش رو می‌خونه
    if os.path.exists(file_path):
        with open(file_path, 'r', encoding='utf-8') as f:
            lines = f.read().splitlines()

    count = 0
    # اگر خط اول عدد بود، اون رو به عنوان شمارنده در نظر می‌گیره و بقیه رو به عنوان لاگ
    if lines and lines[0].isdigit():
        count = int(lines[0])
        logs = lines[1:]
    else:
        logs = lines

    # اضافه کردن یک واحد به شمارنده
    count += 1
    
    # چسباندن شمارنده جدید، لاگ‌های قدیمی و لاگِ جدید به صورت زیر هم
    new_content = [str(count)] + logs + [new_log_line]

    # بازنویسی تمیزِ فایل
    with open(file_path, 'w', encoding='utf-8') as f:
        f.write('\n'.join(new_content) + '\n')

# =========================================================

print(f"Target Database Path: {BASE_DIR}")
print(f"[{PORT}] Initializing Python IoT Server...")

try:
    ser = serial.Serial(PORT, BAUD_RATE, timeout=1)
    print(f"✅ Successfully connected to {PORT}! Waiting for Smart Factory data...\n")
    
    while True:
        raw_data = ser.readline()
        
        if raw_data:
            incoming_msg = raw_data.decode(errors="ignore").strip()
            if not incoming_msg:
                continue
                
            print(f"\n📥 [STM32 COMMAND]: {incoming_msg}")

            # ۱. حضور و غیاب
            if incoming_msg.startswith("CHECKIN:"):
                finger_id = incoming_msg.split(":")[1].strip()
                file_path = os.path.join(ATTENDANCE_DIR, f"{finger_id}.txt")
                update_worker_file(file_path, f"Checked in at: {get_today()}")
                print(f"✅ [ATTENDANCE] Worker {finger_id} checked in successfully.")

            # ۲. مرخصی
            elif incoming_msg.startswith("VACATION:"):
                parts = incoming_msg.split(":")
                if len(parts) >= 3:
                    finger_id = parts[1].strip()
                    status = parts[2].strip()
                    file_path = os.path.join(VACATION_DIR, f"{finger_id}.txt")
                    update_worker_file(file_path, f"Date: {get_today()} - Status: {status}")
                    print(f"🌴 [VACATION] Worker {finger_id} vacation is {status}")

            # ۳. وضعیت نامه‌ها
            elif incoming_msg.startswith("LETTER:"):
                # این دستور پیام را از روی دو نقطه (:) حداکثر ۲ بار جدا می‌کند
                parts = incoming_msg.split(":", 2)
                if len(parts) >= 3:
                    finger_id = parts[1].strip()
                    custom_message = parts[2].strip()
                    file_path = os.path.join(REQUEST_DIR, f"{finger_id}.txt")
                    
                    update_worker_file(file_path, f"[{get_today()}] Message: {custom_message}")
                    print(f"📩 [LETTER] New message saved for worker {finger_id}")

            # ۴. ثبت غیبت کارگر توسط مدیر
            elif incoming_msg.startswith("ABSENCE:"):
                parts = incoming_msg.split(":")
                if len(parts) >= 2:
                    finger_id = parts[1].strip()
                    file_path = os.path.join(ABSENCE_DIR, f"{finger_id}.txt")
                    update_worker_file(file_path, f"Date: {get_today()} - Status: ABSENT (Reported by Manager)")
                    print(f"⚠️ [MANAGER] Absence recorded for worker {finger_id}.")

            # ۵. گزارش خرابی
            elif incoming_msg.startswith("MANAGER:REPORT_FAIL:"):
                parts = incoming_msg.split(":", 2)
                if len(parts) >= 3:
                    report_text = parts[2].strip()
                    # ساخت خودکار پوشه در صورت نبودن
                    os.makedirs(os.path.dirname(REPORTS_PATH), exist_ok=True)
                    with open(REPORTS_PATH, 'a+', encoding='utf-8') as f:
                        f.write(f"[{get_today()}] SYSTEM FAILURE: {report_text}\n")
                    print("🚨 [MANAGER] Failure report saved.")

            # ۶. ارسال دستور مدیر
            elif incoming_msg.startswith("MANAGER:DEMAND:"):
                parts = incoming_msg.split(":", 2)
                if len(parts) >= 3:
                    demand_text = parts[2].strip()
                    # ساخت خودکار پوشه در صورت نبودن
                    os.makedirs(os.path.dirname(DEMANDS_PATH), exist_ok=True)
                    with open(DEMANDS_PATH, 'a+', encoding='utf-8') as f:
                        f.write(f"[{get_today()}] Demand/Task: {demand_text}\n")
                    print("📨 [MANAGER] Foreman demand saved.")
                
           # ۷. مدیریت پیشرفته بحران (Emergency)
            elif incoming_msg.startswith("EMERGENCY:"):
                parts = incoming_msg.split(":")
                alarm_type = parts[1].strip()
                
                # مشخص کردن پیام فارسی و انگلیسی برای فایل گزارش
                msg_mapping = {
                    "FIRE": "Fire - siren and hazard lights activated",
                    "MEDICAL_MACHINERY": "Medical emergency - machinery accident",
                    "MEDICAL_CHEMICAL": "Medical emergency - chemical poisoning or spill",
                    "CRITICAL_EVACUATE": "Critical situation - Order immediate evacuation and shutdown of the entire factory"
                }
                
                status_text = msg_mapping.get(alarm_type, f"بحران ناشناخته: {alarm_type}")
                
                # چاپ یک آلارم گرافیکی بزرگ در ترمینال پایتون
                print("\n" + "🚨"*20)
                print(f"⚠️  [ALERT]: {status_text}")
                print("🚨"*20 + "\n")
                
                # ذخیره گزارش در یک فایل اختصاصی برای مدیران
                emergency_log_path = os.path.join(BASE_DIR, 'foreman_info', 'reports', 'emergency_logs.txt')
                os.makedirs(os.path.dirname(emergency_log_path), exist_ok=True)
                
                with open(emergency_log_path, 'a+', encoding='utf-8') as f:
                    f.write(f"[{get_today()}] {status_text}\n")

except serial.SerialException as e:
    print(f"\n[ERROR]: Could not open {PORT}.")
    print("مطمئن شوید VSPD باز است و پورت COM2 اشغال نشده باشد.")
except KeyboardInterrupt:
    print("\nMonitoring stopped by operator. Exiting.")