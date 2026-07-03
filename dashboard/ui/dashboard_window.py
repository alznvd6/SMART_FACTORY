# dashboard/app/ui/dashboard_window.py
from PySide6.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QLabel, 
                             QGroupBox, QGridLayout, QPushButton)
from PySide6.QtCore import Qt, Slot
from app.config import SECTION_AVAILABILITY, CMD_FIRE_OFF

class DashboardWindow(QWidget):
    def __init__(self, coordinator, factory_model, serial_manager):
        super().__init__()
        self.coordinator = coordinator
        self.model = factory_model
        self.serial = serial_manager
        
        self.main_layout = QVBoxLayout(self)
        
        self.top_bar = QHBoxLayout()
        self.bus_status_lbl = QLabel("System Status: Local Loop Initialization...")
        self.bus_status_lbl.setStyleSheet("background-color: #2c3e50; color: white; padding: 8px; font-weight: bold;")
        self.top_bar.addWidget(self.bus_status_lbl)
        self.main_layout.addLayout(self.top_bar)
        
        self.grid = QGridLayout()
        self.main_layout.addLayout(self.grid)
        
        self.build_subsystem_blocks()
        self.model.state_changed.connect(self.handle_model_refresh)

    def build_subsystem_blocks(self):
        # 1. Fire Safety block
        self.fire_box = QGroupBox("Fire Mitigation Subsystem")
        fl = QVBoxLayout(self.fire_box)
        if not SECTION_AVAILABILITY["fire_system"]:
            fl.addWidget(QLabel("⚠️ Section Not Available yet in current schematic blueprint."), alignment=Qt.AlignCenter)
            self.fire_box.setDisabled(True)
        else:
            self.temp_lbl = QLabel("Temperature: -- °C")
            self.smoke_lbl = QLabel("Smoke Latch: --")
            self.silence_btn = QPushButton("Silence Fire Alarm")
            self.silence_btn.clicked.connect(lambda: self.serial.transmit_command(CMD_FIRE_OFF))
            fl.addWidget(self.temp_lbl)
            fl.addWidget(self.smoke_lbl)
            fl.addWidget(self.silence_btn)
        self.grid.addWidget(self.fire_box, 0, 0)

        # 2. Machine Monitoring block
        self.mach_box = QGroupBox("Machine Telemetry Subsystem")
        ml = QVBoxLayout(self.mach_box)
        if not SECTION_AVAILABILITY["machine_monitoring"]:
            ml.addWidget(QLabel("⚠️ Section Not Available yet in current schematic blueprint."), alignment=Qt.AlignCenter)
            self.mach_box.setDisabled(True)
        else:
            self.m1_lbl = QLabel("Machine 1 State: --")
            ml.addWidget(self.m1_lbl)
        self.grid.addWidget(self.mach_box, 0, 1)

        # 3. Quality Control block
        self.qc_box = QGroupBox("Quality Evaluation Line")
        ql = QVBoxLayout(self.qc_box)
        if not SECTION_AVAILABILITY["quality_control"]:
            ql.addWidget(QLabel("⚠️ Section Not Available yet in current schematic blueprint."), alignment=Qt.AlignCenter)
            self.qc_box.setDisabled(True)
        else:
            self.qc_lbl = QLabel("Line Status: --")
            ql.addWidget(self.qc_lbl)
        self.grid.addWidget(self.qc_box, 1, 0)

        # 4. Workforce Management block
        self.wf_box = QGroupBox("Workforce Logistics Matrix")
        wl = QVBoxLayout(self.wf_box)
        if not SECTION_AVAILABILITY["workforce_management"]:
            wl.addWidget(QLabel("⚠️ Section Not Available yet in current schematic blueprint."), alignment=Qt.AlignCenter)
            self.wf_box.setDisabled(True)
        else:
            self.wf_lbl = QLabel("Active Staff: --")
            wl.addWidget(self.wf_lbl)
        self.grid.addWidget(self.wf_box, 1, 1)

    @Slot(bool, str)
    def update_bus_status(self, is_online: bool, message: str):
        self.bus_status_lbl.setText(f"SCADA State: {message}")
        if not is_online:
            self.bus_status_lbl.setStyleSheet("background-color: #7f8c8d; color: white; padding: 8px;")

    @Slot(str, object)
    def handle_model_refresh(self, key: str, value: object):
        if SECTION_AVAILABILITY["fire_system"]:
            if key == "temp": self.temp_lbl.setText(f"Temperature: {value} °C")
            if key == "smoke": self.smoke_lbl.setText(f"Smoke Latch: {'ALERT' if value == 1 else 'CLEAR'}")
        if SECTION_AVAILABILITY["machine_monitoring"] and key == "machine1":
            self.m1_lbl.setText(f"Machine 1 State: {value}")