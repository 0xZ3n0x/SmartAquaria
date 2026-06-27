import subprocess
import sys
from pathlib import Path

from aquarunner.config          import load, patch_gpio_chip
from aquarunner.hw.gpio_sim     import GpioSim
from aquarunner.hw.virtual_uart import VirtualUart
from aquarunner.process_manager import ProcessManager

DEV_DIR       = Path(__file__).parent.parent
SMART_AQUARIA = DEV_DIR / "smart_aquaria"
MOCK_DIR      = DEV_DIR / "mock_peripheral"


def build(build_type: str) -> None:
    preset = f"linux-{build_type}"
    print(f"[build] Configuring preset {preset}...")
    result = subprocess.run(
        ["cmake", "--preset", preset],
        cwd=SMART_AQUARIA,
    )
    if result.returncode != 0:
        print("Configure failed.", file=sys.stderr)
        sys.exit(1)

    print(f"[build] Building preset {preset}...")
    result = subprocess.run(
        ["cmake", "--build", "--preset", preset],
        cwd=SMART_AQUARIA,
    )
    if result.returncode != 0:
        print("Build failed.", file=sys.stderr)
        sys.exit(1)


def main() -> None:
    if len(sys.argv) < 2:
        print("Usage: python3 -m aquarunner <config_path> [debug|release]")
        sys.exit(1)

    config_path = Path(sys.argv[1])
    if not config_path.is_absolute():
        config_path = Path.cwd() / config_path

    build_type = sys.argv[2] if len(sys.argv) >= 3 else "debug"

    build(build_type)

    cfg      = load(str(config_path))
    ports    = cfg["serial"]["ports"]
    mock_cfg = cfg.get("mock", {})

    sensor_mock   = mock_cfg.get("sensor",   False)
    actuator_mock = mock_cfg.get("actuator", False)

    sensor_script   = str(MOCK_DIR / "linux" / "sensor_peripheral.py")
    actuator_script = str(MOCK_DIR / "linux" / "actuator_peripheral.py")
    main_executable = str(SMART_AQUARIA / "build" / f"linux-{build_type}" / "configurator" / "linux" / "smart_aquaria_linux")

    chip_dev:   str | None               = None
    socat_temp: subprocess.Popen | None  = None
    socat_do:   subprocess.Popen | None  = None
    procs:      dict[str, subprocess.Popen] = {}

    try:
        if actuator_mock:
            chip_dev = GpioSim.setup(*_gpio_layout(cfg))
            patch_gpio_chip(str(config_path), chip_dev)

        if sensor_mock:
            socat_temp = VirtualUart.start(ports["temp"]["mock"], ports["temp"]["main"])
            socat_do   = VirtualUart.start(ports["do"]["mock"],   ports["do"]["main"])
            procs["sensor_peripheral"] = ProcessManager.start(
                [sys.executable, sensor_script, str(config_path)], "sensor_peripheral"
            )

        if actuator_mock:
            procs["actuator_peripheral"] = ProcessManager.start(
                [sys.executable, actuator_script, str(config_path)], "actuator_peripheral"
            )

        procs["smart_aquaria"] = ProcessManager.start(
            [main_executable, str(config_path)], "smart_aquaria"
        )

        ProcessManager.run_until_exit(procs)

    finally:
        for name, proc in procs.items():
            ProcessManager.stop(proc, name)
        if socat_do:   VirtualUart.stop(socat_do)
        if socat_temp: VirtualUart.stop(socat_temp)
        if chip_dev:   GpioSim.teardown()


def _gpio_layout(cfg: dict) -> tuple[int, list[str]]:
    gpio_cfg    = cfg["gpio"]
    buzzer_line = gpio_cfg["buzzer"]["line"]
    heater_line = gpio_cfg["heater"]["line"]
    num_lines   = max(buzzer_line, heater_line) + 1
    line_names  = [""] * num_lines
    line_names[buzzer_line] = "buzzer"
    line_names[heater_line] = "heater"
    return num_lines, line_names


main()
