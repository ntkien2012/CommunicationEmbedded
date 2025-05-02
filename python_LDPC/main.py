from Lib.BitMess import *
from Lib.MAVLink import *
from Lib.AES256 import *
from Lib.LDPC import *
import time
import random
import socket
import sys
sys.stdout.reconfigure(encoding='utf-8')

# ESP32 AP configuration
ESP32_AP_IP = "192.168.4.1"  # Default IP for ESP32 in AP mode
ESP32_UDP_PORT = 12345  # UDP port for communication
MAX_PACKET_SIZE = 1024  # Maximum UDP packet size

def send_data_udp(data, esp32_ip=ESP32_AP_IP, esp32_port=ESP32_UDP_PORT):
    """
    Send encoded data to an ESP32 in AP mode via UDP.
    
    Args:
        data: Binary data to send
        esp32_ip: IP address of the ESP32 AP (default: 192.168.4.1)
        esp32_port: UDP port to send to (default: 4210)
        
    Returns:
        bool: True if all data was sent, False otherwise
    """
    try:
        # Create a UDP socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        
        print(f"Setting up UDP transmission to {esp32_ip}:{esp32_port}...")
        
        # Convert the data list to bytes if it's not already
        if isinstance(data, list):
            data_bytes = bytes(data)
        else:
            data_bytes = data
            
        # Calculate number of packets needed
        total_packets = (len(data_bytes) + MAX_PACKET_SIZE - 1) // MAX_PACKET_SIZE
        
        print(f"Sending {len(data_bytes)} bytes in {total_packets} UDP packet(s)...")
        
        # Split data into packets if it's larger than MAX_PACKET_SIZE
        packets_sent = 0
        for i in range(0, len(data_bytes), MAX_PACKET_SIZE):
            # Get chunk of data for this packet
            packet_data = data_bytes[i:i+MAX_PACKET_SIZE]
            
            # Add packet header: [packet_index, total_packets]
            packet_header = bytes([packets_sent, total_packets])
            packet = packet_header + packet_data
            
            # Send the packet
            sock.sendto(packet, (esp32_ip, esp32_port))
            
            # Display progress
            packets_sent += 1
            print(f"Sent packet {packets_sent}/{total_packets} ({len(packet)} bytes)")
            
            # Small delay between packets to avoid overwhelming the receiver
            time.sleep(0.01)
        
        # Close the socket
        sock.close()
        
        if packets_sent == total_packets:
            print(f"All {packets_sent} packets sent successfully!")
            return True
        else:
            print(f"Only sent {packets_sent}/{total_packets} packets")
            return False
            
    except Exception as e:
        print(f"Error sending data via UDP: {e}")
        return False


def main():
    # --------------- ENCODING PROCESS ---------------
    # key 256-bit (32 byte)
    key = [
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    ]

    # Text to be encoded and transmitted
    text = "Helloooo!"
    plaintext = [b for b in text.encode()]

    H = np.load('H.npy')
    G = np.load('G.npy')

    # Step 1: Encrypt with AES256
    aes = AES256(key)
    ciphertext = aes.encrypt(plaintext)
    print("Plaintext: ", ["%02x" % b for b in plaintext])
    print("Ciphertext: ", ["%02x" % b for b in ciphertext])

    # Step 2: Convert ciphertext to bit message
    result_bits = bitMess(ciphertext)
    print("Result_bits: ", ["%02x" % b for b in result_bits])

    # Step 3: Create a MAVLink message
    msg = MavlinkMessage()
    msg.header = 0xFE       # start byte value
    msg.len = len(result_bits)  # payload length
    msg.seq = 1
    msg.sysid = 1
    msg.compid = 200
    msg.msgid = 50
    msg.payload = result_bits

    # Step 4: Encode the MAVLink message into buffer
    buffer = []
    mavlink_encode(msg, buffer)
    print("Encoded MAVLink message (hex):", ["%02x" % b for b in buffer])

    # Step 5: Apply LDPC encoding to the buffer
    # First convert buffer to bits
    buffer_bits = bytes_to_bits(buffer)

    # Make sure we have complete 128-bit blocks
    # If the number of bits is not a multiple of 128, pad with zeros
    remainder = len(buffer_bits) % 128
    if remainder > 0:
        padding = 128 - remainder
        buffer_bits.extend([0] * padding)

    # Encode each 128-bit block
    ldpc_encoded_bits = []
    ldpc_decoded_bits = []
    o_block = []
    decoder = BitFlipDecoder(H)
    for i in range(0, len(buffer_bits), 128):
        block = buffer_bits[i:i+128]
        # If block is less than 128 bits, pad it
        if len(block) < 128:
            block.extend([0] * (128 - len(block)))
        
        # Encode the block
        encoded_block = encode_ldpc(block, G)
        ldpc_encoded_bits.extend(encoded_block)

    # add noisy
    ldpc_encoded_bits[2] ^= 1
    ldpc_encoded_bits[15] ^= 1
    ldpc_encoded_bits[26] ^= 1
    ldpc_encoded_bits[37] ^= 1
    ldpc_encoded_bits[49] ^= 1

    ldpc_encoded_bytes = bits_to_bytes(ldpc_encoded_bits)

    print(f"LDPC encoded size: {len(ldpc_encoded_bytes)} bytes ({len(ldpc_encoded_bits)} bits)")
    print("LDPC encoded message:", ["%02x" % b for b in ldpc_encoded_bytes])

        
    # --------------- TRANSMISSION TO ESP32 ---------------
    print("\n--- Starting ESP32 UDP transmission ---")
    
    # Add protocol marker byte at the beginning (0xAA to identify as our protocol)
    transmission_data = [0xAA] + ldpc_encoded_bytes
    
    # Try to send the data to ESP32 via UDP
    success = send_data_udp(transmission_data)
    
    if success:
        print("Data successfully sent to ESP32 via UDP!")
    else:
        print("Failed to send all data to ESP32.")


if __name__ == "__main__":
    main()