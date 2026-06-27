# SmartAquaria - AGENTS.md

## Project Structure

```
development/
├── Makefile
├── config/                   # Configuration files
│   ├── config_full_mock.toml     # Full mock (all hardware simulated, terminal display)
│   ├── config_sensors_mock.toml # Sensors mock, real display (ST7789) + real actuator
│   ├── config_rpi4_hardware.toml # Full hardware (Raspberry Pi 4)
│   └── config_terminal.toml     # Terminal display + sensor mock (no hardware)
├── log/                     # Log files (ignored in git)
├── aquarunner/               # System bootstrapper and orchestration
│   ├── __init__.py
│   ├── __main__.py           # Entry: python3 -m aquarunner <config> [debug|release]
│   ├── config.py             # Config load + gpio chip path patch
│   ├── process_manager.py    # ProcessManager static helpers (start/stop/monitor)
│   └── hw/
│       ├── gpio_sim.py       # GpioSim static helpers (gpio-sim kernel module)
│       └── virtual_uart.py   # VirtualUart static helpers (socat)
├── mock_peripheral/          # Mock sensor/actuator simulations
│   ├── linux/               # Linux mock peripherals
│   │   ├── base.py               # Peripheral abstract base class
│   │   ├── sensor_peripheral.py  # Sensor process (temp + DO sensors)
│   │   ├── actuator_peripheral.py
│   │   ├── sensors/
│   │   │   ├── temperature.py    # 81-step temperature scenario
│   │   │   └── do.py             # 81-step DO scenario
│   │   └── actuators/
│   │       └── buzzer.py         # Buzzer actuator (gpio-sim sysfs observer)
│   └── pico/                # Raspberry Pi Pico mock
└── smart_aquaria/
    ├── CMakeLists.txt        # Root CMake — adds osal, hal, core, configurator, test
    ├── build.py              # Build script: python3 build.py linux [debug|release]
    ├── osal/
    │   ├── CMakeLists.txt    # adds interface; if(Linux) adds linux
    │   ├── interface/        # OSAL interfaces — CMake INTERFACE target: SmartAquaria_OSAL
    │   │   ├── CMakeLists.txt
    │   │   ├── IClock.h
    │   │   ├── IEventQueue.h
    │   │   ├── ILogger.h
    │   │   └── ITimerService.h
    │   └── stl/              # CMake STATIC target: SmartAquaria_OSAL_STL
    │       ├── CMakeLists.txt
    │       ├── clock/Clock.h/.cpp
    │       ├── event_queue/EventQueue.h/.cpp
    │       ├── logger/Logger.h/.cpp
    │       └── timer/TimerService.h/.cpp
    ├── hal/
    │   ├── CMakeLists.txt    # adds interface; if(Linux) adds linux
    │   ├── interface/        # HAL interfaces — CMake INTERFACE target: SmartAquaria_HAL
    │   │   ├── CMakeLists.txt
    │   │   ├── IGPIO.h
    │   │   ├── ISPI.h
    │   │   └── IUART.h
    │   ├── linux/            # CMake STATIC target: SmartAquaria_HAL_Linux
    │   │   ├── CMakeLists.txt
    │   │   ├── gpio/GPIO.h/.cpp
    │   │   ├── spi/SPI.h/.cpp
    │   │   └── uart/UART.h/.cpp
    │   └── vxworks/
    │       └── dkm/          # VxWorks DKM HAL implementations
    │           ├── gpio/GPIO.h/.cpp
    │           ├── spi/SPI.h/.cpp
    │           └── uart/UART.h/.cpp
    ├── core/                 # CMake STATIC target: SmartAquaria_Core
    │   ├── CMakeLists.txt
    │   ├── adapter/
    │   │   ├── actuator/ActuatorAdapter.h/.cpp
    │   │   ├── display/DisplayAdapter.h/.cpp
    │   │   └── sensor/SensorAdapter.h/.cpp
    │   ├── app/SmartAquaria.h/.cpp
    │   ├── device/
    │   │   ├── actuator/buzzer/IBuzzer.h, BuzzerDevice.h/.cpp
    │   │   │               heater/IHeater.h, HeaterDevice.h/.cpp
    │   │   ├── display/IDisplayDevice.h
    │   │   │           st7789/ST7789DisplayDevice.h/.cpp, Font8x8.hpp
    │   │   └── sensor/temp/ITemperatureSensor.h, EzoRTDSensorDevice.h/.cpp
    │   │                  do/IDOSensor.h, EzoDOSensorDevice.h/.cpp
    │   ├── domain/
    │   │   ├── AquariaState.h/.cpp
    │   │   ├── IActuator.h, IDisplay.h, ISensor.h
    │   │   ├── Readings.h
    │   │   └── Thresholds.hpp
    │   └── renderer/IDisplayRenderer.h, DisplayRenderer.h/.cpp
    ├── configurator/
    │   ├── CMakeLists.txt
    │   ├── common/           # Shared: AppRunner.hpp/.cpp, TerminalDisplayDevice.h/.cpp
    │   ├── linux/            # Linux executable: smart_aquaria_linux
    │   │   ├── CMakeLists.txt
    │   │   ├── Config.hpp
    │   │   └── main.cpp
    │   └── vxworks_dkm/      # VxWorks DKM executable
    │       ├── CMakeLists.txt
    │       ├── Config.hpp
    │       └── main.cpp
    └── test/                 # CMake subdirectory of root (not standalone)
        ├── CMakeLists.txt
        ├── BuzzerDeviceTest.cpp
        ├── EzoRTDSensorDeviceTest.cpp
        ├── ST7789DisplayDeviceTest.cpp
        ├── LinuxAquariaFSMTest.cpp   # Linux only
        ├── LinuxLoggerTest.cpp       # Linux only
        ├── LinuxSmartAquariaTest.cpp # Linux only
        └── TerminalDisplayDeviceTest.cpp # Linux only
```

