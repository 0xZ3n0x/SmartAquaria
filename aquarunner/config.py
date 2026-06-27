import re
import tomllib


def load(path: str) -> dict:
    with open(path, "rb") as f:
        return tomllib.load(f)


def patch_gpio_chip(path: str, chip_path: str) -> None:
    """Update gpio.chip in config.toml without touching other content."""
    with open(path) as f:
        content = f.read()

    new_content = re.sub(
        r'(chip\s*=\s*)"[^"]*"',
        rf'\1"{chip_path}"',
        content,
    )

    with open(path, "w") as f:
        f.write(new_content)
