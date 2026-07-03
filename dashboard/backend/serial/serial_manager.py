# dashboard/app/backend/serial/serial_manager.py
import serial
from PySide6.QtCore import QThread, Signal, Slot
from backend.serial.packet_parser import IndustrialPacketParser

class IndustrialSerialManager(QThread):
    """Background hardware runner for non-blocking telemetry IO loops."""
    data_packet_received = Signal(dict)
    connection_state_changed = Signal(bool, str)

    def __init__(self, port=None, baudrate=9600):
        super().__init__()
        self.port = port
        self.baudrate = baudrate
        self.serial_bus = None
        self._keep_alive = True
        self.parser = IndustrialPacketParser()

    def run(self):
        if not self.port:
            self.connection_state_changed.emit(False, "Bus Offline: No Virtual COM Port Configured.")
            return

        try:
            self.serial_bus = serial.Serial(
                port=self.port, baudrate=self.baudrate,
                bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE, timeout=0.1
            )
            self.connection_state_changed.emit(True, f"Bus Online [{self.port}]")
        except Exception as err:
            self.connection_state_changed.emit(False, f"Connection Fault: {str(err)}")
            return

        buffer = ""
        while self._keep_alive:
            if self.serial_bus and self.serial_bus.is_open:
                try:
                    if self.serial_bus.in_waiting > 0:
                        raw_bytes = self.serial_bus.read(self.serial_bus.in_waiting)
                        buffer += raw_bytes.decode(errors="ignore")
                        if "#" in buffer:
                            parsed_payload = self.parser.parse_stream(buffer)
                            if parsed_payload:
                                self.data_packet_received.emit(parsed_payload)
                            last_hash_idx = buffer.rfind("#")
                            buffer = buffer[last_hash_idx + 1:]
                except Exception as ex:
                    print(f"Bus Read Error: {str(ex)}")
            self.msleep(30)

    @Slot(str)
    def transmit_command(self, cmd_string: str):
        if self.serial_bus and self.serial_bus.is_open:
            try:
                self.serial_bus.write(cmd_string.encode('utf-8'))
                self.serial_bus.flush()
            except Exception as err:
                print(f"Downstream Write Fault: {err}")
        else:
            print(f"Command dropped (Bus Offline): {cmd_string.strip()}")

    def terminate_session(self):
        self._keep_alive = False
        if self.serial_bus and self.serial_bus.is_open:
            self.serial_bus.close()
        self.wait()