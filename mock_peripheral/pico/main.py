# SmartAquaria — Pico EZO Sensor Mock
#
# Simulates two Atlas Scientific EZO sensors over hardware UART:
#   UART0 (GPIO0 TX / GPIO1 RX) — EZO-RTD  (temperature)
#   UART1 (GPIO4 TX / GPIO5 RX) — EZO-DO   (dissolved oxygen)
#
# Wiring to RPi4:
#   Pico GPIO0 (TX) ──► RPi4 GPIO5  (ttyAMA3 RX)
#   Pico GPIO1 (RX) ◄── RPi4 GPIO4  (ttyAMA3 TX)
#   Pico GPIO4 (TX) ──► RPi4 GPIO13 (ttyAMA5 RX)
#   Pico GPIO5 (RX) ◄── RPi4 GPIO12 (ttyAMA5 TX)
#   GND             ──── GND
#
# Protocol: host sends "R\r\n" or "R\n", sensor replies "<value>\r\n"

from machine import UART, Pin

# ---------------------------------------------------------------------------
# Scenario data (matches mock_peripheral/sensors/)
# ---------------------------------------------------------------------------

TEMP_SCENARIO = [
    # 1-30: Stable normal (6s)
    25.0, 25.5, 24.5, 26.0, 25.0, 24.0, 25.5, 26.0, 25.0, 24.5,
    25.0, 25.5, 24.5, 26.0, 25.0, 24.0, 25.5, 26.0, 25.0, 24.5,
    25.0, 25.5, 24.5, 26.0, 25.0, 24.0, 25.5, 26.0, 25.0, 24.5,
    # 31-45: Heating up → Warning (3s) [Warning after 3× >30]
    27.0, 27.5, 28.0, 28.5, 29.0, 29.5, 30.0, 30.5, 31.0, 31.5, 32.0, 32.5, 33.0, 33.5, 33.0,
    # 46-57: Alarm high temp (2.4s) [Alarm after 3× >34]
    34.5, 35.0, 35.5, 36.0, 36.5, 37.0, 37.5, 38.0, 38.0, 38.0, 37.5, 37.0,
    # 58-69: Recovery → Normal (2.4s)
    34.0, 32.5, 31.0, 29.5, 28.0, 26.5, 25.5, 25.0, 25.0, 25.0, 25.0, 25.0,
    # 70-84: Normal (3s)
    25.0, 25.0, 25.5, 25.0, 24.5, 25.0, 25.0, 25.5, 25.0, 24.5, 25.0, 25.5, 25.0, 24.5, 25.0,
    # 85-96: Normal (DO dropping) (2.4s)
    25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0,
    # 97-111: Normal (DO Alarm) (3s)
    25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0,
    # 112-126: Normal (DO recovery) (3s)
    25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0,
    # 127-138: Normal (2.4s)
    25.0, 25.5, 25.0, 24.5, 25.0, 25.5, 25.0, 24.5, 25.0, 25.5, 25.0, 24.5,
    # 139-153: Cooling → Warning (3s) [Warning after 3× <20]
    22.0, 21.5, 21.0, 20.5, 20.0, 19.5, 19.0, 18.5, 18.0, 17.5, 17.0, 16.5, 16.5, 16.5, 16.5,
    # 154-165: Alarm cold (2.4s) [Alarm after 3× <16]
    15.5, 15.0, 14.5, 14.0, 13.5, 13.0, 13.0, 13.0, 13.5, 14.0, 14.5, 15.0,
    # 166-177: Recovery cold → Normal (2.4s)
    16.5, 17.5, 19.0, 21.0, 22.5, 24.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0,
    # 178-189: Normal (2.4s)
    25.0, 25.5, 25.0, 24.5, 25.0, 25.5, 25.0, 24.5, 25.0, 25.5, 25.0, 24.5,
    # 190-207: Both sensors degrading (3.6s)
    27.0, 28.0, 29.0, 30.0, 30.5, 31.0, 31.5, 32.0, 32.5, 33.0, 33.5, 33.8, 34.5, 35.0, 35.5, 36.0, 37.0, 38.0,
    # 208-216: Combined Alarm (1.8s)
    38.0, 38.0, 38.0, 38.0, 38.0, 38.0, 38.0, 38.0, 38.0,
    # 217-231: Recovery (3s)
    35.0, 32.0, 29.5, 27.5, 26.0, 25.5, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0,
]

