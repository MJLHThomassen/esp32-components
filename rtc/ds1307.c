#include "ds1307.h"

#include <esp_err.h>
#include <esp_log.h>
#include <freertos/task.h>

#define BCD_TO_DEC(x) (((x) & 0x0F) + 10 * ((x) >> 4))
#define DEC_TO_BCD(x) (((x) % 10) | (((x) / 10) << 4))

#define CH_BIT_MASK 0x80
#define HOUR_MODE_BIT_MASK 0x40
#define AMPM_BIT_MASK 0x20

typedef struct ds1307_timerkeeper_registers_s
{
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint8_t year;
} ds1307_timerkeeper_registers_t;

static ds1307_err_t ds1307_read_registers(const ds1307_device_handle_t handle, uint8_t address, uint8_t* buffer, size_t length)
{
    esp_err_t res;
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, handle->i2c_addr_byte | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, address, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, handle->i2c_addr_byte | I2C_MASTER_READ, true);
    i2c_master_read(cmd, buffer, length, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    res = i2c_master_cmd_begin(handle->i2c_num, cmd, pdMS_TO_TICKS(1000));
    
    i2c_cmd_link_delete(cmd);

    if(res != ESP_OK)
    {
        return DS1073_ERR_I2C;
    }
    
    return DS1307_OK;
}

static ds1307_err_t ds1307_write_registers(const ds1307_device_handle_t handle, uint8_t address, uint8_t* data, size_t length)
{
    esp_err_t res;
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, handle->i2c_addr_byte | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, address, true);
    i2c_master_write(cmd, data, length, true);
    i2c_master_stop(cmd);
    res = i2c_master_cmd_begin(handle->i2c_num, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    if(res != ESP_OK)
    {
        return DS1073_ERR_I2C;
    }
    
    return DS1307_OK;
}

ds1307_err_t ds1307_add_device(const i2c_port_t i2cNum, const uint8_t i2cAddr, ds1307_device_handle_t* handle)
{
    ds1307_device_handle_t newHandle = (ds1307_device_handle_t) malloc(sizeof(*newHandle));

    if(newHandle == NULL)
    {
        return DS1073_ERR_ALLOC;
    }

    newHandle->i2c_num = i2cNum;
    newHandle->i2c_addr_byte = i2cAddr << 1;

    *handle = newHandle;

    return DS1307_OK;
}

ds1307_err_t ds1307_remove_device(ds1307_device_handle_t handle)
{
    free(handle);

    return DS1307_OK;
}

ds1307_err_t ds1307_set_time(const ds1307_device_handle_t handle, time_t time)
{
    // Convert time_t into seconds, minutes, hours, day, date, month and year
    struct tm* localTime = localtime(&time);

    ds1307_timerkeeper_registers_t timeRegs = {
        .seconds = DEC_TO_BCD(localTime->tm_sec) & ~CH_BIT_MASK, // Clear the CH bit to start the ds1307 oscilator
        .minutes = DEC_TO_BCD(localTime->tm_min),
        .hours = DEC_TO_BCD(localTime->tm_hour) & ~(0x80 | HOUR_MODE_BIT_MASK), // Clear Bit 7 and Bit 6 to set ds1307 in 24-Hour mode
        .day = DEC_TO_BCD(localTime->tm_wday + 1), // tm_wday is days since Sunday - [0, 6], ds1307 starts counting day at 1
        .date = DEC_TO_BCD(localTime->tm_mday),
        .month = DEC_TO_BCD(localTime->tm_mon + 1), // tm_mon is months since January - [0, 11], ds1307 starts counting months at 1
        .year = DEC_TO_BCD(localTime->tm_year - 100) // tm_year is years since 1900, ds1307 starts counting in 2000
    };

    return ds1307_write_registers(handle, 0x00, &timeRegs, sizeof(timeRegs));
}

ds1307_err_t ds1307_get_time(const ds1307_device_handle_t handle, time_t* time)
{
    // Read timekeeper registers from device
    ds1307_timerkeeper_registers_t timeRegs;
    ds1307_err_t err = ds1307_read_registers(handle, 0x00, &timeRegs, sizeof(timeRegs));

    if(err != DS1307_OK)
    {
        time = NULL;
        return err;
    }

    // Convert hours register into hours since midnight - [0, 23]
    uint8_t hoursSinceMidnight = 0;
    if(timeRegs.hours & HOUR_MODE_BIT_MASK)
    {
        // 12-hour mode
        hoursSinceMidnight = BCD_TO_DEC(timeRegs.hours & ~(HOUR_MODE_BIT_MASK | AMPM_BIT_MASK)); // Mask the hour mode and am/pm bit

        if(timeRegs.hours & AMPM_BIT_MASK)
        {
            // Time is PM
            if(hoursSinceMidnight < 12)
            {
                // Add 12 hours to the time to get actual hoursSinceMidnight of day
                hoursSinceMidnight += 12;
            }
            else
            {
                // 12PM is 00:00 so hoursSinceMidnight is 0
                hoursSinceMidnight = 0;
            }
        }
    }
    else
    {
        // 24-hour mode
        hoursSinceMidnight = BCD_TO_DEC(timeRegs.hours & ~HOUR_MODE_BIT_MASK); // Mask the hour mode bit
    }

    // Convert binary-coded-decimal timekeeper registers into time_t
    struct tm timeStruct = {
        .tm_sec = BCD_TO_DEC(timeRegs.seconds & ~CH_BIT_MASK), // Mask the CH bit
        .tm_min = BCD_TO_DEC(timeRegs.minutes),
        .tm_hour = hoursSinceMidnight,
        .tm_mday = BCD_TO_DEC(timeRegs.date),
        .tm_mon = BCD_TO_DEC(timeRegs.month) - 1, // ds1307 starts counting months at 1, need to specify months since January - [0, 11]
        .tm_year = BCD_TO_DEC(timeRegs.year) + 100, // ds1307 starts counting in 2000, need to specify years since 1900
    };

    *time = mktime(&timeStruct);

    return DS1307_OK;
}

ds1307_err_t ds1307_read_ram(const ds1307_device_handle_t handle, uint8_t address, uint8_t* buffer, size_t length)
{
    if(address < 0x08 || address > 0x3f)
    {
        // Can't read
        buffer = NULL;
        return DS1073_ERR_ADDRESS_OUT_OF_RANGE;
    }

    return ds1307_read_registers(handle, address, buffer, length);
}

ds1307_err_t ds1307_write_ram(const ds1307_device_handle_t handle, uint8_t address, uint8_t* data, size_t length)
{
    if(address < 0x08 || address > 0x3f)
    {
        // Can't write
        return DS1073_ERR_ADDRESS_OUT_OF_RANGE;
    }

    return ds1307_write_registers(handle, address, data, length);
}