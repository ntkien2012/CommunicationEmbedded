from Lib.BitMess import *
from Lib.MAVLink import *
from Lib.Hamming74 import *
from Lib.AES256 import *
import time
import struct
import threading
import sys
import tkinter as tk
from tkinter import scrolledtext, font
import datetime
import socket
sys.stdout.reconfigure(encoding='utf-8')

# Global variables for the GUI
received_messages = []
message_lock = threading.Lock()

class UDPCommunicationApp:
    def __init__(self, root):
        self.root = root
        self.root.title("UDP Communication Interface")
        # Increased window size for better resolution
        self.root.geometry("1200x800")
        # Enable high-DPI support
        try:
            from ctypes import windll
            windll.shcore.SetProcessDpiAwareness(1)
        except:
            pass
        
        # Adjusted font sizes for better resolution
        self.default_font = font.Font(family="Times New Roman", size=14)
        self.title_font = font.Font(family="Times New Roman", size=18, weight="bold")
        self.header_font = font.Font(family="Times New Roman", size=16, weight="bold")
        
        # Theme colors (unchanged)
        self.bg_color = "#f5f5f5"
        self.header_bg = "#3a7ca5"
        self.header_fg = "white"
        self.button_bg = "#2f6690"
        self.button_fg = "white"
        self.root.configure(bg=self.bg_color)
        
        # UDP settings (unchanged)
        self.UDP_IP_RECEIVE = "0.0.0.0"
        self.UDP_PORT_RECEIVE = 12345
        self.ESP32_IP = "192.168.4.1"
        self.UDP_PORT_SEND = 12345
        
        # Socket setup (unchanged)
        self.sock_receive = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock_receive.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock_receive.bind((self.UDP_IP_RECEIVE, self.UDP_PORT_RECEIVE))
        self.sock_send = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        
        # AES setup (unchanged)
        self.key = [0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                   0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
                   0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                   0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f]
        self.aes = AES256(self.key)
        
        self.create_widgets()
        
        # Thread setup (unchanged)
        self.receive_thread = threading.Thread(target=self.receive_data)
        self.receive_thread.daemon = True
        self.receive_thread.start()
        
        self.update_display_thread = threading.Thread(target=self.update_display_loop)
        self.update_display_thread.daemon = True
        self.update_display_thread.start()
        
        self.update_status(f"Connected and listening on {self.UDP_IP_RECEIVE}:{self.UDP_PORT_RECEIVE}")
    
    def create_widgets(self):
        # Header frame with increased padding
        header_frame = tk.Frame(self.root, bg=self.header_bg, pady=15)
        header_frame.pack(fill=tk.X)
        
        app_title = tk.Label(header_frame, text="Secure UDP Communication",
                           font=self.title_font, bg=self.header_bg, fg=self.header_fg)
        app_title.pack()
        
        # Main content frame with increased padding
        main_frame = tk.Frame(self.root, bg=self.bg_color, padx=30, pady=20)
        main_frame.pack(fill=tk.BOTH, expand=True)
        
        # Status section
        status_frame = tk.Frame(main_frame, bg=self.bg_color, bd=1, relief=tk.RAISED)
        status_frame.pack(fill=tk.X, pady=(0, 20), padx=10)
        
        status_label = tk.Label(status_frame, text="CONNECTION STATUS",
                              font=self.header_font, bg=self.bg_color)
        status_label.pack(anchor=tk.W, padx=10, pady=5)
        
        status_line = tk.Frame(status_frame, height=3, bg=self.header_bg)
        status_line.pack(fill=tk.X, pady=(0, 10), padx=10)
        
        self.status_label = tk.Label(status_frame, text="Status: Disconnected",
                                   fg="red", bg=self.bg_color, font=self.default_font)
        self.status_label.pack(anchor=tk.W, pady=10, padx=10)
        
        # Input section with better spacing
        input_frame = tk.Frame(main_frame, bg=self.bg_color, bd=1, relief=tk.RAISED)
        input_frame.pack(fill=tk.X, pady=20, padx=10)
        
        input_label = tk.Label(input_frame, text="SEND MESSAGE",
                             font=self.header_font, bg=self.bg_color)
        input_label.pack(anchor=tk.W, padx=10, pady=5)
        
        input_line = tk.Frame(input_frame, height=3, bg=self.header_bg)
        input_line.pack(fill=tk.X, pady=(0, 15), padx=10)
        
        message_frame = tk.Frame(input_frame, bg=self.bg_color)
        message_frame.pack(fill=tk.X, pady=10, padx=10)
        
        tk.Label(message_frame, text="Message:", bg=self.bg_color,
                font=self.default_font).pack(side=tk.LEFT, padx=10)
        
        self.message_entry = tk.Entry(message_frame, width=60, font=self.default_font,
                                    relief=tk.SOLID, bd=2)
        self.message_entry.pack(side=tk.LEFT, padx=10, fill=tk.X, expand=True)
        self.message_entry.bind("<Return>", self.send_message)
        
        self.send_button = tk.Button(message_frame, text="Send", command=self.send_message,
                                   bg=self.button_bg, fg=self.button_fg,
                                   font=self.default_font, pady=5, padx=20,
                                   relief=tk.RAISED, bd=3)
        self.send_button.pack(side=tk.LEFT, padx=10)
        
        # Display section with enhanced appearance
        display_frame = tk.Frame(main_frame, bg=self.bg_color, bd=1, relief=tk.RAISED)
        display_frame.pack(fill=tk.BOTH, expand=True, pady=20, padx=10)
        
        display_label = tk.Label(display_frame, text="RECEIVED MESSAGES",
                               font=self.header_font, bg=self.bg_color)
        display_label.pack(anchor=tk.W, padx=10, pady=5)
        
        display_line = tk.Frame(display_frame, height=3, bg=self.header_bg)
        display_line.pack(fill=tk.X, pady=(0, 15), padx=10)
        
        self.display_text = scrolledtext.ScrolledText(display_frame, wrap=tk.WORD,
                                                    font=self.default_font,
                                                    bg="white", relief=tk.SOLID,
                                                    bd=2, height=20)
        self.display_text.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        self.display_text.config(state=tk.DISABLED)
        
        # Footer with increased padding
        footer_frame = tk.Frame(self.root, bg=self.header_bg, pady=10)
        footer_frame.pack(fill=tk.X, side=tk.BOTTOM)
        
        footer_text = tk.Label(footer_frame, text="Secure Communication Protocol v1.0",
                             fg=self.header_fg, bg=self.header_bg, font=self.default_font)
        footer_text.pack()
    
    def update_status(self, message, color="green"):
        self.status_label.config(text=f"Status: {message}", fg=color, font=self.default_font)
    
    def send_message(self, event=None):
        message = self.message_entry.get()
        if not message:
            return
        
        try:
            threading.Thread(target=self.process_and_send, args=(message,)).start()
            self.message_entry.delete(0, tk.END)
            self.add_to_display(f"Sent: {message}", "blue")
        except Exception as e:
            self.add_to_display(f"Error sending message: {e}", "red")
    
    def process_and_send(self, message):
        # Encrypt and encode the message
        plaintext = [b for b in message.encode()]
        ciphertext = self.aes.encrypt(plaintext)
        
        # Convert ciphertext to bit message
        result_bits = bitMess(ciphertext)
        
        # Create MAVLink message
        msg = MavlinkMessage()
        msg.header = 0xFE
        msg.len = len(result_bits)
        msg.seq = 1
        msg.sysid = 1
        msg.compid = 200
        msg.msgid = 50
        msg.payload = result_bits
        
        # Encode message to buffer
        buffer = []
        mavlink_encode(msg, buffer)
        
        # Convert to nibbles
        nibbles = []
        for hex_str in buffer:
            bin_str = format(hex_str, '08b')
            nibble1 = bin_str[:4]
            nibble2 = bin_str[4:]
            nibbles.append(nibble1)
            nibbles.append(nibble2)
        
        # Apply Hamming encoding
        nibbles_bits = [list(map(int, list(nibble))) for nibble in nibbles]
        encoded_nibbles = [hamming_encode(nb) for nb in nibbles_bits]
        
        # Send the data
        self.sock_send.sendto(self.packet_7bit(encoded_nibbles), (self.ESP32_IP, self.UDP_PORT_SEND))
    
    def receive_data(self):
        while True:
            try:
                # Receive data
                data, addr = self.sock_receive.recvfrom(1024)
                
                # Process the received data
                processed_data = self.process_received_data(data, addr)
                
                if processed_data:
                    with message_lock:
                        received_messages.append({
                            "timestamp": datetime.datetime.now().strftime("%H:%M:%S"),
                            "data": processed_data
                        })
            except Exception as e:
                with message_lock:
                    received_messages.append({
                        "timestamp": datetime.datetime.now().strftime("%H:%M:%S"),
                        "data": f"Error processing data: {e}",
                        "error": True
                    })
    
    def process_received_data(self, data, addr):
        # Convert data to hex string
        hex_data = ' '.join(format(byte, '02x') for byte in data)
        hex_str = hex_data.replace(" ", "")
        
        # Convert hex to binary
        bin_str = bin(int(hex_str, 16))[2:].zfill(len(hex_str) * 4)
        
        # Split into 7-bit groups
        matrix = [[int(bit) for bit in bin_str[i:i+7]] for i in range(0, len(bin_str), 7)]
        
        # Decode using Hamming code
        decoded_nibbles = [hamming_decode(enc) for enc in matrix]
        
        # Combine nibbles into bytes
        decoded_bytes = []
        for i in range(0, len(decoded_nibbles), 2):
            if i+1 < len(decoded_nibbles):  # Ensure we have a pair
                nibble1 = ''.join(map(str, decoded_nibbles[i]))
                nibble2 = ''.join(map(str, decoded_nibbles[i+1]))
                byte_str = nibble1 + nibble2
                byte_val = int(byte_str, 2)
                decoded_bytes.append(byte_val)
        
        # Convert to hex
        decoded_hex = [format(b, '02x') for b in decoded_bytes]
        decoded_int = [int(h, 16) for h in decoded_hex]
        
        # Decode MAVLink message
        rx_msg = MavlinkMessage()
        if mavlink_decode(rx_msg, decoded_int):
            # Convert BitMess payload
            inv_result_bits = bitMess(rx_msg.payload)
            
            # Split into blocks and decrypt
            blocks = self.split_into_blocks(inv_result_bits)
            final = []
            for block in blocks:
                decrypted = self.aes.decrypt(block)
                final.extend(decrypted)
            
            # Convert to hex and then to bytes
            hex_str = ''.join(f"{byte:02X}" for byte in final)
            data = bytes.fromhex(hex_str)
            
            # Decode with padding
            try:
                return self.decode_with_padding(data)
            except Exception as e:
                return f"Error decoding message: {e}"
        else:
            return "Decode failed: checksum mismatch"
    
    def add_to_display(self, message, color="black"):
        with message_lock:
            received_messages.append({
                "timestamp": datetime.datetime.now().strftime("%H:%M:%S"),
                "data": message,
                "color": color
            })
    
    def update_display_loop(self):
        while True:
            self.update_display()
            time.sleep(0.1)  # Update every 100ms
    
    def update_display(self):
        global received_messages
        
        if not received_messages:
            return
        
        with message_lock:
            messages_to_display = received_messages.copy()
            received_messages = []
        
        self.display_text.config(state=tk.NORMAL)
        
        for msg in messages_to_display:
            timestamp = msg["timestamp"]
            data = msg["data"]
            color = msg.get("color", "black")
            
            self.display_text.insert(tk.END, f"[{timestamp}] ", "timestamp")
            self.display_text.insert(tk.END, f"{data}\n", color)
        
        self.display_text.config(state=tk.DISABLED)
        self.display_text.see(tk.END)  # Scroll to bottom
        
        # Configure tags for colors
        self.display_text.tag_configure("timestamp", foreground="gray")
        self.display_text.tag_configure("black", foreground="black")
        self.display_text.tag_configure("blue", foreground="#0066cc")
        self.display_text.tag_configure("red", foreground="#cc0000")
        self.display_text.tag_configure("green", foreground="#006600")
    
    def split_into_blocks(self, data, block_size=16):
        """Split data into fixed-size blocks."""
        return [data[i:i + block_size] for i in range(0, len(data), block_size)]
    
    def decode_with_padding(self, decrypted):
        """Decode data with PKCS#7 padding."""
        if len(decrypted) % 16 != 0:
            raise ValueError("Input length must be a multiple of 16 bytes")
        
        padding_value = decrypted[-1]
        
        if padding_value > 0 and padding_value <= 16:
            padding_start = len(decrypted) - padding_value
            if all(decrypted[i] == padding_value for i in range(padding_start, len(decrypted))):
                plaintext_len = len(decrypted) - padding_value
                received_plaintext = decrypted[:plaintext_len]
            else:
                plaintext_len = len(decrypted)
                received_plaintext = decrypted
        else:
            plaintext_len = len(decrypted)
            received_plaintext = decrypted
        
        return received_plaintext.decode('utf-8', errors='ignore')
    
    def packet_7bit(self, lst):
        """Pack a list of 7-bit integers into a byte array."""
        byte_data = bytearray()
        bits = 0
        temp = 0
        
        # Send the number of elements first (2 bytes, uint16_t)
        byte_data.extend(struct.pack("H", len(lst)))
        
        # Pack each 7-bit number
        for num in lst:
            if num > 127:  # Check 7-bit limit
                raise ValueError(f"Number {num} exceeds 7 bits (max 127)")
            temp |= (num & 0x7F) << bits  # Add 7 bits to temp
            bits += 7
            while bits >= 8:
                byte_data.append(temp & 0xFF)  # Take the lowest 8 bits
                temp >>= 8
                bits -= 8
        if bits > 0:  # Pack remaining bits
            byte_data.append(temp & 0xFF)
        return bytes(byte_data)


def main():
    root = tk.Tk()
    app = UDPCommunicationApp(root)
    root.protocol("WM_DELETE_WINDOW", lambda: (root.quit(), sys.exit()))
    root.mainloop()


if __name__ == "__main__":
    main()