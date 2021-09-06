#include "vl53l0x.h"

#include <esp_err.h>
#include <esp_log.h>

static vl53l0x_err_t vl53l0x_read_registers(const vl53l0x_device_handle_t handle, uint8_t address, uint8_t* buffer, size_t length)
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
        return VL53L0X_ERR_I2C;
    }
    
    return VL53L0X_OK;
}

static vl53l0x_err_t vl53l0x_write_registers(const vl53l0x_device_handle_t handle, uint8_t address, uint8_t* data, size_t length)
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
        return VL53L0X_ERR_I2C;
    }
    
    return VL53L0X_OK;
}

vl53l0x_err_t vl53l0x_add_device(const i2c_port_t i2cNum, const uint8_t i2cAddr, vl53l0x_device_handle_t* handle)
{
    vl53l0x_device_handle_t newHandle = (vl53l0x_device_handle_t) malloc(sizeof(*newHandle));

    if(newHandle == NULL)
    {
        return VL53L0X_ERR_ALLOC;
    }

    newHandle->i2c_num = i2cNum;
    newHandle->i2c_addr_byte = i2cAddr << 1;

    *handle = newHandle;

    return VL53L0X_OK;
}

vl53l0x_err_t vl53l0x_remove_device(vl53l0x_device_handle_t handle)
{
    free(handle);

    return VL53L0X_OK;
}