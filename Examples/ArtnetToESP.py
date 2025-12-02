# Example usage of the ESP TailLink server
# Gets data from universe zero, and sends it over to taillink!
# Channels will be interpreted as RGB values (in that order). 64 pixels are expected, for 192 channels total.
# Other packet types are currently unimplimented.
from stupidArtnet import StupidArtnetServer
import time
import subprocess
import serial

UNIVERSE = 0
PORT = "/dev/ttyACM0"
BAUD_RATE = 460800

packet = b""
lastPacketRx = time.time()


def rcv_artnet(data):
    """Test function to receive callback data."""
    # the received data is an array
    # of the channels value (no headers)
    global packet
    packet = bytes(b"\x01") + bytes(data[0:192])


try:
    ser = serial.Serial(port=PORT, baudrate=BAUD_RATE, timeout=1)
    ser.flushInput()

except serial.SerialException as e:
    print(f"FATAL: Could not open serial port {PORT}: {e}", file=sys.stderr)
    sys.exit(1)

srv = StupidArtnetServer()
u1_listener = srv.register_listener(UNIVERSE, callback_function=rcv_artnet)

# do nothing forever
while True:
    ser.write(packet)
    print(ser.readline().decode("utf-8"), end="")
    time.sleep(0.033)
