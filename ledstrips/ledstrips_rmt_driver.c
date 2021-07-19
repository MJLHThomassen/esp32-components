#include "ledstrips.h"

#include <esp_system.h>

#include <soc/soc.h>
#include <driver/rmt.h>

#include <logger.h>

#include <math.h>

#define RMT_TICKS(x) x*APB_CLK_FREQ

static const char* TAG = "ledstrips_rmt_driver";
static const size_t cgNrOfRmtItemsPerColor = 8;

struct ledstrips_device_s
{
    size_t length;

    uint8_t nrOfColors;
    uint8_t colorSequence[4];

    rmt_channel_t rmt_channel;
    rmt_item32_t rmt_item_0;
    rmt_item32_t rmt_item_1;
    rmt_item32_t rmt_item_res;

    size_t items_length;
    rmt_item32_t* items;
};

static void rmt_transmission(rmt_channel_t rmt_channel, const rmt_item32_t* items, size_t length)
{
    esp_err_t err;

    // Wait for ongoing transmissions to complete
    err = rmt_wait_tx_done(rmt_channel, portMAX_DELAY);
    if(err != ESP_OK)
    {
        switch(err)
        {
            case ESP_ERR_TIMEOUT:
                LOG_E(TAG, "rmt_wait_tx_done timeout waiting");
                break;
            case ESP_ERR_INVALID_ARG:
                LOG_E(TAG, "rmt_wait_tx_done parameter error");
                break;
            case ESP_FAIL:
                LOG_E(TAG, "rmt_wait_tx_done driver not installed");
                break;
            default:
                LOG_E(TAG, "rmt_wait_tx_done unknown error: %d", err);
        }
    }

    err = rmt_write_items(rmt_channel, items, length, false);
    if(err != ESP_OK)
    {
        switch(err)
        {
            case ESP_ERR_INVALID_ARG:
                LOG_E(TAG, "rmt_write_items parameter error");
                break;
            default:
                LOG_E(TAG, "rmt_write_items unknown error: %d", err);
        }
    }
}

static void ledstrips_set_items_for_color(const ledstrips_device_handle_t handle, const ledstrips_color_t* const color, size_t led_idx)
{
    size_t baseIdx = led_idx * handle->nrOfColors * cgNrOfRmtItemsPerColor;

    for (int colorIdx = 0; colorIdx < handle->nrOfColors; ++colorIdx)
    {
        uint8_t colorChannelIdx = handle->colorSequence[colorIdx];
        uint8_t* const colorChannel = &(color->channels[colorChannelIdx]);

        for (int bitIdx = 0; bitIdx < 8; bitIdx++)
        {
            handle->items[baseIdx + colorIdx * 8 + bitIdx] = ((*colorChannel) & (0x80 >> bitIdx)) ? handle->rmt_item_1 : handle->rmt_item_0;
        }
    }
}

static void ledstrips_reset(const ledstrips_device_handle_t handle)
{
    rmt_transmission(handle->rmt_channel, &handle->rmt_item_res, 1);
}

