#ifndef DS1307_H
#define DS1307_H

#include <stdint.h>
#include <time.h>
#include <driver/i2c.h>

typedef enum ds1307_err_e
{
    DS1307_OK = 0,

    DS1073_ERR_ALLOC,
    DS1073_ERR_ADDRESS_OUT_OF_RANGE,
    DS1073_ERR_I2C,

    DS1307_FAIL = -1
} ds1307_err_t;

typedef struct ds1307_device_s
{
    i2c_port_t i2c_num;
    uint8_t i2c_addr_byte;

} ds1307_device_t;

typedef ds1307_device_t* ds1307_device_handle_t;

ds1307_err_t ds1307_add_device(const i2c_port_t i2cNum, const uint8_t i2cAddr, ds1307_device_handle_t* handle);
ds1307_err_t ds1307_remove_device(ds1307_device_handle_t handle);

ds1307_err_t ds1307_set_time(const ds1307_device_handle_t handle, time_t time);
ds1307_err_t ds1307_get_time(const ds1307_device_handle_t handle, time_t* time);

ds1307_err_t ds1307_read_ram(const ds1307_device_handle_t handle, uint8_t address, uint8_t* buffer, size_t length);
ds1307_err_t ds1307_write_ram(const ds1307_device_handle_t handle, uint8_t address, uint8_t* data, size_t length);

#endif // DS1307_H