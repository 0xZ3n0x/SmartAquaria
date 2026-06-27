#!/usr/bin/env python3

import datetime
import os
import signal
import sys
import threading
import tomllib
from collections.abc import Callable, Sequence

import serial

from base import Peripheral
from sensors.temperature import TemperatureSensor
from sensors.do import DOSensor


def make_logger(log_path: str) -> Callable[[str], None]:
    log_file = open(log_path, "a")

    def log(msg: str) -> None:
        line = f"{datetime.datetime.now().isoformat()} - {msg}\n"
        log_file.write(line)
        log_file.flush()

    return log


def run_peripheral_loop(
    ser: serial.Serial,
    peripherals: Sequence[Peripheral],
    log: Callable[[str], None],
    label: str = "",
) -> None:
    """Read queries from serial and dispatch to registered peripherals.

    Iterates through peripherals in order; the first peripheral that returns
    a non-None response handles the query. Unrecognised queries are logged.
    """
    prefix = f"[{label}] " if label else ""
    try:
        while True:
            data = ser.readline()
            if not data:
                continue

            message = data.decode("utf-8", errors="ignore").strip()
            if not message:
                continue

            handled = False
            for peripheral in peripherals:
                response = peripheral.handle_query(message)
                if response is not None:
                    ser.write(f"{response}\n".encode("utf-8"))
                    log(f"{prefix}Query '{message}' -> '{response}'")
                    handled = True
                    break

            if not handled:
                log(f"{prefix}Unknown query: {message}")

    except (serial.SerialException, OSError) as e:
        log(f"Port closed: {e}")
    finally:
        ser.close()


def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <config_path>")
        sys.exit(1)

    config_path = sys.argv[1]
    with open(config_path, "rb") as f:
        config = tomllib.load(f)

    baudrate  = config["serial"]["baudrate"]
    timeout   = config["serial"]["timeout"]
    temp_port = config["serial"]["ports"]["temp"]["mock"]
    do_port   = config["serial"]["ports"]["do"]["mock"]
    log_path  = config["log"]["sensor"]

    log = make_logger(log_path)
    log("Sensor peripheral starting...")

    try:
        temp_ser = serial.Serial(port=temp_port, baudrate=baudrate, timeout=timeout)
    except Exception as e:
        log(f"Failed to open temp port {temp_port}: {e}")
        sys.exit(1)

    try:
        do_ser = serial.Serial(port=do_port, baudrate=baudrate, timeout=timeout)
    except Exception as e:
        log(f"Failed to open DO port {do_port}: {e}")
        temp_ser.close()
        sys.exit(1)

    log(f"Temp port: {temp_port}, DO port: {do_port} at {baudrate} baud")

    def _on_sigterm(signum, frame):
        raise KeyboardInterrupt()

    signal.signal(signal.SIGTERM, _on_sigterm)

    do_thread = threading.Thread(
        target=run_peripheral_loop,
        args=(do_ser, [DOSensor()], log, "DO"),
        daemon=True,
        name="do-peripheral",
    )
    do_thread.start()

    try:
        run_peripheral_loop(temp_ser, [TemperatureSensor()], log, "TEMP")
    except KeyboardInterrupt:
        log("Shutting down...")
    finally:
        do_ser.close()


if __name__ == "__main__":
    main()
