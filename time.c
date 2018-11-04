/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 by Sergey Fetisov <fsenok@gmail.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * version: 1.0 demo (7.02.2015)
 */

#include "time.h"

volatile uint32_t sysTimeTicks;
volatile uint32_t sysTimeDelayCounter;

void time_init(void)
{
    if (SysTick_Config(SystemCoreClock / 1000))
        while (1) {} /* Capture error */
}

volatile uint32_t msAddition = 0;

void SysTick_Handler(void)
{
    msAddition++; /* +1 ms */
}

uint32_t mtime(void)
{
    uint32_t ctrl;
    static int32_t res;
    uint32_t ticks;

    ctrl = SysTick->CTRL;

read:
    ticks = SysTick->VAL;
    res = msAddition;
    ctrl = SysTick->CTRL;
    if (ctrl & SysTick_CTRL_COUNTFLAG_Msk)
        goto read;

    return res;
}

void msleep(int ms)
{
    uint32_t t = mtime();
    while (true)
    {
        uint32_t t1 = mtime();
        if (t1 - t >= ms) break;
        if (t1 < t) break; /* overflow */
    }
}

static stmr_t *stmrs = NULL;

void stmr(void)
{
    stmr_t *tmr;
    uint32_t time;
    time = mtime();
    tmr = stmrs;
    while (tmr != NULL)
    {
        stmr_t *t;
        uint32_t elapsed;
        t = tmr;
        tmr = tmr->next;
        if ((t->flags & STMR_ACTIVE) == 0)
            continue;
        elapsed = time;
        elapsed -= t->event;
        if (elapsed < t->period)
            continue;
        t->proc(t);
        t->event = mtime();
    }
}

void stmr_init(stmr_t *tmr)
{
    tmr->period = 0;
    tmr->event = 0;
    tmr->flags = 0;
    tmr->data = NULL;
    tmr->proc = NULL;
    tmr->next = stmrs;
    stmrs = tmr;
}

void stmr_add(stmr_t *tmr)
{
    tmr->next = stmrs;
    stmrs = tmr;
}

void stmr_free(stmr_t *tmr)
{
    stmr_t *t;
    
    if (stmrs == NULL)
        return;

    if (tmr == stmrs)
    {
        stmrs = tmr->next;
        tmr->next = NULL;
        return;
    }

    t = stmrs;
    while (t->next != NULL)
    {
        if (t->next == tmr)
        {
            t->next = tmr->next;
            tmr->next = NULL;
            return;
        }
        t = t->next;
    }
}

void stmr_stop(stmr_t *tmr)
{
    tmr->flags &= ~(uint32_t)STMR_ACTIVE;
}

void stmr_run(stmr_t *tmr)
{
    tmr->flags |= STMR_ACTIVE;
    tmr->event = mtime();
}
