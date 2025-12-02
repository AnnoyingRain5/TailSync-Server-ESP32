# Example usage of the ESP TailLink Server
# Sending some preset patterns, and a pulse packet for good measure.
import serial
import time
import sys

PORT = "/dev/ttyACM0"
BAUD_RATE = 460800
PAYLOAD_SIZE = 192

# colour payloads
PAYLOAD_1 = b"\x01" + b"\x50\x00\x00\x00\x50\x00\x00\x00\x50\x50\x50\x50" * 16
PAYLOAD_2 = b"\x01" + b"\x00\x50\x00\x50\x50\x50\x50\x00\x00\x00\x00\x50" * 16
PAYLOAD_3 = b"\x00"

try:
    ser = serial.Serial(port=PORT, baudrate=BAUD_RATE, timeout=1)
    ser.flushInput()

except serial.SerialException as e:
    print(f"FATAL: Could not open serial port {PORT}: {e}", file=sys.stderr)
    sys.exit(1)

while True:
    ser.write(PAYLOAD_1)
    print(ser.readline().decode("utf-8"), end="")
    time.sleep(0.2)
    ser.write(PAYLOAD_2)
    print(ser.readline().decode("utf-8"), end="")
    time.sleep(0.2)
    ser.write(PAYLOAD_3)
    print(ser.readline().decode("utf-8"), end="")
