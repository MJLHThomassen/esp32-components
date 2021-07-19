#ifndef LEDSTRIPS_H
#define LEDSTRIPS_H

#include <stddef.h>
#include <stdint.h>
#include <hal/gpio_types.h>

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

/*
 * @brief Chip type enum
 * 
 */
typedef enum ledstrips_chip_type_e
{
    WS2812,
    SK6812,
    SK6812RGBW
} ledstrips_chip_type_t;

/**
 * @brief Color sequence enum
 * 
 */
typedef enum ledstrips_color_sequence_e
{
    REDIDX = 0,
    GREENIDX = 1,
    BLUEIDX = 2,
    WHITEIDX = 3
} ledstrips_color_sequence_t;

/**
 * @brief Device struct
 * 
 */
typedef struct ledstrips_device_s* ledstrips_device_handle_t;

/**
 * @brief Color struct
 * 
 */
typedef struct ledstrips_color_s
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

} __attribute__((packed)) ledstrips_color_t;

/**
 * @brief Add a ledstrip device and allocate all resources required for the device.
 * 
 * @param gpioNum The GPIO number the led strip's data line is attached to
 * @param chip_type The chip type the led strip uses
 * @param length The length of the ledstrip in number of leds
 * @param handle Pointer to variable to hold the device handle
 */
void ledstrips_add_device(gpio_num_t gpioNum, ledstrips_chip_type_t chip_type, size_t length, ledstrips_device_handle_t* handle);

/**
 * @brief Removes a ledstrip device and releases all allocated recources associated with the device.
 * 
 * @param handle Handle to free
 */
void ledstrips_remove_device(ledstrips_device_handle_t handle);

/**
 * @brief Set the colors of the individual leds in the ledstrip.
 * 
 * @param handle Handle to ledstrip device
 * @param colors Array of color structs, one per led, in the order they are connected in
 * @param length Length of the array
 */
void ledstrips_set_colors(const ledstrips_device_handle_t handle, const ledstrips_color_t* const colors, size_t length);

/**
 * @brief Set the colors of the leds in the ledstrip according to a given sequence.
 * 
 * This will set the colors of the leds in the ledstrip according the specified sequence.
 * The sequence will repeat if it is shorter then the amount of leds in the ledstrip.
 * 
 * @param handle Handle to ledstrip device
 * @param sequence_colors Array of color structs, one per led in the sequence
 * @param sequence_length Length of the sequence array
 */
void ledstrips_set_sequence(const ledstrips_device_handle_t handle, const ledstrips_color_t* const sequence_colors, size_t sequence_length);

#endif // LEDSTRIPS_H