## CMake Architecture

There is a single root `CMakeLists.txt`. Each library layer is a subdirectory.

```
CMakeLists.txt                  ← cmake -S . -B build/linux-debug
  ├── add_subdirectory(osal)
  │     ├── add_subdirectory(interface)   ← SmartAquaria_OSAL (INTERFACE)
  │     └── add_subdirectory(stl)         ← SmartAquaria_OSAL_STL (STATIC)
  ├── add_subdirectory(hal)
  │     ├── add_subdirectory(interface)   ← SmartAquaria_HAL (INTERFACE)
  │     ├── if(Linux) add_subdirectory(linux) ← SmartAquaria_HAL_Linux (STATIC)
  │     └── if(VxWorks) add_subdirectory(vxworks/dkm) ← SmartAquaria_HAL_VxWorks (STATIC)
  ├── add_subdirectory(core)              ← SmartAquaria_Core (STATIC)
  ├── add_subdirectory(configurator)
  │     ├── add_subdirectory(common)
  │     ├── if(Linux|Windows|Darwin) add_subdirectory(linux) ← smart_aquaria_linux exe
  │     └── if(VxWorks) add_subdirectory(vxworks_dkm) ← VxWorks DKM
  └── if(Linux|Windows|Darwin) add_subdirectory(test) ← smart_aquaria_tests exe
```

## Configuration

Runtime settings are stored in TOML config files passed as argument to aquarunner.
`aquarunner` reads `[mock]` flags to decide which processes to start:
- `mock.sensor = true` → start virtual UARTs + `sensor_peripheral` mock process
- `mock.actuator = true` → start gpio-sim + `actuator_peripheral` mock process
- `display.type = "terminal"` → use TerminalDisplayDevice (writes to `log.display` file)
- `display.type = "st7789"` → use ST7789DisplayDevice over SPI

