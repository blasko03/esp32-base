# ESP-IDF Hardware Abstraction Base Project

A base project for ESP-IDF applications (ESP32-C5) that provides clean, reusable
abstractions over common hardware peripherals and communication stacks. Use it as
a starting point for your own firmware: instead of fighting the low-level ESP-IDF
APIs, you build on top of the simple typed interfaces exposed here.

Currently implements abstractions over:

- **Bluetooth** (NimBLE) — GAP + GATT server *(work in progress)*
- **WiFi** — station mode *(work in progress)*
- **GPIO** — digital output
- **ADC** — one-shot reads with calibration

## Project layout

```
main/
├── main.c                  # Example app demonstrating the abstractions
├── io/                     # Hardware peripheral abstractions
│   ├── gpio.h / gpio.c     # Digital output (GPIO) wrapper
│   └── adc.h  / adc.c      # One-shot ADC with calibration (raw + millivolts)
├── trans/                  # Communication stack wrappers
│   ├── wifi/
│   │   └── wifi.h/.c       # WiFi station, connects to an AP
│   └── bluetooth/
│       ├── gap.h/.c        # BLE advertising + GAP event handling
│       ├── gatt_svc.h/.c   # GATT server (heart-rate style service)
│       └── common.h        # Shared NimBLE includes/defines
└── devices/                # Driver-level modules built on the abstractions
    └── ACS712.h/.c         # ACS712 hall-effect current sensor
```

### Abstraction layers

The idea is a small dependency pyramid, so application code stays portable:

```
main.c  →  devices/ (sensors)  →  io/ (gpio, adc)  →  ESP-IDF drivers
       →  trans/ (wifi, bluetooth)                 →  ESP-IDF stacks
```

- **`io/`** — Thin, type-safe wrappers. `gpio_config_pin_t` / `adc_config_t` are
  opaque-ish descriptors you init once and pass to every operation.
- **`trans/`** — Bring-up + event loop boilerplate for WiFi and BLE, so you don't
  have to wire up ESP-IDF event handlers or NimBLE callbacks yourself.
- **`devices/`** — Concrete sensor/driver modules that reuse the abstractions.
  Add new sensors here; they only need to include `io/` headers.

## Requirements

- ESP-IDF **v6.0.2** (with Espressif Installation Manager / `idf.py`)
- An ESP32-C5 target (or the board you reconfigure the target for)
- CMake >= 3.22
- Optionally: [Podman/Docker](https://podman.io/) with `compose.yml` for a
  containerized build environment

## Building

### Option A: with the provided Docker environment

```sh
docker compose up -d            # starts the idf container
docker compose exec idf bash     # drop into the shell with IDF activated
```

Inside the container, build and flash as usual:

```sh
idf.py set-target esp32c5        # once, if not already set
idf.py build
idf.py flash monitor
```

### Option B: local ESP-IDF

```sh
# activate ESP-IDF (path depends on your install)
source /opt/espressif/tools/activate_idf_v6.0.2.sh  # or the install script

idf.py set-target esp32c5
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

### Menuconfig

WiFi credentials and connection behavior are configurable via `idf.py menuconfig`
(defined in `main/Kconfig.projbuild`):

- `ESP_WIFI_SSID` / `ESP_WIFI_PASSWORD` — network to join
- `ESP_MAXIMUM_RETRY` — reconnect attempts before giving up
- `ESP_WIFI_AUTH_*` / WPA3 SAE modes — auth threshold and SAE settings

Bluetooth uses NimBLE, enabled through `dkconfig.defaults`
(`CONFIG_BT_ENABLED=y`, `CONFIG_BT_NIMBLE_ENABLED=y`).

## Using it in your project

Build on top of the existing modules in `app_main()` (see `main/main.c`).

### GPIO output

```c
#include "io/gpio.h"

gpio_config_pin_t led = gpio_out_configure_pin(GPIO_NUM_24); // returns .init == true on success
gpio_out_set_level(&led, 1); // drive pin high
gpio_out_set_level(&led, 0); // drive pin low
```

### ADC (raw / millivolts)

```c
#include "io/adc.h"

adc_config_t adc = adc_init(ADC_UNIT_1, ADC_CHANNEL_0, ADC_ATTEN_DB_12);

int mv = 0;
if (adc_read_millivolts(&adc, &mv) == ESP_OK) {
    // use calibrated millivolts (falls back to raw counts if no eFuse calibration)
}
adc_deinit(&adc);
```

### WiFi (station)

```c
#include "trans/wifi/wifi.h"

init_wifi(); // initializes NVS, connects to the SSID from menuconfig, blocks until connected/failed
```

### Bluetooth (GATT server + advertising)

```c
#include "trans/bluetooth/gap.h"
#include "trans/bluetooth/gatt_svc.h"

// During NimBLE host sync: gap_init(); gatt_svc_init(); adv_init();
// Push data: send_heart_rate_indication();
```

### Devices

```c
#include "devices/ACS712.h"

adc_config_t acs = acs712_init(ADC_UNIT_1, ADC_CHANNEL_0);
int ma = acs712_milliamperes(&acs); // current in milliamperes, or ESP_FAIL on read error
```