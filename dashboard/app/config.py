# dashboard/app/config.py

PORT = None  
BAUD_RATE = 9600

DEFAULT_MANAGER_USER = "admin"
DEFAULT_MANAGER_PASS = "1234"

CMD_FIRE_OFF = "CMD_FIRE_OFF\n"
CMD_FIRE_RESET = "CMD_FIRE_RESET\n"
CMD_M1_START = "CMD_M1_START\n"
CMD_M1_STOP = "CMD_M1_STOP\n"

SECTION_AVAILABILITY = {
    "fire_system": False,          
    "machine_monitoring": False,
    "quality_control": False,
    "workforce_management": False,
}