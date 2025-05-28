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
# Khởi tạo Serial (UART2/COM7)
# =============================
ser = serial.Serial('COM7', 115200, timeout=1)

print("[INFO] Đã mở COM7 115200bps")

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

    # Chuẩn hóa payload: 56 byte cố định
    payload_raw = text.encode('utf-8')[:56]  # Giới hạn tối đa 56 byte
    payload = payload_raw.ljust(56, b'\x00')  # Bổ sung 0x00 nếu thiếu
    payload_len = len(payload)  # Luôn = 56

    # MAVLink V1 header: len, seq, sysid, compid, msgid
    header = struct.pack(
        "<BBBBB",
        payload_len,
        SEQUENCE & 0xFF,
        SYSTEM_ID,
        COMPONENT_ID,
        MESSAGE_ID
    )
    SEQUENCE += 1

    # CRC tính trên header + payload
    crc_input = header + payload
    crc = x25_crc(crc_input)

    # Tạo packet hoàn chỉnh (64 byte)
    packet = bytearray()
    packet.append(MAVLINK_STX)                   # 1 byte
    packet.extend(header)                        # 5 byte
    packet.extend(payload)                       # 56 byte
    packet.extend(struct.pack("<H", crc))        # 2 byte

    return packet  # Tổng cộng: 1 + 5 + 56 + 2 = 64 byte

# =============================
# Gửi dữ liệu từ bàn phím
# =============================
# try:
#     while True:
#         msg = input("Nhập nội dung muốn gửi: ").strip()
#         if not msg:
#             continue

#         pkt = mavlink_pack(msg)
#         ser.write(pkt)
#         print(f"[SENT] {len(pkt)} bytes: {pkt.hex(' ').upper()}")
#         time.sleep(0.05)
# except KeyboardInterrupt:
#     print("\n[INFO] Đã thoát.")
# finally:
#     ser.close()
message = "Hello from UART2"

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

