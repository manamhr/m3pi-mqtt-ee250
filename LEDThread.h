/**
 * Copyright (c) 2017, Autonomous Networks Research Group. All rights reserved.
 * Developed by:
 * Autonomous Networks Research Group (ANRG)
 * University of Southern California
 * http://anrg.usc.edu/
 *
 * Contributors:
 * Jason A. Tran <jasontra@usc.edu>
 * Bhaskar Krishnamachari <bkrishna@usc.edu>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal
 * with the Software without restriction, including without limitation the 
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
 * sell copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 * - Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimers.
 * - Redistributions in binary form must reproduce the above copyright notice, 
 *     this list of conditions and the following disclaimers in the 
 *     documentation and/or other materials provided with the distribution.
 * - Neither the names of Autonomous Networks Research Group, nor University of 
 *     Southern California, nor the names of its contributors may be used to 
 *     endorse or promote products derived from this Software without specific 
 *     prior written permission.
 * - A citation to the Autonomous Networks Research Group must be included in 
 *     any publications benefiting from the use of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH 
 * THE SOFTWARE.
 */

/**
 * @file       LEDThread.h
 * @brief      Thread that handles LED requests.
 *
 * @author     Jason Tran <jasontra@usc.edu>
 * @author     Bhaskar Krishnachari <bkrishna@usc.edu>
 */

#ifndef _LEDTHREAD_H_
#define _LEDTHREAD_H_

#include "rtos.h"
#include "MailMsg.h"

#define LEDTHREAD_MAILBOX_SIZE  32

/**
 * @brief      Main LED thread function.
 *
 * @param      args  Pointer to the MQTT client so the LED thread can .publish().
 */
void LEDThread(void *args);

/**
 * @brief      Returns a pointer to the led thread's mailbox
 * @return     Pointer to LED thread's mailbox
 */
Mail<MailMsg, LEDTHREAD_MAILBOX_SIZE> *getLEDThreadMailbox();

// void readFrontSensor();
// void readBackSensor();

#endif /* _LEDTHREAD_H_ */
