# dashboard/app/backend/serial/packet_parser.py
import re

class IndustrialPacketParser:
    """De-serializes incoming $KEY=VALUE# streaming telemetry frames."""
    def __init__(self):
        self.packet_pattern = re.compile(r'\$([^=#]+)=([^=#]+)#')

    def parse_stream(self, raw_string: str) -> dict:
        extracted_data = {}
        matches = self.packet_pattern.findall(raw_string)
        for key, value in matches:
            clean_key = key.strip().lower()
            clean_val = value.strip().upper()
            if clean_val.isdigit():
                extracted_data[clean_key] = int(clean_val)
            else:
                try:
                    extracted_data[clean_key] = float(clean_val)
                except ValueError:
                    extracted_data[clean_key] = clean_val
        return extracted_data