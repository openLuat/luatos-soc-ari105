
#include "luat_base.h"
#include "luat_malloc.h"
#include "luat_timer.h"
#include "luat_msgbus.h"
#include "app_interface.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#define LUAT_LOG_TAG "luat.timer"
#include "luat_log.h"

#define FREERTOS_TIMER_COUNT 128
static luat_timer_t* timers[FREERTOS_TIMER_COUNT] = {0};
/*
static void luat_timer_callback(TimerHandle_t xTimer) {
    //LLOGD("timer callback");
    rtos_msg_t msg;
    luat_timer_t *timer = (luat_timer_t*) pvTimerGetTimerID(xTimer);
    msg.handler = timer->func;
    msg.ptr = timer;
    msg.arg1 = 0;
    msg.arg2 = 0;
    int re = luat_msgbus_put(&msg, 0);
    //LLOGD("timer msgbus re=%ld", re);
}
*/
static int32_t luat_timer_callback(void *pData, void *pParam)
{
    rtos_msg_t msg;
    luat_timer_t *timer = (luat_timer_t*)pParam;
    msg.handler = timer->func;
    msg.ptr = timer;
    msg.arg1 = 0;
    msg.arg2 = 0;
    luat_msgbus_put(&msg, 0);
}

static int nextTimerSlot() {
    for (size_t i = 0; i < FREERTOS_TIMER_COUNT; i++)
    {
        if (timers[i] == NULL) {
            return i;
        }
    }
    return -1;
}

int luat_timer_start(luat_timer_t* timer) {
    Timer_t *os_timer;
    int timerIndex;
    //LLOGD(">>luat_timer_start timeout=%ld", timer->timeout);
    timerIndex = nextTimerSlot();
    //LLOGD("timer id=%ld", timerIndex);
    if (timerIndex < 0) {
        return 1; // too many timer!!
    }
    os_timer = Timer_Create(luat_timer_callback, timer, NULL);
    //LLOGD("timer id=%ld, osTimerNew=%p", timerIndex, os_timer);
    if (!os_timer) {
        return -1;
    }
    timers[timerIndex] = timer;
    
    timer->os_timer = os_timer;
    return Timer_StartMS(os_timer, timer->timeout, timer->repeat);
}

int luat_timer_stop(luat_timer_t* timer) {
    if (!timer)
        return 1;
    for (size_t i = 0; i < FREERTOS_TIMER_COUNT; i++)
    {
        if (timers[i] == timer) {
            timers[i] = NULL;
            break;
        }
    }
    Timer_Release(timer->os_timer);
    return 0;
};

luat_timer_t* luat_timer_get(size_t timer_id) {
    for (size_t i = 0; i < FREERTOS_TIMER_COUNT; i++)
    {
        if (timers[i] && timers[i]->id == timer_id) {
            return timers[i];
        }
    }
    return NULL;
}


int luat_timer_mdelay(size_t ms) {
    Task_DelayMS(ms);
    return 0;
}


void luat_timer_us_delay(size_t time){
	Task_DelayUS(time);
}
