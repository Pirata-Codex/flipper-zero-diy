*** Settings ***
Documentation       Automated Integration Tests for Flipper Zero DIY in Renode
Suite Setup         Setup
Suite Teardown      Teardown
Test Setup          Reset Emulation
Resource            ${RENODEKEYWORDS}

*** Variables ***
${RESC}             ${CURDIR}/../renode/flipper_zero_diy.resc
${UART}             sysbus.usart1

*** Test Cases ***
Boot Firmware And Check Console
    [Documentation]    Verifies that the firmware boots and outputs the startup banner on USART1.
    Start Emulation
    Wait For Line On Uart    Flipper Zero DIY Firmware v1.0.0 Booting...    timeout=5    terminal=${UART}
    Wait For Line On Uart    [HAL] Display ST7789 initialized    timeout=5    terminal=${UART}
    Wait For Line On Uart    [SYSTEM] Ready    timeout=5    terminal=${UART}

Test Button Press Event Handling
    [Documentation]    Simulates pressing the OK button and checks console output.
    Start Emulation
    Wait For Line On Uart    [SYSTEM] Ready    timeout=5    terminal=${UART}

    # Press OK Button (GPIO E4)
    Execute Command          sysbus.gpioPortE.buttonOk Press
    Wait For Line On Uart    [INPUT] Button OK Pressed    timeout=2    terminal=${UART}

    # Release OK Button
    Execute Command          sysbus.gpioPortE.buttonOk Release
    Wait For Line On Uart    [INPUT] Button OK Released    timeout=2    terminal=${UART}
