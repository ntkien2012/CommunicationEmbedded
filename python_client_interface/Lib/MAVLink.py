# Các hằng số cần thiết
X25_INIT_CRC = 0xFFFF
MAVLINK_CRC_EXTRA = False  # Nếu có dùng CRC extra, đổi thành True

# Ví dụ về hàm trả về CRC extra cho msgid nào đó.
# Trong thực tế, hàm này sẽ trả về một giá trị phụ thuộc vào msgid.
def MAVLINK_MESSAGE_CRC_(msgid):
    # Ví dụ: trả về một giá trị cố định hay lookup từ bảng
    return 0  # Thay đổi nếu cần


# Hàm tính toán CRC cho từng byte (tương đương crc_accumulate)
def crc_accumulate(data, crcAccum):
    # data: một số nguyên 0<=data<256
    # crcAccum: số nguyên 0<=crcAccum<65536
    tmp = data ^ (crcAccum & 0xFF)
    tmp = tmp ^ ((tmp << 4) & 0xFF)
    new_crc = (crcAccum >> 8) ^ ((tmp & 0xFF) << 8) ^ ((tmp & 0xFF) << 3) ^ ((tmp & 0xFF) >> 4)
    return new_crc & 0xFFFF


# Khởi tạo crc
def crc_init():
    return X25_INIT_CRC


# Tính toán CRC cho một buffer (danh sách hoặc bytes)
def crc_calculate(pBuffer):
    crcTmp = crc_init()
    for b in pBuffer:
        crcTmp = crc_accumulate(b, crcTmp)
    return crcTmp


# Tích lũy CRC cho một buffer (buffer truyền vào kiểu chuỗi hoặc danh sách các ký tự)
def crc_accumulate_buffer(crcAccum, pBuffer):
    # pBuffer: chuỗi hoặc danh sách ký tự
    for ch in pBuffer:
        # Nếu pBuffer là chuỗi, chuyển ký tự sang mã ascii (0<=x<256)
        if isinstance(ch, str):
            b = ord(ch)
        else:
            b = ch
        crcAccum = crc_accumulate(b, crcAccum)
    return crcAccum


# Định nghĩa lớp đại diện cho thông điệp MAVLink
class MavlinkMessage:
    def __init__(self):
        self.header = 0         # Start byte
        self.len = 0            # Độ dài payload
        self.seq = 0            # Số thứ tự
        self.sysid = 0          # System ID
        self.compid = 0         # Component ID
        self.msgid = 0          # Message ID
        self.payload = []       # Danh sách các byte (số nguyên 0<=x<256)
        self.checksum = 0       # Giá trị checksum (tính toán trong quá trình encode/decode)
        self.checksum_calculator = [0, 0]  # Checksum nhận được (2 byte)


# Các hàm hỗ trợ checksum cho MAVLink
def mavlink_start_checksum(msg):
    msg.checksum = crc_init()


def mavlink_update_checksum(msg, c):
    msg.checksum = crc_accumulate(c, msg.checksum)


# Các trạng thái của máy trạng thái khi decode
MAVLINK_STATE_IDLE      = 0
MAVLINK_STATE_GOT_STX   = 1
MAVLINK_STATE_GOT_LENGTH = 2
MAVLINK_STATE_GOT_SEQ   = 3
MAVLINK_STATE_GOT_SYSID = 4
MAVLINK_STATE_GOT_COMPID = 5
MAVLINK_STATE_GOT_MSGID = 6
MAVLINK_STATE_GOT_PAYLOAD = 7
MAVLINK_STATE_GOT_CRC   = 8
MAVLINK_STATE_GOT_CRC1  = 9


