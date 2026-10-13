#ifndef GUI_H
#define GUI_H

#include <stdint.h>

void gui_init(void);
void gui_render_home_screen(void);
void gui_process_input(uint8_t key_code);

#endif // GUI_H
