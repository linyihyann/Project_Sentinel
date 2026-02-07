# Project_Sentinel

Firmware for **Raspberry Pi Pico 2 W (RP2350)** using a strict **Clean Architecture**: application logic is fully decoupled from hardware.

---

## Why We Separate HAL and App (Interview Talking Points)

### 1. **Testability**

- **App** (`src/app`) has **zero** dependency on the Pico SDK or any hardware. It only depends on two function types: “set LED” and “sleep.”
- We can unit-test `sentinel_app_run()` on a PC by passing mock callbacks (e.g., record calls or fake delays) with no board, no toolchain, and no SDK. No need for hardware-in-the-loop for core logic.
- **HAL** can be tested or stubbed separately (e.g., replace `hal_led.c` with a simulator or another board’s driver).

### 2. **Portability**

- The same application logic can be reused on another MCU or platform. We only reimplement the HAL (e.g., different `hal_led.c` for another board or OS).
- No “#ifdef PICO” or board-specific code inside the app. Dependencies point inward: App → abstractions; HAL and main → concrete implementations.

### 3. **Dependency Rule (Clean Architecture)**

- **App** does not include `pico/*` or any driver/SDK headers. It only knows about callbacks (`set_led`, `sleep_ms`). So the core of the system does not depend on framework or hardware details.
- **HAL** is the only place that talks to `pico/cyw43_arch` and `pico/stdlib`. All hardware details stay in one layer.
- **main.c** is the composition root: it wires HAL implementations into the app (dependency injection). That keeps “how things are connected” in one file and makes the architecture explicit.

### 4. **Maintainability and Change Isolation**

- Changing the board (e.g., different LED pin or different WiFi chip) only touches HAL and possibly main; the app code stays unchanged.
- Adding features (e.g., another output or sensor) is done by extending the injected interface and implementing it in HAL, without polluting the core logic.

### 5. **Clear Boundaries**

- **App** = “what the product does” (e.g., blink policy, timing, sequences).
- **HAL** = “how we talk to this board” (GPIO, CYW43, sleep).
- **main** = “how we plug them together.”

This separation makes it easier to onboard developers, do code reviews, and enforce that no SDK or hardware includes leak into the app layer.

---

## Layout

```
Project_Sentinel/
├── CMakeLists.txt          # pico2_w, pico_cyw43_arch_none, USB stdio, PICOTOOL_FETCH_FROM_GIT_PATH
├── pico_sdk_import.cmake
├── README.md
└── src/
    ├── main.c              # Injection: stdio_init_all, hal_led_init, sentinel_app_run(hal_led_set, hal_sleep_ms)
    ├── app/
    │   ├── sentinel_app.c  # Pure logic — NO pico/* includes
    │   └── sentinel_app.h  # Callback types + sentinel_app_run()
    └── hal/
        ├── hal_led.c       # CYW43 / pico implementation for LED + sleep
        └── hal_led.h       # hal_led_init(), hal_led_set(), hal_sleep_ms()
```

---

## Build

- **Target board:** `pico2_w` (RP2350).
- **PICOTOOL_FETCH_FROM_GIT_PATH** is set **ON** in `CMakeLists.txt` to avoid RP2350 family ID / picotool issues.
- **CYW43:** linked with `pico_cyw43_arch_none` (LED only, no WiFi stack).
- **Stdio:** USB stdio enabled (`pico_enable_stdio_usb(Project_Sentinel 1)`).

From the **project directory** (the one that contains `CMakeLists.txt` and `src/`):

```bash
export PICO_SDK_PATH=/path/to/pico-sdk   # use your real path, e.g. $HOME/pico/pico-sdk
mkdir build && cd build
cmake -G Ninja ..
ninja
```

If CMake still says "Directory '/path/to/pico-sdk' not found", the build dir has a cached path. Clear cache and reconfigure:

```bash
rm -rf CMakeCache.txt CMakeFiles
export PICO_SDK_PATH=$HOME/pico/pico-sdk   # or your actual path
cmake -G Ninja .. && ninja
```

If your repo has a nested layout (e.g. workspace `Project_Sentinel` with project in `Project_Sentinel/Project_Sentinel/`), either run the above from the inner folder, or from the outer `build` run:

```bash
cmake ../Project_Sentinel
```

Then flash the generated UF2 to the Pico 2 W. The onboard LED should blink at 1 Hz (500 ms on, 500 ms off).
