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
int dist;
int speed = 25;

// 0: CCW
// 1: CW
int angle = 0;

char pub_buf[16];

void rotateCCW(int speed) {
    m3pi.left(speed);
    Thread::wait(140);
    m3pi.stop();
}

void rotateCW(int speed)    {
    m3pi.right(speed);
    Thread::wait(140);
    m3pi.stop();
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

void readFrontSensor()  {
    double voltage = 0;
    AnalogIn Ain(p15);
    voltage = Ain.read();
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

    printf("Front: %d cm\n", dist);
}

void readBackSensor()  {
    double voltage = 0;
    AnalogIn Ain(p16);
    voltage = Ain.read();
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

    printf("Rear: %d cm\n", dist);
}

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
                case '0':
                    printf("LEDThread: received command to publish to topic"
                           "m3pi-mqtt-example/led-thread\n");
                    pub_buf[0] = 'h';
                    pub_buf[1] = 'i';
                    message.qos = MQTT::QOS0;
                    message.retained = false;
                    message.dup = false;
                    message.payload = (void*)pub_buf;
                    message.payloadlen = 2; //MQTTclient.h takes care of adding null char?
                    /* Lock the global MQTT mutex before publishing */
                    mqttMtx.lock();
                    client->publish(topic1, message);
                    mqttMtx.unlock();
                    break;
                case '1':
                    printf("LEDThread: received message to turn LED2 on for"
                           "one second...\n");
                    led2 = 1;
                    wait(1);
                    led2 = 0;
                    break;
                case '2':
                    printf("LEDThread: received message to blink LED2 fast for"
                           "one second...\n");
                    for(int i = 0; i < 10; i++)
                    {
                        led2 = !led2;
                        wait(0.1);
                    }
                    led2 = 0;
                    break;
                // Move forward
                case '3':
                    readFrontSensor();
                    message.qos = MQTT::QOS0;
                    message.retained = false;
                    message.dup = false;
                    message.payload = (void*)pub_buf;
                    message.payloadlen = 6; //MQTTclient.h takes care of adding null char?
                    /* Lock the global MQTT mutex before publishing */
                    mqttMtx.lock();
                    client->publish(topic1, message);
                    mqttMtx.unlock();

                    readBackSensor();
                    message.qos = MQTT::QOS0;
                    message.retained = false;
                    message.dup = false;
                    message.payload = (void*)pub_buf;
                    message.payloadlen = 6; //MQTTclient.h takes care of adding null char?
                    /* Lock the global MQTT mutex before publishing */
                    mqttMtx.lock();
                    client->publish(topic2, message);
                    mqttMtx.unlock();

                    movement('w', speed, 300);
                    // movement('w', speed, 100);
                    // movement('w', speed, 100);
                    // movement('w', speed, 100);
                    
                    break;
                // Move backward
                case '4':
                    readFrontSensor();
                    message.qos = MQTT::QOS0;
                    message.retained = false;
                    message.dup = false;
                    message.payload = (void*)pub_buf;
                    message.payloadlen = 6; //MQTTclient.h takes care of adding null char?
                    /* Lock the global MQTT mutex before publishing */
                    mqttMtx.lock();
                    client->publish(topic1, message);
                    mqttMtx.unlock();

                    readBackSensor();
                    message.qos = MQTT::QOS0;
                    message.retained = false;
                    message.dup = false;
                    message.payload = (void*)pub_buf;
                    message.payloadlen = 6; //MQTTclient.h takes care of adding null char?
                    /* Lock the global MQTT mutex before publishing */
                    mqttMtx.lock();
                    client->publish(topic2, message);
                    mqttMtx.unlock();

                    movement('s', speed, 300);
                    // movement('s', speed, 100);
                    // movement('s', speed, 100);
                    // movement('s', speed, 100);

                    break;
                // Rotate CW
                case '5':
                    rotateCW(30);
                    readFrontSensor();
                    readBackSensor();

                    message.qos = MQTT::QOS0;
                    message.retained = false;
                    message.dup = false;
                    message.payload = (void*)pub_buf;
                    message.payloadlen = 6; //MQTTclient.h takes care of adding null char?
                    /* Lock the global MQTT mutex before publishing */
                    mqttMtx.lock();
                    client->publish(topic1, message);
                    mqttMtx.unlock();
                    break;
                // Rotate CCW
                case '6':
                    rotateCCW(30);
                    readFrontSensor();
                    readBackSensor();

                    message.qos = MQTT::QOS0;
                    message.retained = false;
                    message.dup = false;
                    message.payload = (void*)pub_buf;
                    message.payloadlen = 6; //MQTTclient.h takes care of adding null char?
                    /* Lock the global MQTT mutex before publishing */
                    mqttMtx.lock();
                    client->publish(topic1, message);
                    mqttMtx.unlock();

                    break;
                // Avoid to CW
                case '7':
                    angle = 1;
                    break;
                // Avoid to CCW
                case '8':
                    angle = 0;
                    break;
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


