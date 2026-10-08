#include "gui.h"
#include "hal_display.h"
#include "hal_uart.h"

void gui_init(void) {
    hal_display_init();
    hal_display_clear(0x0000); // Black
}

void gui_render_home_screen(void) {
    hal_uart_write_string(HAL_UART_1, "[GUI] Rendering Home Screen\n");
}

void gui_process_input(uint8_t key_code) {
    (void)key_code;
    hal_uart_write_string(HAL_UART_1, "[GUI] Key input processed\n");
}
