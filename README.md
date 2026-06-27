# SmartAquaria

> Aquarium monitoring and control system — temperature, dissolved oxygen, buzzer, heater, ST7789 display.  
> Targets: **Linux** (Raspberry Pi 4) and **VxWorks 7 DKM**.

---

## Prerequisites

<details>
<summary><strong>Linux</strong></summary>

- CMake ≥ 3.20
- GCC or Clang (C++17)
- Python 3.11+
- `socat` &nbsp;—&nbsp; virtual UART pairs &nbsp;(`sudo apt install socat`)
- `gpio-sim` kernel module &nbsp;—&nbsp; mock GPIO &nbsp;(`sudo modprobe gpio-sim`)

</details>

<details>
<summary><strong>VxWorks DKM</strong></summary>

- Wind River SDK with `WRSDK_HOME` set
- CMake ≥ 3.20 (bundled in SDK)

</details>

---

## Quick Start

```bash
cd development
make launch          # full mock, debug build
```

Builds the binary, starts virtual UARTs + gpio-sim, launches mock peripherals, then runs the app.  
Logs land in `log/`. Press `Ctrl+C` to stop everything cleanly.

| Log file            | Contents                |
| ------------------- | ----------------------- |
| `log/main.log`      | Application output      |
| `log/sensor.log`    | Sensor mock             |
| `log/actuator.log`  | Actuator mock           |
| `log/display.log`   | Terminal display output |

---

## Configs

All config files live in `config/`:

| File                          | What runs                                          |
| ----------------------------- | -------------------------------------------------- |
| `config_full_mock.toml`       | Everything simulated — virtual UARTs, gpio-sim, terminal display |
| `config_terminal.toml`        | Mock sensors · terminal display · no actuator mock |
| `config_sensors_mock.toml`    | Mock sensors · real GPIO actuators · terminal display |
| `config_rpi4_hardware.toml`   | Full hardware on Raspberry Pi 4 (Linux)            |
| `config_rpi4_vxworks_dkm.toml`| Full hardware on Raspberry Pi 4 (VxWorks DKM)     |

```bash
make launch CONFIG=config/config_rpi4_hardware.toml
make launch CONFIG=config/config_full_mock.toml BUILD_TYPE=release
```

Or directly:

```bash
python3 -m aquarunner config/config_full_mock.toml [debug|release]
```

Key config knobs:

```toml
[mock]
sensor   = true    # virtual UART + sensor_peripheral
actuator = true    # gpio-sim + actuator_peripheral

[display]
type = "terminal"  # "terminal" | "st7789"

[thresholds.temp]
min         = 20   # heater on below this (°C)
max         = 30
critical_lo = 16   # alarm
critical_hi = 34   # alarm

[thresholds.do]
warning  = 6.0     # warning below this (mg/L)
critical = 5.0     # alarm

[intervals]
sampling_seconds = 0.2     # supports float (200 ms)
feeding_seconds  = 28800   # 8 h
```

---

## Build

```bash
# Linux debug / release
cmake --preset linux-debug   -S smart_aquaria && cmake --build --preset linux-debug
cmake --preset linux-release -S smart_aquaria && cmake --build --preset linux-release

# VxWorks DKM  (requires WRSDK_HOME)
cmake --preset vxworks-dkm-debug   -S smart_aquaria && cmake --build --preset vxworks-dkm-debug
cmake --preset vxworks-dkm-release -S smart_aquaria && cmake --build --preset vxworks-dkm-release
```

Output binaries:

| Preset              | Binary                                                                            |
| ------------------- | --------------------------------------------------------------------------------- |
| `linux-debug`       | `smart_aquaria/build/linux-debug/configurator/linux/smart_aquaria_linux`          |
| `linux-release`     | `smart_aquaria/build/linux-release/configurator/linux/smart_aquaria_linux`        |
| `vxworks-dkm-debug` | `smart_aquaria/build/vxworks-dkm-debug/configurator/vxworks_dkm/smart_aquaria_vxworks_dkm.out` |

---

## Tests

```bash
cmake --preset linux-debug -S smart_aquaria
cmake --build --preset linux-debug
ctest --preset linux-debug --output-on-failure
```

---

## Running the Binary Directly

**Linux:**

```bash
./smart_aquaria/build/linux-debug/configurator/linux/smart_aquaria_linux \
    config/config_rpi4_hardware.toml
```

**VxWorks DKM** — copy `.out` + config to the target (e.g. `/sd0a`), then from the shell:

```
-> ld < /sd0a/smart_aquaria_vxworks_dkm.out
-> smart_aquaria_vxworks_dkm "/sd0a/config_rpi4_vxworks_dkm.toml"
```

If no argument is given, the binary looks for `./config_rpi4_vxworks_dkm.toml` in the current directory.

---

## Hardware Wiring — Raspberry Pi 4

```
ST7789 (SPI0)                     Sensors
─────────────────────────         ──────────────────────────────────
VCC  →  3.3 V   (Pin  1)          EZO-RTD  TX → GPIO4  (Pin  7)
GND  →  GND     (Pin  6)                   RX → GPIO5  (Pin 29)
SCL  →  GPIO11  (Pin 23)  SCLK             /dev/ttyAMA3  (dtoverlay=uart3)
SDA  →  GPIO10  (Pin 19)  MOSI
RES  →  GPIO27  (Pin 13)  Reset   EZO-DO   TX → GPIO12 (Pin 32)
DC   →  GPIO25  (Pin 22)  D/C              RX → GPIO13 (Pin 33)
CS   →  GPIO8   (Pin 24)  CE0              /dev/ttyAMA5  (dtoverlay=uart5)

Actuators
─────────────────────────
Buzzer  →  GPIO17  (Pin 11)
Heater  →  GPIO18  (Pin 12)
```

Add to `/boot/firmware/config.txt` and reboot:

```
dtoverlay=uart3
dtoverlay=uart5
```

Enable SPI: `raspi-config` → Interface Options → SPI.

**VxWorks UART mapping:**

| VxBus device    | Node      | Sensor   |
| --------------- | --------- | -------- |
| `pl01x-uart-0`  | `/ttyS1`  | EZO-RTD  |
| `pl01x-uart-1`  | `/ttyS2`  | EZO-DO   |

> `/ttyS0` is the debug console — do not connect sensors to it.

---

## Clean

```bash
make clean   # removes smart_aquaria/build/
```
