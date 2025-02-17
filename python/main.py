from Lib.BitMess import *
from Lib.MAVLink import *
from Lib.Hamming74 import *
from Lib.AES256 import *
import random
import sys
sys.stdout.reconfigure(encoding='utf-8')

# # key 256-bit (32 byte)
# key = [
#     0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
#     0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
#     0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
#     0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0xff,
# ]
# # plaintext 128-bit (16 byte)
# plaintext = [
#     0x00, 0x11, 0x22, 0x33,
#     0x44, 0x55, 0x96, 0x77,
#     0x88, 0x99, 0xaa, 0xbb,
#     0xcc, 0xdd, 0xee, 0xff,
# ]

def processing(plaintext,key):
    aes = AES256(key)
    ciphertext = aes.encrypt(plaintext)
    # print("Plaintext: ", ["%02x" % b for b in plaintext])
    # print("Ciphertext: ", ["%02x" % b for b in ciphertext])
    # Chuyển ciphertext sang chuỗi bit 128-bit
    result_bits = bitMess(ciphertext)
    # Tách chuỗi thành các nhóm 8 bit và chuyển mỗi nhóm sang hex
    hex_list = [int(result_bits[i:i+8], 2) for i in range(0, len(result_bits), 8)]

    # Tạo một thông điệp MAVLink
    msg = MavlinkMessage()
    msg.header = 0xFE         # giá trị start byte
    msg.len = 16               # Độ dài payload là 16 byte
    msg.seq = 1
    msg.sysid = 1
    msg.compid = 200
    msg.msgid = 50
    msg.payload = hex_list

    # Mã hoá thông điệp vào buffer
    buffer = []
    mavlink_encode(msg, buffer)
    # print("Encoded message (hex) MAVLink:", ["%02x" % b for b in buffer])

    # Danh sách để chứa các nibble (4-bit)
    nibbles = []

    for hex_str in buffer:
        # Chuyển đổi thành chuỗi nhị phân 8 bit, ví dụ '11111110' cho 'fe'
        bin_str = format(hex_str, '08b')
        # Tách chuỗi 8 bit thành 2 phần 4-bit
        nibble1 = bin_str[:4]
        nibble2 = bin_str[4:]
        nibbles.append(nibble1)
        nibbles.append(nibble2)

    nibbles_bits = [list(map(int, list(nibble))) for nibble in nibbles]
    encoded_nibbles = [hamming_encode(nb) for nb in nibbles_bits]
    # print("Encoded message hamming code:")
    # print(encoded_nibbles)

    # Giải mã từng nibble bằng Hamming (7,4)
    decoded_nibbles = [hamming_decode(enc) for enc in encoded_nibbles]
    # Sau đó, ghép từng 2 nibble thành 1 byte (8 bit)
    decoded_bytes = []
    for i in range(0, len(decoded_nibbles), 2):
        # Nối 2 nibble thành 1 chuỗi 8 bit
        nibble1 = ''.join(map(str, decoded_nibbles[i]))
        nibble2 = ''.join(map(str, decoded_nibbles[i+1]))
        byte_str = nibble1 + nibble2  # chuỗi 8 bit
        # Chuyển chuỗi nhị phân thành số nguyên
        byte_val = int(byte_str, 2)
        decoded_bytes.append(byte_val)

    # Chuyển đổi các byte thành chuỗi hex
    decoded_hex = [format(b, '02x') for b in decoded_bytes]
    # print("Decoded message (hex) hamming code:", ["%02x" % b for b in buffer])
    decoded_int = [int(h, 16) for h in decoded_hex]

    # Giả sử nhận được buffer đã mã hoá, giải mã lại
    rx_msg = MavlinkMessage()
    if mavlink_decode(rx_msg, decoded_int):
        # print("Decode MAVLink message thành công!")
        # print("Header: 0x%02x" % rx_msg.header)
        # print("Len: %d" % rx_msg.len)
        # print("Seq: %d" % rx_msg.seq)
        # print("SysID: %d" % rx_msg.sysid)
        # print("CompID: %d" % rx_msg.compid)
        # print("MsgID: %d" % rx_msg.msgid)
        # print("Payload:", rx_msg.payload)
        pass
    else:
        print("Decode thất bại: checksum không khớp") 

    inv_result_bits = inv_bitMess(rx_msg.payload)
    decrypted = aes.decrypt(inv_result_bits)
    # print("decrypted: ", ["%02x" % b for b in decrypted])
    return decrypted==plaintext

# Function to generate a random key (256-bit, 32 bytes)
def generate_random_key():
    return [random.randint(0, 255) for _ in range(32)]

# Function to generate a random plaintext (128-bit, 16 bytes)
def generate_random_plaintext():
    return [random.randint(0, 255) for _ in range(16)]

# Example usage
random_key = generate_random_key()
random_plaintext = generate_random_plaintext()

test_case = 1000

for i in range(test_case):
    random_key = generate_random_key()
    random_plaintext = generate_random_plaintext()
    if processing(random_plaintext,random_key):
        print(f"Test {i+1} OK!")
    else:
        print(f"Test {i+1} ERROR!")