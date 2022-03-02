/*
 * Copyright (c) 2022 OpenLuat & AirM2M
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "luat_base.h"
#include "luat_i2c.h"

#include "app_interface.h"

#define LUAT_LOG_TAG "luat.i2c"
#include "luat_log.h"

int luat_i2c_exist(int id) {
    return id == 0;
}
    
int luat_i2c_setup(int id, int speed, int slaveaddr) {
    if (speed == 0)
        speed = 100 * 1000; // SLOW
    else if (speed == 1)
        speed = 400 * 1000; // FAST
    else if (speed == 2)
        speed = 400 * 1000; // SuperFast
	GPIO_Iomux(GPIOE_06, 2);
	GPIO_Iomux(GPIOE_07, 2);
	I2C_MasterSetup(id, speed);
    return 0;
}

int luat_i2c_close(int id) {
    return 0;
}

int luat_i2c_send(int id, int addr, void* buff, size_t len) {
	return I2C_BlockWrite(id, addr, (const uint8_t *)buff, len, 100, NULL, NULL);
    // I2C_Prepare(id, addr, 1, NULL, NULL);
    // I2C_MasterXfer(id, I2C_OP_WRITE, 0, buff, len, 20);
}

int luat_i2c_recv(int id, int addr, void* buff, size_t len) {
	return I2C_BlockRead(id, addr, 0, (uint8_t *)buff, len, 100, NULL, NULL);
    // I2C_Prepare(id, addr, 1, NULL, NULL);
    // I2C_MasterXfer(id, I2C_OP_READ, 0, buff, len, 20);
}
