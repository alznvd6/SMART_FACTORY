# dashboard/app/main.py
import sys
import os

# Append project root paths to secure local directory imports
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from PySide6.QtWidgets import QApplication
from app.app import SmartFactoryApplication

def main():
    app = QApplication(sys.argv)
    app.setStyle("Fusion")
    
    window = SmartFactoryApplication()
    window.show()
    
    sys.exit(app.exec())

if __name__ == "__main__":
    main()