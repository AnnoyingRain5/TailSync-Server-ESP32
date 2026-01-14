# Example usage of the ESP TailLink Server
# Sending some preset patterns, and a pulse packet for good measure.
import serial
import time
import sys

PORT = sys.argv[1]
BAUD_RATE = 460800
PAYLOAD_SIZE = 192

# colour payloads
PAYLOAD = (
    b"\x01"  # header
    + (b"\x00\x00\x00" + b"\x00\x00\x00" + b"\x00\x00\x00") * 2  # head
    + (b"\x00\x00\x00" + b"\x00\x00\x00" + b"\x00\x00\x00") * 2  # body
    + (b"\x00\x00\x00" + b"\x00\x00\x00" + b"\x00\x00\x00") * 2  # arms
    + (b"\x00\x00\x00" + b"\x00\x00\x00" + b"\x00\x00\x00") * 2  # legs
    + (b"\x00\x00\x00" + b"\x00\x00\x00" + b"\x00\x00\x00") * 2  # tail
)
print(len(PAYLOAD))

try:
    ser = serial.Serial(port=PORT, baudrate=BAUD_RATE, timeout=1)
    ser.flushInput()

except serial.SerialException as e:
    print(f"FATAL: Could not open serial port {PORT}: {e}", file=sys.stderr)
    sys.exit(1)

ser.write(PAYLOAD)
print(ser.readline().decode("utf-8"), end="")