```toml
[display]
type = "terminal"

[mock]
sensor   = true
actuator = true

[serial]
baudrate = 115200
timeout  = 1.0

[serial.ports.temp]
mock = "/tmp/vport0"
main = "/tmp/vport1"

[serial.ports.do]
mock = "/tmp/vport2"
main = "/tmp/vport3"

[log]
main     = "log/main.log"
sensor   = "log/sensor.log"
actuator = "log/actuator.log"
display  = "log/display.log"

[gpio]
chip = "/dev/gpiochip2"   # patched at runtime by aquarunner

[gpio.buzzer]
line = 0

[gpio.heater]
line = 1

[thresholds.temp]
min         = 20
max         = 30
critical_lo = 16
critical_hi = 34

[thresholds.do]
warning  = 6.0
critical = 5.0

[intervals]
sampling_seconds = 0.2
feeding_seconds  = 28800
```

## Build and Run

```bash
# Launch with full mock (builds automatically)
make launch

# Launch release build
make launch BUILD_TYPE=release

# Launch with specific config
make launch CONFIG=config/config_full_mock.toml

# Build only (Linux)
python3 smart_aquaria/build.py linux debug

# Run tests
cmake -S smart_aquaria -B smart_aquaria/build/linux-debug
cmake --build smart_aquaria/build/linux-debug --target smart_aquaria_tests
ctest --test-dir smart_aquaria/build/linux-debug

# Clean
make clean
```

## Log Location

All log files are written to the `log/` directory (relative to process working directory `development/`):
- `log/main.log`     — main program logs
- `log/sensor.log`   — sensor mock logs
- `log/actuator.log` — actuator mock logs
- `log/display.log`  — TerminalDisplayDevice output

## Code Standards

- **C++17** (`-std=c++17`)
- **CMake** build system (no root CMakeLists.txt — platform-specific roots)
- **Member variables**: `m_snake_case`
- **Brace style**: Allman (opening brace on new line)
- **Exceptions**: only when necessary (constructor failures, file open failures)
- **Return value**: `bool` for GPIO/SPI operations; `int` (bytes transferred) for UART `read`/`write`
- **RAII**: resource management in destructors
- **final, = default, delete**: used in class definitions
- **pragma once**: header guard
- **.hpp extension**: files with inline implementation (templates, constexpr tables)
- **Interface classes**: pure virtual with `I` prefix (`ILogger`, `IUART`, `ISensor`, etc.)
- **Include order**: standard library first (alphabetical), then project headers (alphabetical)
- **Mandatory dependencies**: constructor injection as references (`ISensor&`, `IActuator&`)
- **Optional dependencies**: setter injection (`setLogger()`, `setDisplay()`)
- **Warnings**: `-Wall -Wextra -Wpedantic` always enabled

## Architecture

### Layering

```
core/domain/     — pure interfaces and value types (no concrete deps)
  AquariaState, ISensor, IActuator, IDisplay, Readings, Thresholds

core/app/        — application logic (FSM, event loop)
  SmartAquaria   — FSM, event loop, state transitions

core/adapter/    — bridges domain interfaces to device interfaces
  SensorAdapter  — ISensor ← ITemperatureSensor + IDOSensor
  ActuatorAdapter — IActuator ← IBuzzer + IHeater
  DisplayAdapter — IDisplay ← IDisplayRenderer + IDisplayDevice

core/renderer/   — rendering logic (state→style, text formatting)
  DisplayRenderer

core/device/     — hardware device interfaces and drivers (depend on HAL interfaces)
  sensor/        — ITemperatureSensor, IDOSensor, EzoRTD, EzoDO
  actuator/      — IBuzzer, IHeater, BuzzerDevice, HeaterDevice
  display/       — IDisplayDevice, ST7789DisplayDevice

osal/interface/  — OSAL interfaces (IClock, ILogger, IEventQueue, ITimerService)
hal/interface/   — HAL interfaces (IGPIO, ISPI, IUART)

osal/stl/        — STL OSAL implementations (Clock, Logger, TimerService, EventQueue)
hal/linux/       — Linux HAL implementations (GPIO, SPI, UART)
hal/vxworks/dkm/ — VxWorks DKM HAL implementations (GPIO, SPI, UART)
configurator/common/ — Shared wiring: AppRunner, TerminalDisplayDevice
configurator/linux/  — Linux entry point + Config
configurator/vxworks_dkm/ — VxWorks DKM entry point + Config
```

