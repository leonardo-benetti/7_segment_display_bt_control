/* goals
send signal from cellphone to esp32 to lightup 7-segment display
    - use volume buttons (AVRCP) to increase and decrease number displayed
        - learn how display works
        - manage to recieve volume change events from cellphone
*/
#include <stdio.h>
#include "7-segment-display.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define D7S_PIN_A   16
#define D7S_PIN_B   17
#define D7S_PIN_C   18
#define D7S_PIN_D   19
#define D7S_PIN_E   21
#define D7S_PIN_F   22
#define D7S_PIN_G   23
#define D7S_PIN_DP  3


static d7s_ios_num pin_numbers = {
    .sega_io_num = D7S_PIN_A,
    .segb_io_num = D7S_PIN_B,
    .segc_io_num = D7S_PIN_C,
    .segd_io_num = D7S_PIN_D,
    .sege_io_num = D7S_PIN_E,
    .segf_io_num = D7S_PIN_F,
    .segg_io_num = D7S_PIN_G,
    .segdp_io_num = D7S_PIN_DP,
};


void init_function(void)
{
    init_display_pins(pin_numbers);
}

void app_main(void)
{
    init_function();

    uint8_t num = 0;

    while (true)
    {
        set_display_number(pin_numbers, num++);

        if (num > 19)
            num = 0;
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
}