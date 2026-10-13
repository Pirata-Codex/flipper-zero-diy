cmake_minimum_required(VERSION 3.20)
project(flipper_zero_diy C)

set(CMAKE_C_STANDARD 11)

include_directories(
    include
    include/hal
    include/drivers
    include/services
    include/protocols
)

# Core logic & protocol sources
set(CORE_SOURCES
    src/services/event_loop.c
    src/services/input_service.c
    src/protocols/subghz_protocol.c
    src/protocols/ir_protocol.c
    src/protocols/rfid_protocol.c
)

# Unit test executable for host / test harness
add_executable(unit_tests
    tests/test_main.c
    ${CORE_SOURCES}
)

# Target ELF for Renode / STM32F4
add_executable(flipper_zero_diy.elf
    src/main.c
    src/hal/hal_stm32f4.c
    src/drivers/st7789.c
    src/services/gui.c
    ${CORE_SOURCES}
)

target_compile_definitions(flipper_zero_diy.elf PRIVATE
    STM32F407xx
)
