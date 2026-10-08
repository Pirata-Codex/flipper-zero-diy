# Renode Simulation Architecture & Integration Guide

This document details the **Renode-First Simulation Architecture** for the `flipper-zero-diy` project, including peripheral object hierarchies, REPL syntax, CLI monitor commands, and automated test execution.

---

## 📐 Architecture Overview

The project is built on a **One Firmware → Two Execution Targets** principle:

```
┌─────────────────────────────────────────────────────────────┐
│                     Application Layer                       │
│        (GUI Engine, Sub-GHz App, NFC App, Tools)            │
└──────────────────────────────┬──────────────────────────────┘
                               │
┌──────────────────────────────▼──────────────────────────────┐
│                  Hardware Abstraction Layer                 │
│    (hal_gpio, hal_spi, hal_i2c, hal_uart, hal_timer)        │
└───────────────┬─────────────────────────────┬───────────────┘
                │                             │
    ┌───────────▼───────────┐     ┌───────────▼───────────┐
    │     Real Hardware     │     │   Renode Simulation   │
    │  (STM32F407VG MCU)    │     │  (.repl / .resc)      │
    └───────────────────────┘     └───────────────────────┘
```

---

## 🌳 Renode System Bus & Peripheral Hierarchy

In Renode, when peripherals are registered on GPIO ports or system buses, they form a structured object hierarchy under `sysbus`.

### Platform Mapping (`simulation/renode/flipper_zero_diy.repl`)

| Peripheral | REPL Definition | Full Object Path in Renode Monitor |
|---|---|---|
| **CPU** | `platforms/cpus/stm32f4.repl` | `sysbus.cpu` |
| **Console UART** | `usart1: STM32_UART @ sysbus 0x40011000` | `sysbus.usart1` |
| **OK Button** | `buttonOk: Miscellaneous.Button @ gpioPortE 4` | `sysbus.gpioPortE.buttonOk` |
| **Up Button** | `buttonUp: Miscellaneous.Button @ gpioPortE 0` | `sysbus.gpioPortE.buttonUp` |
| **Down Button** | `buttonDown: Miscellaneous.Button @ gpioPortE 1` | `sysbus.gpioPortE.buttonDown` |
| **Left Button** | `buttonLeft: Miscellaneous.Button @ gpioPortE 2` | `sysbus.gpioPortE.buttonLeft` |
| **Right Button** | `buttonRight: Miscellaneous.Button @ gpioPortE 3` | `sysbus.gpioPortE.buttonRight` |
| **Back Button** | `buttonBack: Miscellaneous.Button @ gpioPortE 5` | `sysbus.gpioPortE.buttonBack` |
| **Green LED** | `ledGreen: Miscellaneous.LED @ gpioPortA 8` | `sysbus.gpioPortA.ledGreen` |
| **Red LED** | `ledRed: Miscellaneous.LED @ gpioPortA 9` | `sysbus.gpioPortA.ledRed` |
| **Blue LED** | `ledBlue: Miscellaneous.LED @ gpioPortA 10` | `sysbus.gpioPortA.ledBlue` |

> ⚠️ **Key Finding**: Peripherals declared with `@ gpioPortE <pin>` reside inside the `gpioPortE` namespace under `sysbus`. Accessing them as `buttonOk` at the root prompt returns `No such command or device`. You must use the full path: `sysbus.gpioPortE.buttonOk`.

---

## 🖥️ Renode Monitor CLI Reference

### 1. Execution Control
```mon
# Load script
include @simulation/renode/flipper_zero_diy.resc

# Start / Pause / Reset emulation
start   (or 's')
pause   (or 'p')
reset
```

### 2. Button Input Simulation
```mon
# Press and hold OK button
sysbus.gpioPortE.buttonOk Press

# Release OK button
sysbus.gpioPortE.buttonOk Release

# Perform a click (press and release)
sysbus.gpioPortE.buttonOk PressAndRelease

# Toggle state
sysbus.gpioPortE.buttonOk Toggle
```

### 3. LED & GPIO State Inspection
```mon
# Get Red LED state
sysbus.gpioPortA.ledRed State

# Read GPIO Port E registers
sysbus.gpioPortE ReadDoubleWord 0x10
```

---

## 🛠️ Building and Running

### 1. Build Firmware Binary
Generate the target ARM Cortex-M4 ELF executable:
```bash
python firmware/generate_elf.py
```
*Output binary*: `firmware/build/flipper_zero_diy.elf`

### 2. Launch Simulation
Run Renode with the platform script:
```bash
renode simulation/renode/flipper_zero_diy.resc
```

### 3. Verification over Scriptable Socket
Renode exposes a monitor CLI socket on port `1234`. Python scripts can automate stimulus injection and telemetry assertion:
```python
import telnetlib

tn = telnetlib.Telnet("localhost", 1234)
tn.write(b"s\n")
tn.write(b"sysbus.gpioPortE.buttonOk PressAndRelease\n")
```
