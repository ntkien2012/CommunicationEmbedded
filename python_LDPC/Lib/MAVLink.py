# Necessary constants
X25_INIT_CRC = 0xFFFF
MAVLINK_CRC_EXTRA = False  # If using CRC extra, change to True

# Example of a function that returns the extra CRC for a given msgid.
# In practice, this function will return a value that depends on the msgid.
def MAVLINK_MESSAGE_CRC_(msgid):
    # For example: return a fixed value or lookup from a table
    return 0  # or change if needed


# CRC calculation function for each byte (equivalent to crc_accumulate)
def crc_accumulate(data, crcAccum):
    # data: an integer 0<=data<256
    # crcAccum: an integer 0<=crcAccum<65536
    tmp = data ^ (crcAccum & 0xFF)
    tmp = tmp ^ ((tmp << 4) & 0xFF)
    new_crc = (crcAccum >> 8) ^ ((tmp & 0xFF) << 8) ^ ((tmp & 0xFF) << 3) ^ ((tmp & 0xFF) >> 4)
    return new_crc & 0xFFFF


# Initialize crc
def crc_init():
    return X25_INIT_CRC


# Calculate CRC for a buffer (list or bytes)
def crc_calculate(pBuffer):
    crcTmp = crc_init()
    for b in pBuffer:
        crcTmp = crc_accumulate(b, crcTmp)
    return crcTmp


# Accumulate CRC for a buffer (buffer passed as string or list of characters)
def crc_accumulate_buffer(crcAccum, pBuffer):
    # pBuffer: string or list of characters
    for ch in pBuffer:
        # If pBuffer is a string, convert the character to ascii code (0<=x<256)
        if isinstance(ch, str):
            b = ord(ch)
        else:
            b = ch
        crcAccum = crc_accumulate(b, crcAccum)
    return crcAccum


# Defines the class that represents the MAVLink message
class MavlinkMessage:
    def __init__(self):
        self.header = 0         # Start byte
        self.len = 0            # Payload length
        self.seq = 0            # Numerical order
        self.sysid = 0          # System ID
        self.compid = 0         # Component ID
        self.msgid = 0          # Message ID
        self.payload = []       # List of bytes (integer 0<=x<256)
        self.checksum = 0       # Checksum value (calculated during encode/decode)
        self.checksum_calculator = [0, 0]  # Received checksum (2 bytes)


# Checksum support functions for MAVLink
def mavlink_start_checksum(msg):
    msg.checksum = crc_init()


def mavlink_update_checksum(msg, c):
    msg.checksum = crc_accumulate(c, msg.checksum)


# The states of the state machine when decoding
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
    Decode a MAVLink message from a list of bytes (c).
    rxmsg: MavlinkMessage object has been initialized
    c: list of bytes (integer 0<=x<256) containing the full message
    expected format: [header, len, seq, sysid, compid, msgid, payload[0...len-1], crc_low, crc_high]
    Returns True if the checksum is correct, False otherwise.
    """
    state = MAVLINK_STATE_IDLE

    # Use access index to list c (fixed value in MAVLink format)
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
            # If there is an undefined state, exit the loop.
            break

    # Split the calculated checksum into 2 bytes
    crc_low  = rxmsg.checksum & 0xFF
    crc_high = (rxmsg.checksum >> 8) & 0xFF

    if (crc_low == rxmsg.checksum_calculator[0] and
        crc_high == rxmsg.checksum_calculator[1]):
        return True  # Success
    else:
        return False  # Checksums do not match


def mavlink_encode(msg, buffer):
    """
    Encodes the MAVLink message from the msg object (MavlinkMessage)
    and writes the result to a buffer (a list, expandable as needed).
    Returns the length of the encoded message.
    """
    offset = 0

    # Write header and other fields
    buffer.append(msg.header)   ; offset += 1
    buffer.append(msg.len)      ; offset += 1
    buffer.append(msg.seq)      ; offset += 1
    buffer.append(msg.sysid)    ; offset += 1
    buffer.append(msg.compid)   ; offset += 1
    buffer.append(msg.msgid)    ; offset += 1

    # Initialize checksum
    mavlink_start_checksum(msg)

    # Accumulate checksum for header and message ID
    mavlink_update_checksum(msg, msg.len)
    mavlink_update_checksum(msg, msg.seq)
    mavlink_update_checksum(msg, msg.sysid)
    mavlink_update_checksum(msg, msg.compid)
    mavlink_update_checksum(msg, msg.msgid)

    # Add payload and accumulate checksum for payload
    for i in range(msg.len):
        byte = msg.payload[i]
        buffer.append(byte)
        mavlink_update_checksum(msg, byte)
    # If there is extra CRC, accumulate more
    if MAVLINK_CRC_EXTRA:
        mavlink_update_checksum(msg, MAVLINK_MESSAGE_CRC_(msg.msgid))

    # Take the calculated checksum and write it to the buffer (order: low byte first, then high byte)
    checksum = msg.checksum
    buffer.append(checksum & 0xFF)
    buffer.append((checksum >> 8) & 0xFF)
    offset += (6 + msg.len + 2)

    return offset