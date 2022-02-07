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
#include "luat_malloc.h"
#include "luat_msgbus.h"
#include "luat_timer.h"
#include "luat_gpio.h"
#include "core_gpio.h"
#include "platform_define.h"
#include "luat_irq.h"

typedef struct wm_gpio_conf
{
    luat_gpio_irq_cb cb;
    void* args;
}wm_gpio_conf_t;


static wm_gpio_conf_t confs[HAL_GPIO_MAX];

static void luat_gpio_irq_callback(void *ptr, void *pParam)
{
    int pin = (int)ptr;
    luat_gpio_irq_cb cb = confs[pin].cb;
    if (cb == NULL)
        luat_irq_gpio_cb(pin, confs[pin].args);
    else
        cb(pin, confs[pin].args);
}

int luat_gpio_setup(luat_gpio_t *gpio){
    if (gpio->pin < HAL_GPIO_2 || gpio->pin > HAL_GPIO_MAX) return 0;
    GPIO_Iomux(gpio->pin, 1);
    switch (gpio->mode){
        case Luat_GPIO_OUTPUT:
            GPIO_Config(gpio->pin, 0, 0);
            break;
        case Luat_GPIO_INPUT:
            GPIO_Config(gpio->pin, 1, 0);
            break;
        case Luat_GPIO_IRQ:{
            switch (gpio->irq){
                case Luat_GPIO_RISING:
                    GPIO_ExtiConfig(gpio->pin, 0,1,0);
                    break;
                case Luat_GPIO_FALLING:
                    GPIO_ExtiConfig(gpio->pin, 0,0,1);
                    break;
                case Luat_GPIO_BOTH:
                    GPIO_ExtiConfig(gpio->pin, 0,1,1);
                default:
                    break;
            }
            if (gpio->irq_cb) {
                confs[gpio->pin].cb = gpio->irq_cb;
                confs[gpio->pin].args = gpio->irq_args;
            }

        }break;
        default:
            break;
    }
    switch (gpio->pull){
        case Luat_GPIO_PULLUP:
            GPIO_PullConfig(gpio->pin, 1, 1);
            break;
        case Luat_GPIO_PULLDOWN:
            GPIO_PullConfig(gpio->pin, 1, 0);
            break;
        case Luat_GPIO_DEFAULT:
            GPIO_PullConfig(gpio->pin, 0, 0);
        default:
            break;
    }
    return 0;
}

int luat_gpio_set(int pin, int level){
    if (pin < HAL_GPIO_2 || pin >= HAL_GPIO_MAX) return 0;
    GPIO_Output(pin, level);
    return 0;
}

int luat_gpio_get(int pin){
    if (pin < HAL_GPIO_2 || pin >= HAL_GPIO_MAX) return 0;
    int re = GPIO_Input(pin);
    return re;
}

void luat_gpio_close(int pin){
    if (pin < HAL_GPIO_2 || pin >= HAL_GPIO_MAX) return ;
    confs[pin].cb = NULL;
    return ;
}

void luat_gpio_init(void){
    GPIO_GlobalInit(luat_gpio_irq_callback);
}

int luat_gpio_set_irq_cb(int pin, luat_gpio_irq_cb cb, void* args) {
    if (pin < HAL_GPIO_2 || pin >= HAL_GPIO_MAX) return -1;
    if (cb) {
        confs[pin].cb = cb;
        confs[pin].args = args;
    }
    return 0;
}
