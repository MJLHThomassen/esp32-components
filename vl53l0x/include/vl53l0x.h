#ifndef VL53L0X_H
#define VL53L0X_H

#include <stdint.h>
#include <driver/i2c.h>

typedef enum vl53l0x_err_e
{
    VL53L0X_OK = 0,

    VL53L0X_ERR_ALLOC,
    VL53L0X_ERR_ADDRESS_OUT_OF_RANGE,
    VL53L0X_ERR_I2C,

    VL53L0X_FAIL = -1
} vl53l0x_err_t;

typedef struct vl53l0x_device_s
{
    i2c_port_t i2c_num;
    uint8_t i2c_addr_byte;

} vl53l0x_device_t;

typedef vl53l0x_device_t* vl53l0x_device_handle_t;

vl53l0x_err_t vl53l0x_add_device(const i2c_port_t i2cNum, const uint8_t i2cAddr, vl53l0x_device_handle_t* handle);
vl53l0x_err_t vl53l0x_remove_device(vl53l0x_device_handle_t handle);

#endif // VL53L0X_H