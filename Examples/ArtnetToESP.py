# Example usage of the ESP TailLink server
# Gets data from universe zero, and sends it over to taillink!
# Channels will be interpreted as RGB values (in that order). 30 pixels are expected, for 90 channels total.
# Other packet types are currently unimplimented.
from stupidArtnet import StupidArtnetServer
import time
import subprocess
import serial
import sys

UNIVERSE = 0
try:
    PORT = sys.argv[1]
except IndexError:
    print("Usage: python3 ArtnetToESP.py PATH/TO/SERIAL_PORT")
    exit(1)
BAUD_RATE = 460800

packet = b""
lastpacket = b""
lastPacketTx = time.time()

pulse = False


def rcv_artnet(data):
    """Test function to receive callback data."""
    # the received data is an array
    # of the channels value (no headers)
    global packet
    print("packet from ART-NET!")
    packet = bytes(b"\x01") + bytes(data[0:90])
    global pulse
    if data[90] == 255 and pulse == False:
        ser.write(bytes(b"\x00"))
        print(ser.readline().decode("utf-8"), end="")
        pulse = True
    else:
        pulse = False


try:
    ser = serial.Serial(port=PORT, baudrate=BAUD_RATE, timeout=1)
    ser.flushInput()

except serial.SerialException as e:
    print(f"FATAL: Could not open serial port {PORT}: {e}", file=sys.stderr)
    sys.exit(1)

srv = StupidArtnetServer()
listener = srv.register_listener(UNIVERSE, callback_function=rcv_artnet)

while True:
    if packet != lastpacket or time.time() - lastPacketTx > 2:
        ser.write(packet)
        print(ser.readline().decode("utf-8"), end="")
        lastpacket = packet
        lastPacketTx = time.time()
        time.sleep(0.0165)
