def hamming_encode(data):
    """
    Encode 4-bit data into 7-bit Hamming(7,4) code.
    Input: data (list of 4 bits)
    Output: 7-bit encoded data
    """
    D1, D2, D3, D4 = data  # Lấy 4 bit dữ liệu

    # Tính toán các bit kiểm tra
    P1 = D1 ^ D2 ^ D4
    P2 = D1 ^ D3 ^ D4
    P3 = D2 ^ D3 ^ D4

    # Sắp xếp theo đúng vị trí (P1, P2, D1, P3, D2, D3, D4)
    encoded_data = [P1, P2, D1, P3, D2, D3, D4]

    return encoded_data

def hamming_decode(encoded_data):
    """
    Decode Hamming(7,4) and correct 1-bit errors.
    Input: 7-bit encoded data
    Output: 4-bit original data, corrected data if error exists
    """
    P1, P2, D1, P3, D2, D3, D4 = encoded_data

    # Kiểm tra lỗi bằng cách tính lại các bit kiểm tra
    C1 = P1 ^ D1 ^ D2 ^ D4  # Kiểm tra bit 1
    C2 = P2 ^ D1 ^ D3 ^ D4  # Kiểm tra bit 2
    C3 = P3 ^ D2 ^ D3 ^ D4  # Kiểm tra bit 4

    # Tính toán vị trí lỗi (nếu có)
    error_position = C1 * 1 + C2 * 2 + C3 * 4

    corrected_data = encoded_data[:]

    if error_position != 0:
        print(f"Error detected at position {error_position}, correcting...")
        corrected_data[error_position - 1] ^= 1  # Đảo bit bị lỗi

    # Trích xuất dữ liệu gốc từ dãy bit đã sửa lỗi
    P1, P2, D1, P3, D2, D3, D4 = corrected_data
    original_data = [D1, D2, D3, D4]

    return original_data


