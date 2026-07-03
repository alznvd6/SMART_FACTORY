# dashboard/app/ui/login_window.py
from PySide6.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QLabel, 
                             QLineEdit, QPushButton, QGroupBox, QFrame)
from PySide6.QtCore import Qt
from PySide6.QtGui import QFont
from app.config import DEFAULT_MANAGER_USER, DEFAULT_MANAGER_PASS, PORT

class LoginWindow(QWidget):
    def __init__(self, coordinator):
        super().__init__()
        self.coordinator = coordinator
        
        layout = QVBoxLayout(self)
        layout.setAlignment(Qt.AlignCenter)
        
        box = QGroupBox("SMART FACTORY CONTROL PANEL")
        box.setFixedWidth(380)
        box_layout = QVBoxLayout(box)
        box_layout.setSpacing(12)
        
        title_lbl = QLabel("SYSTEM AUTHENTICATION")
        title_lbl.setFont(QFont("Arial", 12, QFont.Bold))
        title_lbl.setAlignment(Qt.AlignCenter)
        box_layout.addWidget(title_lbl)
        
        self.user_input = QLineEdit()
        self.user_input.setPlaceholderText("Username")
        self.pass_input = QLineEdit()
        self.pass_input.setPlaceholderText("Password")
        self.pass_input.setEchoMode(QLineEdit.Password)
        
        box_layout.addWidget(QLabel("Operator User ID:"))
        box_layout.addWidget(self.user_input)
        box_layout.addWidget(QLabel("Security Key:"))
        box_layout.addWidget(self.pass_input)
        
        login_btn = QPushButton("Authenticate Access")
        login_btn.setStyleSheet("background-color: #2c3e50; color: white; padding: 6px; font-weight: bold;")
        login_btn.clicked.connect(self.process_login)
        box_layout.addWidget(login_btn)        
        status_frame = QFrame()
        status_frame.setFrameShape(QFrame.StyledPanel)
        status_layout = QHBoxLayout(status_frame)
        self.indicator_dot = QLabel("●")
        self.indicator_dot.setStyleSheet("color: red;")
        self.status_msg = QLabel(f"COM Port Linked: {PORT if PORT else 'NONE'}")
        status_layout.addWidget(self.indicator_dot)
        status_layout.addWidget(self.status_msg)
        status_layout.addStretch()
        box_layout.addWidget(status_frame)
        layout.addWidget(box)

    def process_login(self):
        if self.user_input.text() == DEFAULT_MANAGER_USER and self.pass_input.text() == DEFAULT_MANAGER_PASS:
            self.coordinator.authorize_and_launch()
        else:
            self.status_msg.setText("Authentication Rejected.")