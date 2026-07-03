# dashboard/app/backend/models/factory_state.py
from PySide6.QtCore import QObject, Signal

class FactoryState(QObject):
    """The central state engine for the SCADA dashboard."""
    state_changed = Signal(str, object)

    def __init__(self):
        super().__init__()
        self._data = {
            "temp": 0.0, "smoke": 0, "fire": 0,
            "machine1": "OFFLINE", "machine2": "OFFLINE",
            "quality": "UNKNOWN", "employees": 0, "power": "OFF",
            "healthy_count": 0, "defect_count": 0
        }

    def update_property(self, key: str, value: any):
        if key in self._data and self._data[key] != value:
            self._data[key] = value
            self.state_changed.emit(key, value)

    def get(self, key: str, default=None):
        return self._data.get(key, default)