### FSM States and Transitions

```
Hysteresis: kHysteresisUp=3 consecutive worse readings required to worsen state.
Recovery (improving): immediate on first good reading.

Normal  ──[K× warning]──▶ Warning ──[K× critical]──▶ Alarm
Normal  ──[K× critical]─────────────────────────────▶ Alarm
Warning ──[1× normal]───▶ Normal
Alarm   ──[1× normal]───▶ Normal
Alarm   ──[1× warning]──▶ Warning

Any Operational state ──[kFaultThreshold=5 consecutive failed reads]──▶ SensorFault
SensorFault ──[1× successful read]──▶ Normal

Entry actions: Alarm → buzzer.buzz(true)
               SensorFault → buzzer.buzz(true), heater.heat(false)
Exit actions:  Alarm → buzzer.buzz(false)
               SensorFault → buzzer.buzz(false)
Heater: pure thermostat in Operational states → heater.heat(temp < tempMin)
        Disabled in SensorFault (no valid reading available)
```

### Event Loop

```
AppRunner creates one TimerService externally (2 threads total):
  sampleTimer.run([&app]{ app.push(SmartAquaria::Event::SampleTimerElapsed); })

Feed timing is handled inside handleSample() via clock comparison — no separate timer thread.

TimerService owns its own thread (non-blocking run()); push() is thread-safe.
SmartAquaria::run() blocks — processes events sequentially from the queue.
All FSM logic runs in the single event loop thread.
```

## Classes

### SmartAquaria (core/app/SmartAquaria.h/cpp)

- `final` standalone FSM class
- **Nested types**: `enum class Event { SampleTimerElapsed }`
- **Constants**: `static constexpr int kHysteresisUp = 3` (consecutive worse readings to worsen state), `static constexpr int kFaultThreshold = 5` (consecutive failed reads → SensorFault)
- `SmartAquaria(ISensor&, IActuator&, const Thresholds&, IEventQueue&, IClock&, std::atomic<bool>&)` — mandatory deps by reference
- `void setLogger(std::unique_ptr<ILogger>)` — optional, transfers ownership
- `void setDisplay(std::unique_ptr<IDisplay>)` — optional, transfers ownership
- `void setFeedInterval(uint32_t ms)`
- `void run()` — blocking event loop
- `void stop()` — signals shutdown, unblocks queue
- `void push(Event event)` — thread-safe enqueue
- Private members: `m_worsen_count` (hysteresis counter), `m_fault_count` (consecutive failed reads counter)

### Readings (core/domain/Readings.h)

- Plain struct — snapshot of sensor and actuator state passed to display
- Members: `std::optional<float> temperature`, `std::optional<float> do_value`, `bool buzzer_on`, `bool heater_on`, `uint32_t time_until_feed_s`

### IDisplay (core/domain/IDisplay.h)

- `virtual void display(AquariaState state, const Readings& readings) = 0`

### DisplayAdapter (core/adapter/display/DisplayAdapter.h/cpp)

- `DisplayAdapter(std::unique_ptr<IDisplayRenderer>, std::unique_ptr<IDisplayDevice>)` — takes ownership via move
- `void display(AquariaState state, const Readings& readings) override` — delegates to renderer

### DisplayRenderer (core/renderer/DisplayRenderer.h/cpp)

- Stateless — no member variables
- `void render(AquariaState state, const Readings& readings, IDisplayDevice& device) const`
- Calls `device.clear()`, `device.print/printStyled()`, `device.flush()`

### IDisplayDevice (core/device/display/IDisplayDevice.h)

