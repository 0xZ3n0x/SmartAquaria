import subprocess

_CONFIG_FS_BASE = "/sys/kernel/config/gpio-sim"
_SIM_NAME       = "aquaria"
_BANK_NAME      = "gpio-bank0"


class GpioSim:
    @staticmethod
    def setup(num_lines: int, line_names: list[str]) -> str:
        """Load gpio-sim, create virtual chip, return chip device path (e.g. /dev/gpiochip4)."""
        if len(line_names) != num_lines:
            raise ValueError("len(line_names) must equal num_lines")

        print("Loading gpio-sim module...")
        _sudo("modprobe", "gpio-sim")

        sim_dir  = f"{_CONFIG_FS_BASE}/{_SIM_NAME}"
        bank_dir = f"{sim_dir}/{_BANK_NAME}"

        if _exists_root(f"{sim_dir}/live"):
            _write(f"{sim_dir}/live", "0")
        for i in range(num_lines):
            _rmdir(f"{bank_dir}/line{i}")
        _rmdir(bank_dir)
        _rmdir(sim_dir)

        for i in range(num_lines):
            _mkdir(f"{bank_dir}/line{i}")

        _write(f"{bank_dir}/num_lines", str(num_lines))
        for i, name in enumerate(line_names):
            _write(f"{bank_dir}/line{i}/name", name)

        _write(f"{sim_dir}/live", "1")

        result = subprocess.run(
            ["sudo", "cat", f"{bank_dir}/chip_name"],
            capture_output=True, text=True,
        )
        if result.returncode != 0 or not result.stdout.strip():
            raise RuntimeError("Failed to read chip_name from gpio-sim configfs")

        chip_name = result.stdout.strip()
        chip_dev  = f"/dev/{chip_name}"
        _sudo("chmod", "a+rw", chip_dev)
        print(f"gpio-sim chip: {chip_dev}  ({num_lines} lines: {line_names})")
        return chip_dev

    @staticmethod
    def teardown() -> None:
        """Remove gpio-sim configfs entries."""
        sim_dir  = f"{_CONFIG_FS_BASE}/{_SIM_NAME}"
        bank_dir = f"{sim_dir}/{_BANK_NAME}"

        if _exists_root(f"{sim_dir}/live"):
            _write(f"{sim_dir}/live", "0")

        i = 0
        while _exists_root(f"{bank_dir}/line{i}"):
            _rmdir(f"{bank_dir}/line{i}")
            i += 1

        _rmdir(bank_dir)
        _rmdir(sim_dir)
        print("gpio-sim torn down.")


# ---------------------------------------------------------------------------
# Module-level helpers (private)
# ---------------------------------------------------------------------------

def _sudo(*args: str) -> None:
    result = subprocess.run(["sudo"] + list(args))
    if result.returncode != 0:
        raise RuntimeError(f"Command failed: sudo {' '.join(args)}")


def _write(path: str, value: str) -> None:
    result = subprocess.run(
        ["sudo", "tee", path],
        input=value + "\n",
        text=True,
        capture_output=True,
    )
    if result.returncode != 0:
        raise RuntimeError(f"Failed to write '{value}' to {path}")


def _mkdir(path: str) -> None:
    _sudo("mkdir", "-p", path)


def _rmdir(path: str) -> None:
    subprocess.run(["sudo", "rmdir", path], capture_output=True)


def _exists_root(path: str) -> bool:
    return subprocess.run(["sudo", "test", "-e", path], capture_output=True).returncode == 0
