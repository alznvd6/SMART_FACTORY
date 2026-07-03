# dashboard/app/app.py

from PySide6.QtWidgets import QMainWindow, QStackedWidget
from ui.dashboard_window import DashboardWindow
from app.config import PORT, BAUD_RATE
from ui.login_window import LoginWindow
from backend.models.factory_state import FactoryState
from backend.serial.serial_manager import IndustrialSerialManager


class SmartFactoryApplication(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Smart Factory Suite - SCADA Interface Engine")
        self.resize(1000, 650)

        self.factory_model = FactoryState()
        self.serial_manager = IndustrialSerialManager(port=PORT, baudrate=BAUD_RATE)

        self.view_stack = QStackedWidget()
        self.setCentralWidget(self.view_stack)

        self.login_view = LoginWindow(self)
        self.dashboard_view = DashboardWindow(self, self.factory_model, self.serial_manager)

        self.view_stack.addWidget(self.login_view)
        self.view_stack.addWidget(self.dashboard_view)
        self.view_stack.setCurrentWidget(self.login_view)

        self.serial_manager.data_packet_received.connect(self.route_telemetry)
        self.serial_manager.connection_state_changed.connect(self.dashboard_view.update_bus_status)

    def authorize_and_launch(self):
        self.view_stack.setCurrentWidget(self.dashboard_view)
        self.serial_manager.start()

    def route_telemetry(self, packet: dict):
        for key, value in packet.items():
            self.factory_model.update_property(key, value)

    def closeEvent(self, event):
        self.serial_manager.terminate_session()
        event.accept()