def mavlink_decode(rxmsg, c):
    """
    Giải mã một thông điệp MAVLink từ danh sách các byte (c).
    rxmsg: đối tượng MavlinkMessage đã được khởi tạo
    c: danh sách các byte (số nguyên 0<=x<256) chứa thông điệp đầy đủ
       dự kiến định dạng: [header, len, seq, sysid, compid, msgid, payload[0...len-1], crc_low, crc_high]
    Trả về True nếu checksum đúng, False nếu sai.
    """
    state = MAVLINK_STATE_IDLE

    # Dùng chỉ số truy cập vào danh sách c (giá trị cố định theo định dạng MAVLink)
    while state != MAVLINK_STATE_GOT_CRC1:
        if state == MAVLINK_STATE_IDLE:
            mavlink_start_checksum(rxmsg)
            state = MAVLINK_STATE_GOT_STX

        elif state == MAVLINK_STATE_GOT_STX:
            rxmsg.header = c[0]
            state = MAVLINK_STATE_GOT_LENGTH

        elif state == MAVLINK_STATE_GOT_LENGTH:
            rxmsg.len = c[1]
            mavlink_update_checksum(rxmsg, c[1])
            state = MAVLINK_STATE_GOT_SEQ

        elif state == MAVLINK_STATE_GOT_SEQ:
            rxmsg.seq = c[2]
            mavlink_update_checksum(rxmsg, c[2])
            state = MAVLINK_STATE_GOT_SYSID

        elif state == MAVLINK_STATE_GOT_SYSID:
            rxmsg.sysid = c[3]
            mavlink_update_checksum(rxmsg, c[3])
            state = MAVLINK_STATE_GOT_COMPID

        elif state == MAVLINK_STATE_GOT_COMPID:
            rxmsg.compid = c[4]
            mavlink_update_checksum(rxmsg, c[4])
            state = MAVLINK_STATE_GOT_MSGID

        elif state == MAVLINK_STATE_GOT_MSGID:
            rxmsg.msgid = c[5]
            mavlink_update_checksum(rxmsg, c[5])
            state = MAVLINK_STATE_GOT_PAYLOAD

        elif state == MAVLINK_STATE_GOT_PAYLOAD:
            rxmsg.payload = []
            for i in range(rxmsg.len):
                byte = c[6 + i]
                rxmsg.payload.append(byte)
                mavlink_update_checksum(rxmsg, byte)
            if MAVLINK_CRC_EXTRA:
                mavlink_update_checksum(rxmsg, MAVLINK_MESSAGE_CRC_(rxmsg.msgid))
            state = MAVLINK_STATE_GOT_CRC

        elif state == MAVLINK_STATE_GOT_CRC:
            rxmsg.checksum_calculator = [ c[6 + rxmsg.len], c[7 + rxmsg.len] ]
            state = MAVLINK_STATE_GOT_CRC1

        else:
            # Nếu có trạng thái không xác định, thoát vòng lặp
            break

    # Tách checksum tính được thành 2 byte
    crc_low  = rxmsg.checksum & 0xFF
    crc_high = (rxmsg.checksum >> 8) & 0xFF

    if (crc_low == rxmsg.checksum_calculator[0] and
        crc_high == rxmsg.checksum_calculator[1]):
        return True  # Thành công
    else:
        return False  # Checksum không khớp


def mavlink_encode(msg, buffer):
    """
    Mã hoá thông điệp MAVLink từ đối tượng msg (MavlinkMessage)
    và ghi kết quả vào buffer (danh sách, được mở rộng theo nhu cầu).
    Trả về chiều dài của thông điệp đã mã hoá.
    """
    offset = 0

    # Ghi header và các trường khác
    buffer.append(msg.header)   ; offset += 1
    buffer.append(msg.len)      ; offset += 1
    buffer.append(msg.seq)      ; offset += 1
    buffer.append(msg.sysid)    ; offset += 1
    buffer.append(msg.compid)   ; offset += 1
    buffer.append(msg.msgid)    ; offset += 1

    # Khởi tạo checksum
    mavlink_start_checksum(msg)

    # Tích lũy checksum cho header và message ID
    mavlink_update_checksum(msg, msg.len)
    mavlink_update_checksum(msg, msg.seq)
    mavlink_update_checksum(msg, msg.sysid)
    mavlink_update_checksum(msg, msg.compid)
    mavlink_update_checksum(msg, msg.msgid)

    # Thêm payload và tích lũy checksum cho payload
    for i in range(msg.len):
        byte = msg.payload[i]
        buffer.append(byte)
        mavlink_update_checksum(msg, byte)
    # Nếu có CRC extra, tích lũy thêm
    if MAVLINK_CRC_EXTRA:
        mavlink_update_checksum(msg, MAVLINK_MESSAGE_CRC_(msg.msgid))

    # Lấy checksum đã tính và ghi vào buffer (thứ tự: low byte trước, sau đó high byte)
    checksum = msg.checksum
    buffer.append(checksum & 0xFF)
    buffer.append((checksum >> 8) & 0xFF)
    offset += (6 + msg.len + 2)

    return offset