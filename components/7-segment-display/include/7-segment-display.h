#include <stdint.h>
#include <driver/gpio.h>

#define LIGHT_UP 0
#define LIGHT_DOWN 1

/* pin diagram:

    G F + A B
    . . . . .

    . . . . .
    E D + C DP
*/

typedef struct
{
    uint8_t sega_io_num;
    uint8_t segb_io_num;
    uint8_t segc_io_num;
    uint8_t segd_io_num;
    uint8_t sege_io_num;
    uint8_t segf_io_num;
    uint8_t segg_io_num;
    uint8_t segdp_io_num;
} d7s_ios_num;


void init_display_pins(d7s_ios_num io_pins);

void set_display_number(d7s_ios_num io_pins, uint8_t target_num);