DO_SCENARIO = [
    # 1-30: Stable normal (6s)
    7.5, 7.2, 7.8, 7.0, 7.5, 7.3, 7.6, 7.0, 7.5, 7.2,
    7.5, 7.2, 7.8, 7.0, 7.5, 7.3, 7.6, 7.0, 7.5, 7.2,
    7.5, 7.2, 7.8, 7.0, 7.5, 7.3, 7.6, 7.0, 7.5, 7.2,
    # 31-45: Normal (temp heating) (3s)
    7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5,
    # 46-57: Normal (temp Alarm) (2.4s)
    7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5,
    # 58-69: Normal (temp recovery) (2.4s)
    7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5,
    # 70-84: Normal (3s)
    7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5,
    # 85-96: DO dropping → Warning (2.4s) [Warning after 3× <6.0]
    7.0, 6.8, 6.5, 6.3, 6.1, 5.9, 5.8, 5.7, 5.6, 5.5, 5.5, 5.5,
    # 97-111: DO Alarm (3s) [Alarm after 3× <5.0]
    4.9, 4.7, 4.5, 4.3, 4.1, 3.9, 3.8, 3.7, 3.6, 3.5, 3.5, 3.5, 3.5, 3.5, 3.5,
    # 112-126: DO recovery (3s)
    4.8, 5.2, 5.6, 6.0, 6.5, 7.0, 7.3, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5,
    # 127-138: Normal (2.4s)
    7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5,
    # 139-153: Normal (temp cooling) (3s)
    7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5,
    # 154-165: Normal (temp cold Alarm) (2.4s)
    7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5,
    # 166-177: Normal (temp cold recovery) (2.4s)
    7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5,
    # 178-189: Normal (2.4s)
    7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5,
    # 190-207: Both sensors degrading — DO falling (3.6s)
    7.0, 6.5, 6.0, 5.8, 5.5, 5.3, 5.1, 4.9, 4.7, 4.5, 4.4, 4.3, 4.2, 4.1, 4.0, 3.9, 3.8, 3.8,
    # 208-216: Combined Alarm (1.8s)
    3.8, 3.8, 3.8, 3.8, 3.8, 3.8, 3.8, 3.8, 3.8,
    # 217-231: Recovery (3s)
    4.5, 5.0, 5.5, 6.0, 6.5, 7.0, 7.3, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5, 7.5,
]

# ---------------------------------------------------------------------------
# Cyclic iterator (no itertools in MicroPython)
# ---------------------------------------------------------------------------

class Cycle:
    def __init__(self, data):
        self._data = data
        self._i = 0

    def next(self):
        val = self._data[self._i]
        self._i = (self._i + 1) % len(self._data)
        return val

# ---------------------------------------------------------------------------
# UART setup
# ---------------------------------------------------------------------------

BAUDRATE = 9600

uart_temp = UART(0, baudrate=BAUDRATE, tx=Pin(0), rx=Pin(1))
uart_do   = UART(1, baudrate=BAUDRATE, tx=Pin(4), rx=Pin(5))

temp_cycle = Cycle(TEMP_SCENARIO)
do_cycle   = Cycle(DO_SCENARIO)

buf_temp = b""
buf_do   = b""

# ---------------------------------------------------------------------------
# Main loop
# ---------------------------------------------------------------------------

print("SmartAquaria Pico mock started")

while True:
    try:
        # --- Temp sensor ---
        if uart_temp.any():
            buf_temp += uart_temp.read(uart_temp.any())
            while b"\n" in buf_temp:
                line, buf_temp = buf_temp.split(b"\n", 1)
                cmd = line.strip().decode("utf-8", "ignore")
                if cmd == "R":
                    val = temp_cycle.next()
                    uart_temp.write("{:.2f}\r\n".format(val).encode())
                    print("TEMP <- R  ->", val)
                elif cmd:
                    print("TEMP unknown:", repr(cmd))

        # --- DO sensor ---
        if uart_do.any():
            buf_do += uart_do.read(uart_do.any())
            while b"\n" in buf_do:
                line, buf_do = buf_do.split(b"\n", 1)
                cmd = line.strip().decode("utf-8", "ignore")
                if cmd == "R":
                    val = do_cycle.next()
                    uart_do.write("{:.2f}\r\n".format(val).encode())
                    print("DO   <- R  ->", val)
                elif cmd:
                    print("DO   unknown:", repr(cmd))

    except Exception as e:
        print("ERROR:", e)
        buf_temp = b""
        buf_do   = b""