- `enum class DisplayStyle { Normal, Bold, Dim, Red, Yellow, Green, Cyan }`
- `clear()`, `print(text)`, `printStyled(text, style)`, `flush()` (default no-op)

### ST7789DisplayDevice (core/device/display/st7789/ST7789DisplayDevice.h/cpp)

- Inherits `IDisplayDevice`, `final`
- `ST7789DisplayDevice(ISPI&, IGPIO& dc, IGPIO& rst, IClock&)`
- **Tile buffer**: `m_tilebuf[16×16×2]` = 512 B (no full framebuffer)
- `drawChar()` — renders one 16×16 glyph into `m_tilebuf`, then immediately sends via `setAddressWindow` + SPI
- `clear()` — zeroes `m_tilebuf`, sends to full screen (225 × 512 B)
- `flush()` — no-op (data already sent during drawChar)
- Uses `Font8x8.hpp` (8×8 bitmap font, scaled 2×)
- SPI mode 3 (CPOL=1/CPHA=1)

### TerminalDisplayDevice (configurator/common/TerminalDisplayDevice.h/cpp)

- Inherits `IDisplayDevice`, `final`
- `explicit TerminalDisplayDevice(const char* filename)` — writes to file
- `clear()` — writes `'\n'` separator to file
- `printStyled()` — plain text in file mode

### BuzzerDevice / HeaterDevice (core/device/actuator/)

- `explicit BuzzerDevice(IGPIO& gpio)` / `HeaterDevice(IGPIO& gpio)` — takes reference
- `bool buzz(bool on)` / `bool heat(bool on)` — writes to GPIO

### EzoRTDSensorDevice / EzoDOSensorDevice (core/device/sensor/)

- `explicit EzoRTDSensorDevice(IUART& uart)` — takes reference
- `OptFloat getTemperature()` / `getDO()` — sends `R\n`, reads ASCII float, strips `\r\n`
- Member: `char m_buffer[256]` — read buffer

### SensorAdapter / ActuatorAdapter (core/adapter/)

- `SensorAdapter(ITemperatureSensor& temp, IDOSensor& doSensor)` — references
- `ActuatorAdapter(IBuzzer& buzzer, IHeater& heater)` — references

### Clock (osal/stl/clock/)

- `[[nodiscard]] uint64_t now_ms() const noexcept` — steady_clock since epoch set at construction
- `void sleep_ms(uint32_t ms) const`

### Logger (osal/stl/logger/)

- `explicit Logger(const char* filename)` — opens with `O_WRONLY|O_CREAT|O_TRUNC`
- `bool log(const char* prefix, const char* message)` — writes `prefix: message\n` via `write()`

### TimerService (osal/stl/timer/)

- `void run(std::function<void()> callback)` — starts internal thread
- `void setInterval(uint32_t ms)`, `void stop()`
- Members: `m_thread`, `m_mutex`, `m_cv`, `m_running` (atomic), `m_interval_ms` (atomic)

### EventQueue (osal/stl/event_queue/)

- `void push(int event)`, `bool wait(int& out)`, `void shutdown()`
- Members: `std::queue<int>`, `std::mutex`, `std::condition_variable`, `std::atomic<bool>`

### GPIO (hal/linux/gpio/ and hal/vxworks/dkm/gpio/)

- `GPIO(const std::string& chip, unsigned int line)` — Linux: GPIO character device; VxWorks: VxBus
- `bool write(bool active)`, `bool isOpen() const`, `void close()`

### SPI (hal/linux/spi/ and hal/vxworks/dkm/spi/)

- `SPI(const std::string& device, uint32_t speed_hz, uint8_t mode)` — opens spidev
- `bool transfer(const uint8_t* tx, uint8_t* rx, size_t len)`, `bool isOpen() const`, `void close()`

### UART (hal/linux/uart/ and hal/vxworks/dkm/uart/)

- `UART(const std::string& port, int baudrate, float timeout)`
- `int write(const char* data, size_t len)` — returns bytes written
- `int read(char* buf, size_t max)` — returns bytes read
- `bool isOpen() const`, `void close()`

