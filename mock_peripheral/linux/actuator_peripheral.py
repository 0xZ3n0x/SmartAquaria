#!/usr/bin/env python3

import datetime
import os
import signal
import sys
import tomllib
from collections.abc import Callable

from actuators.buzzer import BuzzerActuator


def make_logger(log_path: str) -> Callable[[str], None]:
    log_file = open(log_path, "a")

    def log(msg: str) -> None:
        line = f"{datetime.datetime.now().isoformat()} - {msg}\n"
        log_file.write(line)
        log_file.flush()

    return log


def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <config_path>")
        sys.exit(1)

    config_path = sys.argv[1]
    with open(config_path, "rb") as f:
        config = tomllib.load(f)

    chip_path   = config["gpio"]["chip"]
    buzzer_line = config["gpio"]["buzzer"]["line"]
    chip_name   = os.path.basename(chip_path)
    log_path    = config["log"]["actuator"]

    log = make_logger(log_path)
    log(f"Actuator peripheral starting (chip={chip_name}, buzzer_line={buzzer_line})...")

    buzzer = BuzzerActuator(chip_name, buzzer_line, log=log)

    def _on_sigterm(signum, frame):
        raise KeyboardInterrupt()

    signal.signal(signal.SIGTERM, _on_sigterm)

    try:
        buzzer.run()
    except KeyboardInterrupt:
        log("Shutting down...")
    finally:
        buzzer.stop()


if __name__ == "__main__":
    main()
