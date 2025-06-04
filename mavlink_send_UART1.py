import serial
import struct
import time

# =============================
# MAVLink V1 Packet Constants
# =============================
MAVLINK_STX       = 0xFE
SYSTEM_ID         = 1
COMPONENT_ID      = 1
MESSAGE_ID        = 0x01  # Message ID bạn tự định nghĩa
SEQUENCE          = 0     # Global sequence counter

# =============================
# Khởi tạo Serial (UART1/COM11)
# =============================
ser = serial.Serial('COM11', 115200, timeout=1)
print("[INFO] Đã mở COM11 115200bps (UART1)")

def x25_crc(data: bytes) -> int:
    """Tính CRC theo chuẩn X.25 (MAVLink V1)"""
    crc = 0xFFFF
    for b in data:
        tmp = b ^ (crc & 0xFF)
        tmp ^= (tmp << 4) & 0xFF
        crc = ((crc >> 8) ^ (tmp << 8) ^ (tmp << 3) ^ (tmp >> 4)) & 0xFFFF
    return crc

def mavlink_pack(text: str) -> bytes:
    global SEQUENCE

    # Chuẩn hóa payload
    payload = text.encode('utf-8')[:255]
    payload_len = len(payload)  

    # MAVLink V1 header
    header = struct.pack(
        "<BBBBB",
        payload_len,
        SEQUENCE & 0xFF,
        SYSTEM_ID,
        COMPONENT_ID,
        MESSAGE_ID
    )
    SEQUENCE = (SEQUENCE + 1) % 256

    # Tính CRC
    crc_input = header + payload
    crc = x25_crc(crc_input)

    # Gói đầy đủ
    packet = bytearray()
    packet.append(MAVLINK_STX)             # 1 byte STX
    packet.extend(header)                  # 5 byte header
    packet.extend(payload)                 # 0-255 byte payload
    packet.extend(struct.pack("<H", crc))  # 2 byte CRC

    return packet 

# =============================
# Gửi dữ liệu tự động mỗi t giây
# =============================
message = "Hello KHANH from UART1"
interval = 1.0  # giây/gói

try:
    while True:
        pkt = mavlink_pack(message)
        ser.write(pkt)
        print(f"[SENT] {len(pkt)} bytes: {pkt.hex(' ').upper()}")
        time.sleep(interval)
except KeyboardInterrupt:
    print("\n[INFO] Đã thoát.")
finally:
    ser.close()
