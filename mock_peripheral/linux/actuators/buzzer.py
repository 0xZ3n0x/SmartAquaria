import os
import subprocess
import time
from collections.abc import Callable


def find_sim_value_path(chip_name: str, line_offset: int) -> str:
    """Resolve the gpio-sim sysfs value path for a given chip and line.

    gpio-sim exposes each line as sim_gpioN under the chip's platform device
    directory.  The chip sysfs entry is a symlink under /sys/bus/gpio/devices/;
    following it leads to the platform device (e.g. gpio-sim.0/gpiochip4).
    """
    chip_sys  = f"/sys/bus/gpio/devices/{chip_name}"
    real      = os.path.realpath(chip_sys)          # .../platform/gpio-sim.0/gpiochipN
    platform  = os.path.dirname(real)               # .../platform/gpio-sim.0
    return os.path.join(platform, chip_name, f"sim_gpio{line_offset}", "value")


class BuzzerActuator:
    """Observes the gpio-sim sysfs value for the buzzer line.

    When the driven value is 1 (C++ set the GPIO HIGH) a square-wave tone is
    produced via ``play``.  When the value returns to 0 the sound is stopped.
    """

    POLL_INTERVAL = 0.05   # seconds
    FREQUENCY     = 880    # Hz

    def __init__(self, chip_name: str, line_offset: int,
                 log: Callable[[str], None] | None = None) -> None:
        self._value_path  = find_sim_value_path(chip_name, line_offset)
        self._play_proc: subprocess.Popen | None = None
        self._running     = True
        self._log         = log

    # ------------------------------------------------------------------
    # Internal helpers
    # ------------------------------------------------------------------

    def _read_value(self) -> int:
        try:
            with open(self._value_path) as f:
                return int(f.read().strip())
        except OSError:
            return 0

    def _start_sound(self) -> None:
        if self._play_proc is not None:
            return
        self._play_proc = subprocess.Popen(
            ["play", "-n", "-q", "synth", "sq", str(self.FREQUENCY)],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )

    def _stop_sound(self) -> None:
        if self._play_proc is None:
            return
        self._play_proc.terminate()
        try:
            self._play_proc.wait(timeout=1)
        except subprocess.TimeoutExpired:
            self._play_proc.kill()
        self._play_proc = None

    # ------------------------------------------------------------------
    # Public interface
    # ------------------------------------------------------------------

    def stop(self) -> None:
        """Signal the run loop to exit and immediately kill any playing sound."""
        self._running = False
        self._stop_sound()

    def run(self) -> None:
        """Poll loop — intended to run in a dedicated thread."""
        last_value = -1
        try:
            while self._running:
                value = self._read_value()
                if value != last_value:
                    if value:
                        if self._log: self._log("Buzzer ON")
                        self._start_sound()
                    else:
                        if self._log: self._log("Buzzer OFF")
                        self._stop_sound()
                    last_value = value
                time.sleep(self.POLL_INTERVAL)
        finally:
            self._stop_sound()
