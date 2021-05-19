/*
 * gprs.h
 * A library for SeeedStudio seeeduino GPRS shield
 *
 * Copyright (c) 2013 seeed technology inc.
 * Author        :   lawliet zou
 * Create Time   :   Dec 2013
 * Change Log    :
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef __GPRS_H__
#define __GPRS_H__

#include "sim800.h"

enum Protocol {
    CLOSED = 0,
    TCP    = 1,
    UDP    = 2,
};

/** GPRS class.
 *  used to realize GPRS communication
 */
#define MESSAGE_LENGTH  100

    /** Create GPRS instance
     *  @param number default phone number during mobile communication
     */
    void GPRS_init(sim800_cfg_t* cfg);

    /** initialize GPRS module including SIM card check & signal strength
     *  @returns
     *      0 on success
     *      -1 on error
     */
    int GPRS_initialize(void);

    /** Join APN
     *  @return
     *        true Jion APN successfully
     *        false failed to join VPN
     */
    bool GPRS_join(const char  *apn, const char *userName, const char *passWord);


    /** parse IP string
     *  @return
     *      ip in hex
     */
    uint32_t GPRS_str_to_ip(const char* str);

    /** get Ip address
     *  @return
     *       IP string
     */
    char* GPRS_getIPAddress();

    /** check SIM card' Status
     *  @returns
     *      0 on success
     *      -1 on error
     */
    int GPRS_checkSIMStatus(void);

    /** check network is OK or not
     *  @returns
     *      0 on success
     *      -1 on error
     */
    int GPRS_networkCheck(void);

    /** send text SMS
     *  @param  *number phone number which SMS will be send to
     *  @param  *data   message that will be send to
     *  @returns
     *      0 on success
     *      -1 on error
     */
    int GPRS_sendSMS(char* number, char* data);

    /** read SMS if getting a SMS message
     *  @param  buffer  buffer that get from GPRS module(when getting a SMS, GPRS module will return a buffer array)
     *  @param  message buffer used to get SMS message
     *  @param  check   whether to check phone number(we may only want to read SMS from specified phone number)
     *  @returns
     *      0 on success
     *      -1 on error
     */
    int GPRS_readSMS(int messageIndex, char *message, int length);

    /** delete SMS message on SIM card
     *  @param  index   the index number which SMS message will be delete
     *  @returns
     *      0 on success
     *      -1 on error
     */
    int GPRS_deleteSMS(int index);

    /** call someone
     *  @param  number  the phone number which you want to call
     *  @returns
     *      0 on success
     *      -1 on error
     */
    int GPRS_callUp(char* number);

    /** auto answer if coming a call
     *  @returns
     *      0 on success
     *      -1 on error
     */
    int GPRS_answer(void);
    // Http Initialize , Send data via Http
    int GPRS_InitHttp(const char* apn);
    int GPRS_sendHttpData(const char* apn, uint8_t* data, int plen);

    /** build TCP connect
     *  @param  ip  ip address which will connect to
     *  @param  port    TCP server' port number
     *  @returns
     *      0 on success
     *      -1 on error
     */
    int GPRS_connectTCP(const char* ip, int port);

    /** send data to TCP server
     *  @param  data    data that will be send to TCP server
     *  @returns
     *      0 on success
     *      -1 on error
     */
    int GPRS_sendTCPData(char* data);

    /** close TCP connection
     *  @returns
     *      0 on success
     *      -1 on error
     */
    int GPRS_closeTCP(void);

    /** close TCP service
     *  @returns
     *      0 on success
     *      -1 on error
     */
    int GPRS_shutTCP(void);

#endif
