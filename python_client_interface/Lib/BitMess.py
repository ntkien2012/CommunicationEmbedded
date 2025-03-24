def bitMess(input_mess):
    """
    Hàm thực hiện:
      1. Đảo ngược tất cả các bit của input_mess.
      2. Hoán đổi các cặp bit (bit chẵn với bit lẻ liên tiếp).
    
    Tham số:
      input_mess: danh sách số nguyên (int list) với độ dài là bội của 16.
    
    Trả về:
      Danh sách các số nguyên hex (int list), mỗi số là 1 byte.
    """
    # Kiểm tra độ dài là bội của 16
    if len(input_mess) % 16 != 0:
        raise ValueError("Độ dài đầu vào phải là bội của 16")

    # Chuyển danh sách số nguyên thành chuỗi bit
    bit_message = ''.join(format(byte, '08b') for byte in input_mess)
    bit_message = [True if ch == '1' else False for ch in bit_message]
    
    # Bước 1: Đảo ngược tất cả bit
    inverted = [not bit for bit in bit_message]
    
    # Bước 2: Hoán đổi các cặp bit
    result = [False] * len(bit_message)
    for i in range(0, len(bit_message), 2):
        result[i] = inverted[i + 1]
        result[i + 1] = inverted[i]
    
    # Chuyển thành chuỗi bit và sau đó thành danh sách số nguyên hex
    result_bit_str = ''.join('1' if bit else '0' for bit in result)
    hex_result = [int(result_bit_str[i:i+8], 2) for i in range(0, len(result_bit_str), 8)]
    return hex_result  # Trả về danh sách số nguyên hex