#ifndef PIXEL_H
#define PIXEL_H

#include "driver/rmt.h"

// WS2812 https://cdn-shop.adafruit.com/datasheets/WS2812.pdf
#define WS2812_T0L 0.00000035
#define WS2812_T1L 0.00000070
#define WS2812_T0H 0.00000080
#define WS2812_T1H 0.00000060
#define WS2812_RES 0.00005000

// SK6812 https://cdn-shop.adafruit.com/product-files/1138/SK6812+LED+datasheet+.pdf
#define SK6812_T0L 0.00000090
#define SK6812_T1L 0.00000060
#define SK6812_T0H 0.00000030
#define SK6812_T1H 0.00000060
#define SK6812_RES 0.00008000

// SK6812RGBW https://cdn-shop.adafruit.com/product-files/2757/p2757_SK6812RGBW_REV01.pdf
#define SK6812RGBW_T0L 0.00000090
#define SK6812RGBW_T1L 0.00000060
#define SK6812RGBW_T0H 0.00000030
#define SK6812RGBW_T1H 0.00000060
#define SK6812RGBW_RES 0.00008000

#define RMT_TICKS(x) x*APB_CLK_FREQ

/*
 * Device type enum
 */
typedef enum pixel_device_type_e
{
    WS2812,
    SK6812,
    SK6812RGBW
} pixel_device_type_t;

typedef enum pixel_color_sequence_e
{
    REDIDX = 0,
    GREENIDX = 1,
    BLUEIDX = 2,
    WHITEIDX = 3
} pixel_color_sequence_t;

/*
 * Pixel device struct
 */
typedef struct
{
    rmt_channel_t rmt_channel;
    rmt_item32_t rmt_item_0;
    rmt_item32_t rmt_item_1;
    rmt_item32_t rmt_item_res;

    uint8_t nrOfColors;
    uint8_t colorSequence[4];
    
    rmt_item32_t items[32];
} pixel_device_t;

/* 
 * Pixel color struct
 */
typedef struct
{
    union
    {
        struct
        {
            uint8_t red;
            uint8_t green;
            uint8_t blue;
            uint8_t white;
        };
        uint8_t channels[4];
    }; 

} __attribute__((packed)) pixel_color_t;

void pixel_add_device(gpio_num_t gpioNum, pixel_device_type_t type, pixel_device_t* const handle);
void pixel_remove_device(const pixel_device_t* const handle);

void pixel_set_colors(pixel_device_t* const handle, const pixel_color_t* const colors, size_t length);

#endif // PIXEL_H