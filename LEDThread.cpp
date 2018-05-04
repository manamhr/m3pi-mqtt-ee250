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
 * @file       LEDThread.cpp
 * @brief      Implementation of thread that handles LED requests.
 *
 * @author     Jason Tran <jasontra@usc.edu>
 * @author     Bhaskar Krishnachari <bkrishna@usc.edu>
 */

#include "LEDThread.h"
#include "MQTTmbed.h"
#include "MQTTNetwork.h"
#include "mbed.h"
#include "m3pi.h"
#include "MQTTClient.h"

Mail<MailMsg, LEDTHREAD_MAILBOX_SIZE> LEDMailbox;

static DigitalOut led2(LED2);

static const char *topic1 = "r2d2/front";
static const char *topic2 = "r2d2/rear";

m3pi m3pi(p23, p9, p10);

// extern void movement(char command, char speed, int delta_t);
const int SIZE = 4;
char pub_buf[16];
int dist;

// Min: 1
// Max: 100
int speed = 35;

// 0: CCW
// 1: CW
int angle = 0;

// direction = 0 -> CCW
// direction = 1 -> CW
void rotate(int speed, int wait, int direction) {
    if (direction)  {
        m3pi.right(speed);
        Thread::wait(wait);
        m3pi.stop();
    }
    else    {
         m3pi.left(speed);
        Thread::wait(wait);
        m3pi.stop();
    }
}


void movement(char command, char speed, int delta_t)    {
    if (command == 's')
    {
        m3pi.backward(speed);
        Thread::wait(delta_t);
        m3pi.stop();
    }    
    else if (command == 'a')
    {
        m3pi.left(speed);
        Thread::wait(delta_t);
        m3pi.stop();
    }   
    else if (command == 'w')
    {
        m3pi.forward(speed);
        Thread::wait(delta_t);
        m3pi.stop();
    }
    else if (command == 'd')
    {
        m3pi.right(speed);
        Thread::wait(delta_t);
        m3pi.stop();
    }
}

void readSensor(MQTT::Client<MQTTNetwork, Countdown> *client, MailMsg *msg, MQTT::Message message, osEvent evt, int sensor)  {
    double voltage = 0, newVoltage = 0;

    if (sensor == 0)    {
        AnalogIn Ain(p15);
        voltage = Ain.read();
    }
    else    {
        AnalogIn Ain(p16);
        voltage = Ain.read();
    }

    dist = (voltage / 0.0064) * 2.54;

    if (dist < 100) {
        pub_buf[0] = (dist / 10) + 48;
        pub_buf[1] = (dist % 10) + 48;
        pub_buf[2] = ' ';
        pub_buf[3] = 'c';
        pub_buf[4] = 'm';
        pub_buf[5] = ' ';
    }
    else    {
        pub_buf[0] = (dist / 100) + 48;
        pub_buf[1] = ((dist / 10) % 10) + 48;
        pub_buf[2] = (dist % 10) + 48;
        pub_buf[3] = ' ';
        pub_buf[4] = 'c';
        pub_buf[5] = 'm';
    }

    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*)pub_buf;
    message.payloadlen = 6; //MQTTclient.h takes care of adding null char?
    /* Lock the global MQTT mutex before publishing */
    mqttMtx.lock();
    if (!sensor)
        client->publish(topic1, message);
    else
        client->publish(topic2, message);
    mqttMtx.unlock();
}

// void readBackSensor(MQTT::Client<MQTTNetwork, Countdown> *client, MailMsg *msg, MQTT::Message message, osEvent evt)  {
//     double voltage = 0;
//     AnalogIn Ain(p16);
//     voltage = Ain.read();
//     dist = (voltage / 0.0064) * 2.54;

//     if (dist < 100) {
//         pub_buf[0] = (dist / 10) + 48;
//         pub_buf[1] = (dist % 10) + 48;
//         pub_buf[2] = ' ';
//         pub_buf[3] = 'c';
//         pub_buf[4] = 'm';
//         pub_buf[5] = ' ';
//     }
//     else    {
//         pub_buf[0] = (dist / 100) + 48;
//         pub_buf[1] = ((dist / 10) % 10) + 48;
//         pub_buf[2] = (dist % 10) + 48;
//         pub_buf[3] = ' ';
//         pub_buf[4] = 'c';
//         pub_buf[5] = 'm';
//     }

//     message.qos = MQTT::QOS0;
//     message.retained = false;
//     message.dup = false;
//     message.payload = (void*)pub_buf;
//     message.payloadlen = 6; //MQTTclient.h takes care of adding null char?
//     /* Lock the global MQTT mutex before publishing */
//     mqttMtx.lock();
//     client->publish(topic2, message);
//     mqttMtx.unlock();
// }

void LEDThread(void *args) 
{
    MQTT::Client<MQTTNetwork, Countdown> *client = (MQTT::Client<MQTTNetwork, Countdown> *)args;

    MailMsg *msg;
    MQTT::Message message;
    osEvent evt;

    while(1) {

        evt = LEDMailbox.get();

        if(evt.status == osEventMail) {
            msg = (MailMsg *)evt.value.p;

            /* the second byte in the message denotes the action type */
            switch (msg->content[1]) {
                // Move forward
                case '3':
                    readSensor(client, msg, message, evt, 0);
                    if (dist < 26)  {
                        rotate(35, 220, angle);
                        Thread::wait(1);
                        readSensor(client, msg, message, evt, 0);
                        if (dist < 26)  {
                            rotate(35, 220, angle);
                            movement('w', speed, 400);
                        }
                        else    {
                            movement('w', speed, 400);
                        }

                    }
                    else    {
                        movement('w', speed, 400);
                    }
                    // Front
                    readSensor(client, msg, message, evt, 0);

                    // Rear
                    readSensor(client, msg, message, evt, 1);

                    break;
                // Move backward
                case '4':
                    // Rear
                    readSensor(client, msg, message, evt, 1);
                    if (dist < 26)  {
                        rotate(35, 220, angle);
                        Thread::wait(1);
                        readSensor(client, msg, message, evt, 1);
                        if (dist < 26)  {
                            rotate(35, 220, angle);
                            movement('s', speed, 400);
                        }
                        else    {
                            movement('s', speed, 400);
                        }

                    }
                    else    {
                        movement('s', speed, 400);
                    }
                    
                    // Front
                    readSensor(client, msg, message, evt, 0);

                    // Rear
                    readSensor(client, msg, message, evt, 1);

                    break;
                // Rotate CW
                case '5':
                    rotate(35, 130, 1);

                    // Front
                    readSensor(client, msg, message, evt, 0);

                    // Rear
                    readSensor(client, msg, message, evt, 1);

                    break;
                // Rotate CCW
                case '6':
                    rotate(35, 130, 0);

                    // Front
                    readSensor(client, msg, message, evt, 0);

                    // Rear
                    readSensor(client, msg, message, evt, 1);

                    break;
                // Avoid to CW
                case '7':
                    angle = 1;
                    break;
                // Avoid to CCW
                case '8':
                    angle = 0;
                    break;
                case '9':
                    speed = atoi(msg->content + 2);
                default:
                    printf("LEDThread: invalid message\n");
                    break;
            }            

            LEDMailbox.free(msg);
        }
    } /* while */

    /* this should never be reached */

}


Mail<MailMsg, LEDTHREAD_MAILBOX_SIZE> *getLEDThreadMailbox() 
{
    return &LEDMailbox;
}