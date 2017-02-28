/*
    Copyright 2016 fishpepper <AT> gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    author: fishpepper <AT> gmail.com
*/

#ifndef GUI_H_
#define GUI_H_

#include <stdint.h>
#include "adc.h"

#define GUI_PREV_CLICK_X 10
#define GUI_NEXT_CLICK_X (LCD_WIDTH - GUI_PREV_CLICK_X)
#define GUI_PAGE_MAIN     0
#define GUI_PAGE_STICKS   1
#define GUI_PAGE_SETTINGS 2
#define GUI_MAX_PAGE GUI_PAGE_SETTINGS
#define GUI_STATUSBAR_FONT font_tomthumb3x5


#define GUI_LOOP_DELAY_MS 100
#define GUI_SHUTDOWN_PRESS_S 2.0
#define GUI_SHUTDOWN_PRESS_COUNT_FROM_MS(_ms) ((_ms)/GUI_LOOP_DELAY_MS)
#define GUI_SHUTDOWN_PRESS_COUNT (GUI_SHUTDOWN_PRESS_COUNT_FROM_MS(1000*GUI_SHUTDOWN_PRESS_S))

// touch function pointer
typedef void (*f_ptr_t)(void);
typedef void (*f_ptr_32_32_t)(uint32_t x, uint32_t y);

// touch callback
typedef struct {
    // defines the touch sensitive rect
    uint8_t xs;
    uint8_t xe;
    uint8_t ys;
    uint8_t ye;
    // and the callback to run
    f_ptr_t callback;
} touch_callback_entry_t;

typedef struct {
    int8_t *data;
    int8_t min;
    int8_t max;
    int8_t step;
} option_int8_t;

#define GUI_TOUCH_CALLBACK_COUNT 10
static void gui_touch_callback_register(uint8_t xs, uint8_t xe, uint8_t ys, uint8_t ye, f_ptr_t cb);
static void gui_touch_callback_clear(void);


#define GUI_PAGE_CONFIG_FLAG        0x80
#define GUI_PAGE_CONFIG_OPTION_FLAG 0x40
#define GUI_PAGE_SETUP_FLAG         0x20
#define GUI_PAGE_NOFLAGS (~(GUI_PAGE_CONFIG_FLAG|GUI_PAGE_CONFIG_OPTION_FLAG|GUI_PAGE_SETUP_FLAG))


#define GUI_PAGE_SETUP_MAIN       (GUI_PAGE_SETUP_FLAG | 0)
#define GUI_PAGE_SETUP_CLONETX    (GUI_PAGE_SETUP_FLAG | 1)
#define GUI_PAGE_SETUP_BIND       (GUI_PAGE_SETUP_FLAG | 2)
#define GUI_PAGE_SETUP_BOOTLOADER (GUI_PAGE_SETUP_FLAG | 3)

#define GUI_PAGE_CONFIG_MAIN            (GUI_PAGE_CONFIG_FLAG | 0)
#define GUI_PAGE_CONFIG_STICK_CAL       (GUI_PAGE_CONFIG_FLAG | 1)
#define GUI_PAGE_CONFIG_MODEL_SETTINGS  (GUI_PAGE_CONFIG_FLAG | 2)
#define GUI_PAGE_CONFIG_4IN1_SETTINGS   (GUI_PAGE_CONFIG_FLAG | 3)


#define GUI_SUBPAGE_SETTING_MODEL_NAME  0
#define GUI_SUBPAGE_SETTING_MODEL_SCALE 1
#define GUI_SUBPAGE_SETTING_MODEL_TIMER 2

#define GUI_SUBPAGE_SETTING_4IN1_PROTOCOL 0
#define GUI_SUBPAGE_SETTING_4IN1_SUB_PROTOCOL 1
#define GUI_SUBPAGE_SETTING_4IN1_RX_NUM 2
#define GUI_SUBPAGE_SETTING_4IN1_OPTION 3
#define GUI_SUBPAGE_SETTING_4IN1_AUTOBIND 4
#define GUI_SUBPAGE_SETTING_4IN1_LOW_POWER 5

void gui_init(void);
void gui_loop(void);

uint32_t gui_running(void);

static void gui_process_touch(void);
static void gui_render_main_screen(void);
static void gui_process_logic(void);


static void gui_config_render(void);
static void gui_config_stick_calibration_store_adc_values(void);
static void gui_config_stick_calibration_render(void);

static void gui_setup_render(void);
static void gui_setup_main_render(void);


static void gui_render(void);
static void gui_render_sliders(void);
static void gui_render_battery(void);
static void gui_render_statusbar(void);
static void gui_render_bottombar(void);
static void gui_render_settings(void);

static void gui_touch_callback_execute(uint8_t i);
static void gui_add_button(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t *str, f_ptr_t cb);
static void gui_add_button_smallfont(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
                                     uint8_t *str, f_ptr_t cb);
static void gui_cb_model_timer_reload(void);
static void gui_cb_model_prev(void);
static void gui_cb_model_next(void);
static void gui_cb_setting_model_stickscale(void);
static void gui_cb_setting_model_name(void);
static void gui_cb_setting_model_timer(void);
static void gui_cb_setting_option_leave(void);
static void gui_cb_previous_page(void);
static void gui_cb_next_page(void);
static void gui_cb_config_back(void);
static void gui_cb_config_save(void);
static void gui_cb_config_stick_cal(void);
static void gui_cb_config_model(void);
static void gui_cb_config_exit(void);

static void gui_cb_setup_clonetx(void);
static void gui_cb_setup_bootloader(void);
static void gui_cb_setup_exit(void);

static void gui_config_main_render(void);
static void gui_config_model_render(void);
static void gui_config_4in1_render(void);

static void gui_setup_clonetx_render(void);
static void gui_setup_bindmode_render(void);
static void gui_setup_bootloader_render(void);

#endif  // GUI_H_