void ledstrips_add_device(gpio_num_t gpioNum, ledstrips_chip_type_t chip_type, size_t length, ledstrips_device_handle_t* handle)
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

    ledstrips_device_handle_t newHandle = (ledstrips_device_handle_t)calloc(1, sizeof(*newHandle));
    if(newHandle == NULL)
    {
        LOG_E(TAG, "Can not allocate memory for ledstrips device handle");
        return;
    }

    // Assign the new handle
    *handle = newHandle;

    newHandle->length = length;
    newHandle->rmt_channel = config.channel;

    switch(chip_type)
    {
        case WS2812:
            newHandle->rmt_item_0 = (rmt_item32_t){{{ RMT_TICKS(WS2812_T0H), 1 , RMT_TICKS(WS2812_T0L), 0 }}};
            newHandle->rmt_item_1 = (rmt_item32_t){{{ RMT_TICKS(WS2812_T1H), 1, RMT_TICKS(WS2812_T1L), 0 }}};
            newHandle->rmt_item_res = (rmt_item32_t){{{ RMT_TICKS(WS2812_RES), 0, 0, 0 }}};

            newHandle->nrOfColors = 3;
            newHandle->colorSequence[REDIDX]   = 1;
            newHandle->colorSequence[GREENIDX] = 0;
            newHandle->colorSequence[BLUEIDX]  = 2;
            break;

        case SK6812:
            newHandle->rmt_item_0 = (rmt_item32_t){{{ RMT_TICKS(SK6812_T0H), 1 , RMT_TICKS(SK6812_T0L), 0}}};
            newHandle->rmt_item_1 = (rmt_item32_t){{{ RMT_TICKS(SK6812_T1H), 1, RMT_TICKS(SK6812_T1L), 0 }}};
            newHandle->rmt_item_res = (rmt_item32_t){{{ RMT_TICKS(SK6812_RES), 0, 0, 0 }}};

            newHandle->nrOfColors = 3;
            newHandle->colorSequence[REDIDX]   = 1;
            newHandle->colorSequence[GREENIDX] = 0;
            newHandle->colorSequence[BLUEIDX]  = 2;
            break;

        case SK6812RGBW:
            newHandle->nrOfColors = 4;
            newHandle->colorSequence[REDIDX]   = 1;
            newHandle->colorSequence[GREENIDX] = 0;
            newHandle->colorSequence[BLUEIDX]  = 2;
            newHandle->colorSequence[WHITEIDX] = 3;

            newHandle->rmt_item_0 = (rmt_item32_t){{{ RMT_TICKS(SK6812RGBW_T0H), 1 , RMT_TICKS(SK6812RGBW_T0L), 0}}};
            newHandle->rmt_item_1 = (rmt_item32_t){{{ RMT_TICKS(SK6812RGBW_T1H), 1, RMT_TICKS(SK6812RGBW_T1L), 0 }}};
            newHandle->rmt_item_res = (rmt_item32_t){{{ RMT_TICKS(SK6812RGBW_RES), 0, 0, 0 }}};

            newHandle->items_length = newHandle->length * newHandle->nrOfColors * cgNrOfRmtItemsPerColor;
            newHandle->items = (rmt_item32_t*)malloc(newHandle->items_length * sizeof(rmt_item32_t));
            if(newHandle->items == NULL)
            {
                LOG_E(TAG, "Can not allocate memory for ledstrips items buffer");
                return;
            }

            break;

        default:
            // Unknown
            newHandle->rmt_item_0 = (rmt_item32_t){{{ 0, 0, 0, 0 }}};
            newHandle->rmt_item_1 = (rmt_item32_t){{{ 0, 0, 0, 0 }}};
            newHandle->rmt_item_res = (rmt_item32_t){{{ 0, 0, 0, 0 }}};
            return;
    }
}

void ledstrips_remove_device(ledstrips_device_handle_t handle)
{
    if(handle != NULL)
    {
        ESP_ERROR_CHECK(rmt_driver_uninstall(handle->rmt_channel));

        free(handle);
    }
}

void ledstrips_set_colors(const ledstrips_device_handle_t handle, const ledstrips_color_t* const colors, size_t length)
{
    // Ensure length does not exeed ledstrip length
    length= fmin(length, handle->length);

    for(size_t i = 0; i < length; ++i)
    {
        ledstrips_set_items_for_color(handle, &colors[i], i);
    }
    
    rmt_transmission(handle->rmt_channel, handle->items, handle->items_length);
    ledstrips_reset(handle);
}

void ledstrips_set_sequence(const ledstrips_device_handle_t handle, const ledstrips_color_t* const sequence_colors, size_t sequence_length)
{
    // Ensure sequence length does not exeed ledstrip length
    sequence_length = fmin(sequence_length, handle->length);

    for(size_t i = 0; i < handle->length; i += sequence_length)
    {
        for(size_t j = 0; j < sequence_length; ++j)
        {
            ledstrips_set_items_for_color(handle, &sequence_colors[j], i+j);
        }
    }

    rmt_transmission(handle->rmt_channel, handle->items, handle->items_length);
    ledstrips_reset(handle);
}