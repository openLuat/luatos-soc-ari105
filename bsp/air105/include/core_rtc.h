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

#ifndef __CORE_RTC_H__
#define __CORE_RTC_H__
#include "bsp_common.h"
void RTC_GlobalInit(void);

void RTC_SetDateTime(Date_UserDataStruct *pDate, Time_UserDataStruct *pTime, uint8_t isForce);

void RTC_GetDateTime(Date_UserDataStruct *pDate, Time_UserDataStruct *pTime);

uint64_t RTC_GetUTC(void);

void RTC_SetAlarm(uint32_t TimeSecond, CBFuncEx_t CB, void *pParam);
#endif