## main.cpp Flow (Linux)

1. Parse `config.toml` with `toml++`
2. Install signal handler SIGINT/SIGTERM → `app.stop()`
3. Open `LinuxLogger`
4. Create `LinuxUART` × 2, `EzoRTDSensorDevice`, `EzoDOSensorDevice`
5. Create `LinuxGPIO` × 2, `BuzzerDevice`, `HeaterDevice`
6. Create `SensorAdapter`, `ActuatorAdapter`
7. Create `LinuxClock` (always)
8. Display selection: `display.type == "st7789"` → `ST7789DisplayDevice`; else → `TerminalDisplayDevice`
9. Create `DisplayRenderer`, `DisplayAdapter`
10. Create `LinuxEventQueue`, `LinuxAtomicBool`
11. Create `SmartAquaria(sensor, actuator, thresholds, queue, clock, shutdown)`
12. `app.setLogger(std::move(logger))`, `app.setDisplay(std::move(displayAdapter))`, `app.setFeedInterval`
13. Create `TimerService` × 1 (sampleTimer); call `run([&app]{ app.push(SampleTimerElapsed); })`
14. `app.run()` — blocks until `stop()`
15. Stop timers, turn off buzzer/heater

## aquarunner Bootstrapper Flow

1. Parse CLI: `<config_path> [debug|release]`
2. **Build**: `subprocess.run([sys.executable, "build.py", "linux", build_type], cwd=SMART_AQUARIA)`
3. Load `config.toml`; read `mock.sensor`, `mock.actuator`
4. If `mock.actuator`: `GpioSim.setup()` — load `gpio-sim`, patch chip path in config
5. If `mock.sensor`: `VirtualUart.start()` × 2 — create virtual UART pairs with `socat`
6. Start `mock_peripheral/linux/sensor_peripheral.py` and/or `actuator_peripheral.py` if mocked
7. Launch `smart_aquaria/build/linux-{build_type}/configurator/linux/smart_aquaria_linux <config>`
8. Block until a process dies or SIGINT/SIGTERM
9. `finally`: stop processes, VirtualUarts, GpioSim in reverse order

## Important Notes

- Root `CMakeLists.txt` is in `smart_aquaria/` directory
- `core/osal/` and `core/hal/` are CMake INTERFACE targets (header-only); platform implementations link against them
- Linux platform cmake targets: `SmartAquaria_OSAL_Linux`, `SmartAquaria_HAL_Linux`
- `ST7789DisplayDevice::flush()` is a no-op — pixel data is sent immediately during `drawChar()`
- `EzoRTDSensorDevice`/`EzoDOSensorDevice` strip trailing `\r\n` from UART responses before parsing
- Both EZO devices use `R\n` command (Atlas Scientific protocol); responses are ASCII floats
- `LinuxLogger` opens in truncate mode (fresh log each run)
- `toml++` fetched via FetchContent (v3.4.0, header-only); `tomllib` is Python 3.11+ stdlib
- `aquarunner/__main__.py` uses `Path(__file__).parent.parent` for directory resolution — safe to run from any directory
- Log files written to `log/` directory (aquarunner cwd)
- `ActuatorAdapter::feed()` is currently empty — no feeding hardware connected

## Adding New Sensors

1. Add method to `core/domain/ISensor.h`
2. Create device interface + implementation under `core/device/sensor/`
3. Add reference member to `SensorAdapter`; implement delegation
4. Add field to `core/domain/Readings.h` if it should appear on display
5. Update `DisplayRenderer::render()` to format and display the new value

## TODO

- **Feeding actuator**: `ActuatorAdapter::feed()` is currently a no-op. Needs hardware (servo or pump) and corresponding HAL wiring.
- **Security considerations**: Document in System-Level Considerations section (even if minimal for aquarium use case). Discuss: unauthorized access to system, sensor tampering, actuator hijacking, network security if remote monitoring enabled.
