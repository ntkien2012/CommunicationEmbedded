# Secure WiFi Data Communication with ESP32

This project implements a robust, secure link between a PC and an ESP32 microcontroller over WiFi, combining industry-standard encryption with lightweight channel coding and structured messaging to ensure both confidentiality and reliability in embedded applications.

## Key Components and Features

### 1. ESP32 WiFi Communication  
- Leverages the ESP32’s built-in WiFi to form a bi-directional, real-time data link.  
- Ideal for embedded and IoT scenarios where cable-free connectivity is required.

### 2. MAVLink Messaging Protocol  
- Uses the MAVLink v2 protocol to frame and parse every packet.  
- Guarantees interoperability with MAVLink-based ground stations and autopilots (common in drones & robotics).

### 3. AES-256 Encryption  
- Applies AES-256 in CBC mode (configurable IV) for payload confidentiality.  
- Only devices with the correct 256-bit key can decrypt and read the data.

### 4. BitMess Obfuscation  
- Introduces a bit-level scrambling step after encryption to thwart simple traffic analysis.  
- Compatible with any underlying transport—adds negligible latency.

### 5. LDPC Channel Coding with Bit-Flipping Decoder  
- Replaces the previous Hamming(7,4) block code with a modern **Low-Density Parity-Check (LDPC)** code.  
- Encodes each packet with an LDPC parity-check matrix and decodes on the ESP32 using a hard-decision **Bit-Flipping** algorithm:  
  - **Static phase**: flips all bits whose unsatisfied-check count exceeds a per-bit threshold.  
  - **Dynamic phase**: if no static flips occur, flips one bit at a time (highest syndrome count).  
- Corrects single and multiple errors efficiently, yielding much higher resilience to WiFi interference than Hamming.

---

## Workflow Overview

1. **PC Side**  
   - Payload → AES-256 encrypt → BitMess scramble → MAVLink framing → LDPC encode → WiFi send  

2. **ESP32 Side**  
   - WiFi receive → LDPC Bit-Flip decode → MAVLink parse → BitMess descramble → AES-256 decrypt → Application  

---

## Benefits

- **Security**: AES-256 + BitMess ensures confidentiality and obfuscation.  
- **Reliability**: LDPC with Bit-Flipping corrects errors from noisy WiFi links.  
- **Interoperability**: MAVLink framing fits into existing UAV/robotics ecosystems.  
- **Performance**: All operations run in real time on the ESP32 with minimal overhead.

---

## Getting Started

See [INSTALL.md](INSTALL.md) for setup steps, and [CODE.md](CODE.md) for build & run instructions.
