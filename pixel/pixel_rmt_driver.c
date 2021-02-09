#include "pixel.h"

#include "esp_system.h"

#include "driver/gpio.h"

static void pixel_set_color(pixel_device_t* const handle, const pixel_color_t* const color)
{
    for (int i = 0; i < handle->nrOfColors; ++i)
    {
        uint8_t colorChannelIdx = handle->colorSequence[i];
        uint8_t* colorChannel = &color->channels[colorChannelIdx];

        for (int bitIdx = 0; bitIdx < 8; bitIdx++)
        {
            handle->items[i * 8 + bitIdx] = ((*colorChannel) & (0x80 >> bitIdx)) ? handle->rmt_item_1 : handle->rmt_item_0;
        }
    }

    // Wait for ongoing transmissions to complete
    ESP_ERROR_CHECK(rmt_wait_tx_done(handle->rmt_channel, portMAX_DELAY));

    uint8_t nrOfItems = handle->nrOfColors * 8;
    ESP_ERROR_CHECK(rmt_write_items(handle->rmt_channel, handle->items, nrOfItems, false));
}

static void pixel_reset(const pixel_device_t* const handle)
{
    // Wait for ongoing transmissions to complete
    ESP_ERROR_CHECK(rmt_wait_tx_done(handle->rmt_channel, portMAX_DELAY));
    ESP_ERROR_CHECK(rmt_write_items(handle->rmt_channel, &handle->rmt_item_res, 1, false));
}

void pixel_add_device(gpio_num_t gpioNum, pixel_device_type_t device_type, pixel_device_t* const handle)
{
    rmt_config_t config = {
        .rmt_mode = RMT_MODE_TX,
        .channel = RMT_CHANNEL_0,
        .clk_div = 1,
        .gpio_num = gpioNum,
        .mem_block_num = 1,

        .tx_config.loop_en = 0,
        .tx_config.carrier_freq_hz = 0,
        .tx_config.carrier_duty_percent = 0,
        .tx_config.carrier_level = 1,
        .tx_config.carrier_en = false,
        .tx_config.idle_level = 0,
        .tx_config.idle_output_en = true};

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));

    handle->rmt_channel = config.channel;

    switch(device_type)
    {
        case WS2812:
            handle->rmt_item_0 = (rmt_item32_t){{{ RMT_TICKS(WS2812_T0H), 1 , RMT_TICKS(WS2812_T0L), 0 }}};
            handle->rmt_item_1 = (rmt_item32_t){{{ RMT_TICKS(WS2812_T1H), 1, RMT_TICKS(WS2812_T1L), 0 }}};
            handle->rmt_item_res = (rmt_item32_t){{{ RMT_TICKS(WS2812_RES), 0, 0, 0 }}};

            handle->nrOfColors = 3;
            handle->colorSequence[REDIDX]   = 1;
            handle->colorSequence[GREENIDX] = 0;
            handle->colorSequence[BLUEIDX]  = 2;
            break;

        case SK6812:
            handle->rmt_item_0 = (rmt_item32_t){{{ RMT_TICKS(SK6812_T0H), 1 , RMT_TICKS(SK6812_T0L), 0}}};
            handle->rmt_item_1 = (rmt_item32_t){{{ RMT_TICKS(SK6812_T1H), 1, RMT_TICKS(SK6812_T1L), 0 }}};
            handle->rmt_item_res = (rmt_item32_t){{{ RMT_TICKS(SK6812_RES), 0, 0, 0 }}};

            handle->nrOfColors = 3;
            handle->colorSequence[REDIDX]   = 1;
            handle->colorSequence[GREENIDX] = 0;
            handle->colorSequence[BLUEIDX]  = 2;
            break;

        case SK6812RGBW:
            handle->rmt_item_0 = (rmt_item32_t){{{ RMT_TICKS(SK6812RGBW_T0H), 1 , RMT_TICKS(SK6812RGBW_T0L), 0}}};
            handle->rmt_item_1 = (rmt_item32_t){{{ RMT_TICKS(SK6812RGBW_T1H), 1, RMT_TICKS(SK6812RGBW_T1L), 0 }}};
            handle->rmt_item_res = (rmt_item32_t){{{ RMT_TICKS(SK6812RGBW_RES), 0, 0, 0 }}};

            handle->nrOfColors = 4;
            handle->colorSequence[REDIDX]   = 1;
            handle->colorSequence[GREENIDX] = 0;
            handle->colorSequence[BLUEIDX]  = 2;
            handle->colorSequence[WHITEIDX] = 3;
            break;

        default:
            // Unknown
            handle->rmt_item_0 = (rmt_item32_t){{{ 0, 0, 0, 0 }}};
            handle->rmt_item_1 = (rmt_item32_t){{{ 0, 0, 0, 0 }}};
            handle->rmt_item_res = (rmt_item32_t){{{ 0, 0, 0, 0 }}};
            return;
    }
}

void pixel_remove_device(const pixel_device_t* const handle)
{
    ESP_ERROR_CHECK(rmt_driver_uninstall(handle->rmt_channel));
}

void pixel_set_colors(pixel_device_t* const handle, const pixel_color_t* const colors, size_t length)
{
    for(size_t i = 0; i < length; ++i)
    {
        pixel_set_color(handle, &colors[i]);
    }
    pixel_reset(handle);
}