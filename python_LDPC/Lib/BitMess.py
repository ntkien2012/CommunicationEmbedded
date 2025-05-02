def bitMess(input_mess):
    """
    The function performs:
    1. Inverts all bits of input_mess.
    2. Swaps pairs of bits (even bits with consecutive odd bits).

    Parameters:
    input_mess: an int list with a length multiple of 16.

    Returns:
    An int list of hex integers, each number is 1 byte.
    """
    # Check if length is a multiple of 16
    if len(input_mess) % 16 != 0:
        raise ValueError("Input length must be a multiple of 16")

    # Convert list of integers to bit string
    bit_message = ''.join(format(byte, '08b') for byte in input_mess)
    bit_message = [True if ch == '1' else False for ch in bit_message]
    
    # Step 1: Invert all bits
    inverted = [not bit for bit in bit_message]
    
    # Step 2: Swap the bit pairs
    result = [False] * len(bit_message)
    for i in range(0, len(bit_message), 2):
        result[i] = inverted[i + 1]
        result[i + 1] = inverted[i]
    
    # Convert to bit string and then to hex integer list
    result_bit_str = ''.join('1' if bit else '0' for bit in result)
    hex_result = [int(result_bit_str[i:i+8], 2) for i in range(0, len(result_bit_str), 8)]
    return hex_result  # Returns a list of hex integers