#include <stdio.h>
#include "7-segment-display.h"

void init_display_pins(d7s_ios_num io_pins)
{
    gpio_reset_pin(io_pins.sega_io_num);
    gpio_set_direction(io_pins.sega_io_num, GPIO_MODE_OUTPUT);
    gpio_set_level(io_pins.sega_io_num, LIGHT_DOWN);

    gpio_reset_pin(io_pins.segb_io_num);
    gpio_set_direction(io_pins.segb_io_num, GPIO_MODE_OUTPUT);
    gpio_set_level(io_pins.segb_io_num, LIGHT_DOWN);

    gpio_reset_pin(io_pins.segc_io_num);
    gpio_set_direction(io_pins.segc_io_num, GPIO_MODE_OUTPUT);
    gpio_set_level(io_pins.segc_io_num, LIGHT_DOWN);

    gpio_reset_pin(io_pins.segd_io_num);
    gpio_set_direction(io_pins.segd_io_num, GPIO_MODE_OUTPUT);
    gpio_set_level(io_pins.segd_io_num, LIGHT_DOWN);

    gpio_reset_pin(io_pins.sege_io_num);
    gpio_set_direction(io_pins.sege_io_num, GPIO_MODE_OUTPUT);
    gpio_set_level(io_pins.sege_io_num, LIGHT_DOWN);

    gpio_reset_pin(io_pins.segf_io_num);
    gpio_set_direction(io_pins.segf_io_num, GPIO_MODE_OUTPUT);
    gpio_set_level(io_pins.segf_io_num, LIGHT_DOWN);

    gpio_reset_pin(io_pins.segg_io_num);
    gpio_set_direction(io_pins.segg_io_num, GPIO_MODE_OUTPUT);
    gpio_set_level(io_pins.segg_io_num, LIGHT_DOWN);

    gpio_reset_pin(io_pins.segdp_io_num);
    gpio_set_direction(io_pins.segdp_io_num, GPIO_MODE_OUTPUT);
    gpio_set_level(io_pins.segdp_io_num, LIGHT_DOWN);

}


void set_display_number(d7s_ios_num io_pins, uint8_t target_num)
{
    if (target_num > 15)
    {
        gpio_set_level(io_pins.sega_io_num, LIGHT_DOWN);
        gpio_set_level(io_pins.segb_io_num, LIGHT_DOWN);
        gpio_set_level(io_pins.segc_io_num, LIGHT_DOWN);
        gpio_set_level(io_pins.segd_io_num, LIGHT_DOWN);
        gpio_set_level(io_pins.sege_io_num, LIGHT_DOWN);
        gpio_set_level(io_pins.segf_io_num, LIGHT_DOWN);
        gpio_set_level(io_pins.segg_io_num, LIGHT_DOWN);
        gpio_set_level(io_pins.segdp_io_num, LIGHT_UP);

        return;
    }

    bool A, B, C, D, AC, BC, CC, DC;
    A = (target_num >> 3) & 0x1;
    AC = !A;
    B = (target_num >> 2) & 0x1;;
    BC = !B;
    C = (target_num >> 1) & 0x1;;
    CC = !C;
    D = target_num & 0x1;
    DC = !D;

    // condition to light a
    if ( (A & BC & CC) | (AC & B & D) | (A & DC) | (AC & C) | (B & C) | (BC & DC) )
        gpio_set_level(io_pins.sega_io_num, LIGHT_UP);
    else
        gpio_set_level(io_pins.sega_io_num, LIGHT_DOWN);
    
    // condition to light b
    if ( (AC & CC & DC) | (AC & C & D) | (A & CC & D) | (BC & CC) | (BC * DC) )
        gpio_set_level(io_pins.segb_io_num, LIGHT_UP);
    else
        gpio_set_level(io_pins.segb_io_num, LIGHT_DOWN);
    
    // condition to light c
    if ( (AC & CC) | (AC & D) | (CC & D) | (AC & B) | (A & BC) )
        gpio_set_level(io_pins.segc_io_num, LIGHT_UP);
    else
        gpio_set_level(io_pins.segc_io_num, LIGHT_DOWN);
    
    // condition to light d
    if ( (AC & BC & DC) | (BC & C & D) | (B & CC & D) | (B & C & DC) | (A & CC) )
        gpio_set_level(io_pins.segd_io_num, LIGHT_UP);
    else
        gpio_set_level(io_pins.segd_io_num, LIGHT_DOWN);

    // condition to light e
    if ( (BC & DC) | (C & DC) | (A & C) | (A & B) )
        gpio_set_level(io_pins.sege_io_num, LIGHT_UP);
    else
        gpio_set_level(io_pins.sege_io_num, LIGHT_DOWN);
    
    // condition to light f
    if ( (AC & B & CC) | (CC & DC) | (B & DC) | (A & BC) | (A & C) )
        gpio_set_level(io_pins.segf_io_num, LIGHT_UP);
    else
        gpio_set_level(io_pins.segf_io_num, LIGHT_DOWN);

    // condition to light g
    if ( (AC & B & CC) | (BC & C) | (C & DC) | (A & BC) | (A & D) )
        gpio_set_level(io_pins.segg_io_num, LIGHT_UP);
    else
        gpio_set_level(io_pins.segg_io_num, LIGHT_DOWN);
}
