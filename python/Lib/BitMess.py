def bitMess(input_mess):
    """
    Hàm thực hiện:
      1. Đảo ngược tất cả các bit của input_bits.
      2. Hoán đổi các cặp bit (bit chẵn với bit lẻ liên tiếp).
    
    Tham số:
      input_bits: danh sách gồm 128 giá trị boolean (True/False),
                  biểu diễn bitset 128 bit.
    
    Trả về:
      Một danh sách 128 giá trị boolean sau khi đã xử lý.
    """
    bit_message = ''.join(format(byte, '08b') for byte in input_mess)
    bit_message = [True if ch == '1' else False for ch in bit_message]
    if len(bit_message) != 128:
        raise ValueError("Đầu vào phải là danh sách gồm 128 bit")
    
    # Bước 1: Đảo ngược các bit
    inverted = [not bit for bit in bit_message]
    
    # Bước 2: Hoán đổi các cặp bit
    result = [False] * 128  # Khởi tạo danh sách 128 phần tử
    for i in range(0, 128, 2):
        # Hoán đổi vị trí i và i+1
        result[i] = inverted[i + 1]
        result[i + 1] = inverted[i]
    # Giả sử result_bits là danh sách các boolean, chuyển thành chuỗi "0" và "1"
    result_bit_str = ''.join('1' if bit else '0' for bit in result)
    
    return result_bit_str

def inv_bitMess(input_mess):
    """
    Đảo ngược lại của hàm bitMess
    """
    inv_bit_message = ''.join(format(byte, '08b') for byte in input_mess)
    inv_bit_message = [True if ch == '1' else False for ch in inv_bit_message]
    if len(inv_bit_message) != 128:
        raise ValueError("Đầu vào phải là danh sách gồm 128 bit")
    
    # Bước 1: Hoán đổi các cặp bit
    result = [False] * 128  # Khởi tạo danh sách 128 phần tử
    for i in range(0, 128, 2):
        # Hoán đổi vị trí i và i+1
        result[i] = inv_bit_message[i + 1]
        result[i + 1] = inv_bit_message[i]

     # Bước 2: Đảo ngược các bit
    inverted = [not bit for bit in result]
    inv_result_bit_str = ''.join('1' if bit else '0' for bit in inverted)
    inv_hex_list = [int(inv_result_bit_str[i:i+8], 2) for i in range(0, len(inv_result_bit_str), 8)]
    
    return inv_hex_list

