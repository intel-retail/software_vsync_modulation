/*
 * Copyright © 2024 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <vsyncalter.h>
#include <debug.h>
#include <stdlib.h>
#include <memory.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <xf86drm.h>
#include "common.h"
#include "dkl.h"
#include "combo.h"


/**
* @brief
* This function calculates how many steps we need to take
* in order to synchronize the primary and secondary systems given the delta
* between primary and secondary and the shift that we need to make in terms of
* percentage. Each steps is a single vsync period (typically 16.666 ms).
* @param time_diff - The time difference in between the two systems in ms.
* @param shift - The percentage shift that we need to make in our vsyncs.
* @return int
*/

int calc_steps_to_sync(double time_diff, double shift)
{
	return (int) ((time_diff * 100) / shift);
}

#if !TESTING

/**
* @brief
*	This function creates a timer.
* @param  expire_ms - The time period in ms after which the timer will fire.
* @param *user_ptr - A pointer to pass to the timer handler
* @param *t - A pointer to a pointer where we need to store the timer
* @return
* - 0 == SUCCESS
* - -1 = FAILURE
*/
int make_timer(long expire_ms, void *user_ptr, timer_t *t, reset_func reset)
{
	struct sigevent         te;
	struct itimerspec       its;
	struct sigaction        sa;
	int                     sig_no = SIGRTMIN;


	// Set up signal handler
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = reset;
	sigemptyset(&sa.sa_mask);
	if (sigaction(sig_no, &sa, NULL) == -1) {
		ERR("Failed to setup signal handling for timer.\n");
		return -1;
	}

	// Set and enable alarm
	te.sigev_notify = SIGEV_SIGNAL;
	te.sigev_signo = sig_no;
	te.sigev_value.sival_ptr = user_ptr;
	timer_create(CLOCK_REALTIME, &te, t);

	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = TV_NSEC(expire_ms);
	its.it_value.tv_sec = TV_SEC(expire_ms);
	its.it_value.tv_nsec = TV_NSEC(expire_ms);
	timer_settime(*t, 0, &its, NULL);

	return 0;
}
#endif
