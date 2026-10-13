# Flipper Zero DIY — Renode-First Embedded Architecture

A open-source, modular multi-tool device inspired by Flipper Zero, architected from Day 1 for **Renode simulation as a first-class execution target**.

📖 **Detailed Documentation**: See [docs/RENODE_INTEGRATION.md](docs/RENODE_INTEGRATION.md) for full peripheral hierarchy maps and Renode Monitor CLI commands.

---

## 🎯 Architecture Philosophy: One Firmware → Two Targets

```
┌─────────────────────────────────────────────────────────────┐
│                     Application Layer                       │
│    (GUI / Sub-GHz App / NFC App / RFID App / Tools)         │
└──────────────────────────────┬──────────────────────────────┘
                               │
┌──────────────────────────────▼──────────────────────────────┐
│                       Services Layer                        │
│   (GUI Engine / Event Queue / Storage Manager / Radio Svc)   │
└──────────────────────────────┬──────────────────────────────┘
                               │
┌──────────────────────────────▼──────────────────────────────┐
│                   Device Driver Layer                       │
│      (ST7789 Display / CC1101 RF / PN532 NFC / Buttons)      │
└──────────────────────────────┬──────────────────────────────┘
                               │
┌──────────────────────────────▼──────────────────────────────┐
│             Hardware Abstraction Layer (HAL)                │
│    (hal_gpio, hal_spi, hal_i2c, hal_uart, hal_timer)        │
└───────────────┬─────────────────────────────┬───────────────┘
                │                             │
    ┌───────────▼───────────┐     ┌───────────▼───────────┐
    │     Real Hardware     │     │   Renode Simulation   │
    │  (STM32F4 / physical) │     │  (.repl / .resc / CI) │
    └───────────────────────┘     └───────────────────────┘
```

The firmware is strictly decoupled from physical silicon. The exact same ELF binary built for the MCU executes inside Renode for hardware-in-the-loop (HIL) testing, GUI verification, and automated Robot Framework CI.

---

## 🔬 Platform & MCU Selection Matrix

| Evaluated Platform | Renode CPU Support | Peripheral Maturity in Renode | Radio Simulation | Display & I/O | Decision |
|---|---|---|---|---|---|
| **STM32F407VG** (ARM Cortex-M4F) | **Native** (`STM32F4`) | **Full** (NVIC, RCC, GPIO, SPI, I2C, USART, DMA, Timers) | Supported via CC1101 C#/Python models over SPI | Native SPI LCD (ST7789/ILI9341) & GPIO Buttons | **Selected Primary Target** |
| **nRF52840** (ARM Cortex-M4F) | **Native** (`nRF52840`) | **Full** (NVMC, PPI, TWIM, SPIM, UARTE, RTC) | Native 2.4GHz / IEEE 802.15.4 / BLE hub | SPI LCD & GPIO Buttons | **Supported Alternative** |
| **STM32WB55** | Partial | M0+ IPC co-processor lacks native models | Requires custom IPCC model | SPI LCD & Buttons | Deferred (High friction) |
| **ESP32-S3** | Experimental | Limited MMIO coverage | No Wi-Fi radio stack in standard Renode | UART / Framebuffer | Deferred |

---

## 📋 Phase 0 Simulation Acceptance Criteria

1. **Exact MCU Target**: STM32F407VGT6 (ARM Cortex-M4F @ 168 MHz, 1MB Flash, 192KB RAM).
2. **Firmware Target**: Single C/C++ codebase compiled with `arm-none-eabi-gcc` targeting CMSIS + HAL.
3. **Simulated Peripherals**:
   - ARM Cortex-M4F core with NVIC, SysTick, and EXTI.
   - RCC, GPIOA-GPIOH, DMA1/DMA2, TIM2/TIM3.
   - USART1 (CLI/Debug Console), USART2.
   - SPI1 (ST7789 Display 240x240 LCD), SPI2 (CC1101 Sub-1GHz RF transceiver).
   - I2C1 (PN532 NFC controller / Sensors).
   - 5-way directional pad + Back button (`sysbus.gpioPortE.buttonOk`, etc.).
4. **Custom Renode Models**:
   - `CC1101` SPI model for Sub-1GHz RF register & FIFO handling.
   - `PN532` I2C model for NFC frame emulation.
5. **Mocked Peripherals**:
   - Battery power management / Fuel gauge via mock I2C register map.
6. **Simulation Boundaries**:
   - RF physical analog wave propagation is abstracted to protocol frame/packet transfers.
7. **Firmware Communication**:
   - Direct memory-mapped registers (`MMIO`) and interrupts (`NVIC`), identical to hardware.
8. **Automated Testing**:
   - Robot Framework runs Renode headlessly, injects GPIO button events, inspects display framebuffers, and verifies UART console responses.
9. **Physical Prototype Transition**:
   - Zero application code changes required; only board pin configuration header matches physical PCB layout.

---

## 🚀 Quick Start: Running Renode Simulation

### Prerequisites
1. [Renode](https://renode.io/) (v1.13+) installed and on your PATH.
2. `arm-none-eabi-gcc` toolchain or Python 3.

### 1. Build Firmware Binary
```bash
python firmware/generate_elf.py
```

### 2. Launch Renode Simulation
```bash
renode simulation/renode/flipper_zero_diy.resc
```

---

## 📁 Repository Structure

```
├── docs/
│   └── RENODE_INTEGRATION.md   # Complete Renode Object Tree & CLI Reference
├── firmware/
│   ├── cmake/                  # CMake toolchain files
│   ├── include/
│   │   ├── hal/                # Hardware Abstraction Layer interfaces
│   │   ├── drivers/            # Device drivers (ST7789, CC1101, PN532)
│   │   └── services/           # GUI, Input, Radio Services
│   ├── src/
│   │   ├── hal/                # STM32F4 HAL implementation
│   │   ├── drivers/            # Display, Radio, NFC drivers
│   │   ├── services/           # Event queue, GUI engine
│   │   └── main.c              # Firmware entry point
│   ├── generate_elf.py         # ELF binary generator
│   └── CMakeLists.txt
├── simulation/
│   ├── renode/
│   │   ├── flipper_zero_diy.repl # Renode Platform Definition
│   │   └── flipper_zero_diy.resc # Renode Execution Script
│   └── tests/
│       └── test_flipper.robot   # Robot Framework automated test suite
└── README.md
```
