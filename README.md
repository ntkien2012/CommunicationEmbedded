Project Overview: Secure WiFi Data Communication with ESP32
This project focuses on securely transmitting data between a PC and an ESP32 microcontroller over a WiFi connection. It integrates AES-256 encryption for data security, BitMess for additional obfuscation, MAVLink for structured messaging, and Hamming(7,4) channel coding for error correction, ensuring reliable and confidential communication in a wireless environment.

Key Components and Features
ESP32 WiFi Communication:
Utilizes the ESP32’s built-in WiFi capabilities to establish a wireless link between the PC and the ESP32.
Suitable for real-time data exchange in embedded systems.
MAVLink Messaging Protocol:
Implements the lightweight MAVLink protocol to structure and standardize data packets exchanged between the PC and ESP32.
Ensures compatibility with MAVLink-based systems, commonly used in drones and robotics.
AES-256 Encryption:
Applies AES-256, an industry-standard encryption algorithm, to protect the confidentiality of transmitted data.
Ensures that only authorized devices with the correct key can decrypt and access the information.
BitMess (Bit Manipulation):
Adds a layer of bit-level transformation to the encrypted data, enhancing obfuscation and potentially aiding compliance with specific transmission protocols.
Hamming(7,4) Channel Coding:
Incorporates Hamming(7,4) error-correcting code to detect and correct single-bit errors during transmission.
Enhances reliability over WiFi, which may be susceptible to interference or packet loss.