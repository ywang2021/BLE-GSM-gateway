/*
 * gprs.cpp
 * A library for SeeedStudio seeeduino GPRS shield
 *
 * Copyright (c) 2013 seeed technology inc.
 * Author        :   lawliet zou
 * Create Time   :   Dec 2013
 * Change Log    :   2017-05-31 GPRS_readSMS modified to support SMS international
                     increment the buffer to read the whole message 
                     Jose Zorrilla (jzorrilla@x-red.com)
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

#include <stdio.h>
#include <stdlib.h>
#include "gprs.h"
#include "nrf_delay.h"

#define BOUNDRY     "----------WebKitFormBoundaryvZ0ZHShNAcBABWFy"

uint32_t m_ip;
char ip_string[20];

void GPRS_init(sim800_cfg_t* cfg)
{
  SIM800_Init(cfg);
}

int GPRS_initialize(void)
{
    int i = 0;
#if 0
    for(i = 0; i < 2; i++){
        SIM800_sendCmd("AT\r\n");
        nrf_delay_ms(100);
    }
    SIM800_sendCmd("AT+CFUN=1\r\n");
    if(0 != checkSIMStatus()) {
        ERROR("ERROR:checkSIMStatus");
        return -1;
    }
    return 0;

#endif
    if(0 != SIM800_sendCmdAndWaitForResp("AT\r\n","OK\r\n",DEFAULT_TIMEOUT*3)){
      return -1;
    }
    //if (!GSM_sendCommand("ATE0")) return -1;
    //if (!GSM_sendCommand("AT+GSN")) return -1;
    if(0 != SIM800_sendCmdAndWaitForResp("AT+CREG?\r\n","0,X\r\n",DEFAULT_TIMEOUT*3)){
      return -1;
    }

    if(0 != SIM800_sendCmdAndWaitForResp("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n",0,1000)){
      return -1;
    }

    if(0 != SIM800_sendCmdAndWaitForResp("AT+SAPBR=3,1,\"APN\",\"CMNET\"\r\n",0,1000)){
      return -1;
    }

    if(0 != SIM800_sendCmdAndWaitForResp("AT+SAPBR=2,1\r\n","1,1",1000)){
        SIM800_sendCmdAndWaitForResp("AT+SAPBR=1,1\r\n","1,1",DEFAULT_TIMEOUT*2);
        return -1;
    }

    if(GPRS_checkSIMStatus()) {
		  return -1;
    }
    return 0;

}

bool GPRS_join(const char *apn, const char *userName, const char *passWord)
{
    char cmd[64];
    char ipAddr[32];
    char gprsBuffer[32];
   
    //Select multiple connection
    //sim900_check_with_cmd("AT+CIPMUX=1\r\n","OK",DEFAULT_TIMEOUT,CMD);
      
    SIM800_cleanBuffer(ipAddr,32);
    SIM800_sendCmd("AT+CIFSR\r\n");    
    SIM800_readBuffer(ipAddr,32,DEFAULT_TIMEOUT*2);
    
    // If no IP address feedback than bring up wireless 
    if( NULL != strstr(ipAddr, "ERROR") )
    {
        GPRS_networkCheck();
        if(SIM800_sendCmdAndWaitForResp("AT+CREG?\r\n",0,DEFAULT_TIMEOUT*3)){
          return -1;
        }
        if( 0 != SIM800_sendCmdAndWaitForResp("AT+CSTT?\r\n", apn, DEFAULT_TIMEOUT*3) )
        {
            SIM800_sendCmd("AT+CSTT=\"");
            SIM800_sendCmd(apn);
            if(userName>0)
            {
                SIM800_sendCmd("\",\"");
                SIM800_sendCmd(userName);
                SIM800_sendCmd("\",\"");
                SIM800_sendCmd(passWord);        
            }
            SIM800_sendCmdAndWaitForResp("\"\r\n","OK\r\n",DEFAULT_TIMEOUT*3);
        }
        if (SIM800_sendCmdAndWaitForResp("AT+SAPBR=3,1,\"Contype\",\"GPRS\"", 0, 2000)) {
                return -1;
		}
    
        SIM800_sendCmd("AT+SAPBR=3,1,\"APN\",\"");
        SIM800_sendCmd(apn);
        if(SIM800_sendCmdAndWaitForResp("\"", 0,  2000)) {
            return -1;
        }
        if (SIM800_sendCmdAndWaitForResp("AT+SAPBR=3,1", "+SAPBR: 1,", 2000)) {
        }
        if (SIM800_sendCmdAndWaitForResp("AT+SAPBR=2,1", "+SAPBR: 1,", 2000)) {
            if (SIM800_sendCmdAndWaitForResp("AT+SAPBR=1,1", "+SAPBR: 1,", 2000)) {
                return -1;
            }
		}
        //Brings up wireless connection
        SIM800_sendCmd("AT+CIICR\r\n");
        SIM800_readBuffer(gprsBuffer,6,DEFAULT_TIMEOUT*2);
        //Get local IP address
        SIM800_cleanBuffer(ipAddr,32);
        SIM800_sendCmd("AT+CIFSR\r\n");
        SIM800_readBuffer(ipAddr,32,DEFAULT_TIMEOUT*3);        
    }          
#if 0    
    Serial.print("ipAddr: ");
    Serial.println(ipAddr);
#endif

    if(NULL != strstr(ipAddr,"AT+CIFSR")) {        
        m_ip = GPRS_str_to_ip(ipAddr+11);
        if(m_ip != 0) {
            //NRF_LOG_INFO("IP address: %s", m_ip);
            return true;
        }
    }
    return false;
}

uint32_t GPRS_str_to_ip(const char* str)
{
    uint32_t ip = 0;
    char *p = (char*)str;
    
    for(int i = 0; i < 4; i++) {
        ip |= atoi(p);
        p = strchr(p, '.');
        if (p == NULL) {
            break;
        }
        if(i < 3) ip <<= 8;
        p++;
    }
    return ip;
}

//HACERR lo de la IP gasta muuuucho espacio (ver .h y todo esto)
char* GPRS_getIPAddress()
{
    uint8_t a = (m_ip>>24)&0xff;
    uint8_t b = (m_ip>>16)&0xff;
    uint8_t c = (m_ip>>8)&0xff;
    uint8_t d = m_ip&0xff;

    snprintf(ip_string, sizeof(ip_string), "%d.%d.%d.%d", a,b,c,d);
    return ip_string;
}

int GPRS_checkSIMStatus(void)
{
    char gprsBuffer[32];
    int count = 0;
    SIM800_cleanBuffer(gprsBuffer,32);
    while(count < 3) {
        SIM800_sendCmd("AT+CPIN?\r\n");
        SIM800_readBuffer(gprsBuffer,32,DEFAULT_TIMEOUT*3);
        if((NULL != strstr(gprsBuffer,"+CPIN: READY"))) {
            break;
        }
        count++;
        nrf_delay_ms(300);
    }
    if(count == 3) {
        return -1;
    }
    return 0;
}

int GPRS_networkCheck(void)
{
    nrf_delay_ms(1000);
    if(0 != SIM800_sendCmdAndWaitForResp("AT+CGREG?\r\n",0/*"+CGREG: 0,1"*/,DEFAULT_TIMEOUT*3)) {
        ERROR("ERROR:CGREG");
        return -1;
    }
    nrf_delay_ms(1000);
//    if(0 != SIM800_sendCmdAndWaitForResp("AT+CGATT?\r\n", "+CGATT: 1",DEFAULT_TIMEOUT*3)) {
    if(0 != SIM800_sendCmdAndWaitForResp("AT+CGATT?\r\n", 0,DEFAULT_TIMEOUT*3)) {
        ERROR("ERROR:CGATT");
        return -1;
    }
    return 0;
}

int GPRS_sendSMS(char *number, char *data)
{
    char cmd[32];
    if(0 != SIM800_sendCmdAndWaitForResp("AT+CMGF=1\r\n", "OK", DEFAULT_TIMEOUT)) { // Set message mode to ASCII
        ERROR("ERROR:CMGF");
        return -1;
    }
    nrf_delay_ms(500);
    snprintf(cmd, sizeof(cmd),"AT+CMGS=\"%s\"\r\n", number);
    if(0 != SIM800_sendCmdAndWaitForResp(cmd,">",DEFAULT_TIMEOUT)) {
        ERROR("ERROR:CMGS");
        return -1;
    }
    nrf_delay_ms(1000);
    SIM800_sendCmd(data);
    nrf_delay_ms(500);
    SIM800_sendEndMark();
    return 0;
}

int GPRS_readSMS(int messageIndex, char *message, int length)
{
    int i = 0;
    char gprsBuffer[144]; // Buffer size for the SMS message
    char cmd[16];
    char *p,*s;

   SIM800_sendCmdAndWaitForResp("AT+CMGF=1\r\n","OK",DEFAULT_TIMEOUT);
    nrf_delay_ms(1000);
    sprintf(cmd,"AT+CMGR=%d\r\n",messageIndex);
    SIM800_sendCmd(cmd);
    SIM800_cleanBuffer(gprsBuffer,144);
    SIM800_readBuffer(gprsBuffer,144,DEFAULT_TIMEOUT*5);

    if(NULL != ( s = strstr(gprsBuffer,"+CMGR"))){
        if(NULL != ( s = strstr(gprsBuffer,"REC"))){  // Search the beginning of the SMS message
            p = s - 1;
            while((i < length-1)) {
                message[i++] = *(p++);
            }
            message[i] = '\0';
        }
    }
    return 0;
}

int GPRS_deleteSMS(int index)
{
    char cmd[16];
    snprintf(cmd,sizeof(cmd),"AT+CMGD=%d\r\n",index);
    SIM800_sendCmd(cmd);
    return 0;
}

int GPRS_callUp(char *number)
{
    char cmd[24];
    if(0 != SIM800_sendCmdAndWaitForResp("AT+COLP=1\r\n","OK",5)) {
        ERROR("COLP");
        return -1;
    }
    nrf_delay_ms(1000);
    sprintf(cmd,"\r\nATD%s;\r\n", number);
    SIM800_sendCmd(cmd);
    return 0;
}

int GPRS_answer(void)
{
    SIM800_sendCmd("ATA\r\n");
    return 0;
}


int GPRS_InitHttp(const char *apn)
{
    char cipstart[50];
    SIM800_sendCmdAndWaitForResp("AT+HTTPTERM\r\n",0, 1000);
    if(0 != SIM800_sendCmdAndWaitForResp("AT+HTTPINIT\r\n", 0, 1000)) {
        
         //SIM800_sendCmdAndWaitForResp("AT+HTTPTERM\r\n",0, 1000);

        //return -1;
    }

    if(0 != SIM800_sendCmdAndWaitForResp("AT+HTTPPARA=\"CID\",1\r\n", 0, 1000)) {
        return -1;
    }
 
    

    if(0 != GPRS_sendHttpData(apn, "Connected!", 0)) {
        return -1;
    }
    return 0;
}

int GPRS_sendHttpData(const char *apn, uint8_t *data, int plen)
{
    int  i; 
    char cmd[128]={0}, tmp[3]={0};
    
    if(plen == 0)
    {
        sprintf(cmd, "?data=%s", data);
    }
    else
    {
        sprintf(cmd, "?data=");
        for(i=0;i<plen;i++)
        {
            sprintf(tmp, "%02x", data[i]);
            sprintf(cmd, "%s%s", cmd, tmp);
        }
    }
    SIM800_sendCmd("AT+HTTPPARA=\"URL\",\"");
    SIM800_sendCmd(apn);
    SIM800_sendCmd(cmd);
    if(0 != SIM800_sendCmdAndWaitForResp("\"\r\n", "OK", 3000)) {
        return -1;
    }

    if(0 != SIM800_sendCmdAndWaitForResp("AT+HTTPACTION=0\r\n",",2XX,2", 3000))
    {
        return -1;
    }

    SIM800_sendCmdAndWaitForResp("AT+HTTPREAD\r\n","+HTTPREAD: 2", 2000);
    nrf_delay_ms(1000);
    return 0;
}

int GPRS_sendHttpDataPost(const char *apn, uint8_t *data, int plen)
{
    int  i; 
    char cmd[128]={0}, tmp[3]={0};
    if(0 != SIM800_sendCmdAndWaitForResp("AT+HTTPINIT\r\n", 0, 1000)) {
        
//        SIM800_sendCmd("AT+HTTPTERM\r\n");
//        nrf_delay_ms(1000);
//        return -1;
    }
    if(plen == 0)
    {
        sprintf(cmd, "?data=%s", data);
    }
    else
    {
        sprintf(cmd, "?data=");
        for(i=0;i<plen;i++)
        {
            sprintf(tmp, "%02x", data[i]);
            sprintf(cmd, "%s%s", cmd, tmp);
        }
    }
    SIM800_sendCmd("AT+HTTPPARA=\"URL\",\"");
    SIM800_sendCmd(apn);
    if(0 != SIM800_sendCmdAndWaitForResp("\"\r\n", "OK", 3000)) {
        return -1;
    }

    SIM800_sendCmd("AT+HTTPPARA=\"CONTENT\",\"multipart/form-data; boundary=");
    SIM800_sendCmd(BOUNDRY);
    if(SIM800_sendCmdAndWaitForResp("\"\r\n", 0, 2000)!=0)
    {
        return -1;
    }
    SIM800_sendCmdAndWaitForResp("AT+HTTPDATA=192,10000\r\n",0, 1000);
    
    SIM800_sendCmd(BOUNDRY);
    SIM800_sendCmd("\r\nContent-Disposition: form-data; name=\"fileToUpload\"; filename=\"data.txt\"\r\n");
    SIM800_sendCmd("Content-Type: text/plain\r\n");
    SIM800_sendCmd("markoskype\r\n");
    SIM800_sendCmd(BOUNDRY);
    if(0 != SIM800_sendCmdAndWaitForResp("\r\n",0, 1000))
    {
        return -1;
    }
    SIM800_sendCmdAndWaitForResp("AT+HTTPACTION=1\r\n",0, 1000);
    SIM800_sendCmdAndWaitForResp("AT+HTTPREAD\r\n",0, 1000);
    nrf_delay_ms(1000);
    return 0;
}

int GPRS_connectTCP(const char *ip, int port)
{
    char cipstart[50];
    
    sprintf(cipstart, "AT+CIPSTART=\"TCP\",\"%s\",\"%d\"\r\n", ip, port);
    sprintf(cipstart, "AT+CIPSTART=\"TCP\",\"194.204.15.165\",\"%d\"\r\n", port);
    if(0 != SIM800_sendCmdAndWaitForResp(cipstart, "CONNECT OK", 3000)) {// connect tcp
        ERROR("ERROR:CIPSTART");
        return -1;
    }

    return 0;
}
int GPRS_sendTCPData(char *data)
{
    char cmd[32];
    int len = strlen(data); 
    snprintf(cmd,sizeof(cmd),"AT+ok=%d\r\n",len);
    if(0 != SIM800_sendCmdAndWaitForResp(cmd,">",2*DEFAULT_TIMEOUT)) {
        ERROR("ERROR:CIPSEND");
        return -1;
    }
        
    if(0 != SIM800_sendCmdAndWaitForResp(data,"SEND OK",2*DEFAULT_TIMEOUT)) {
        ERROR("ERROR:SendTCPData");
        return -1;
    }     
    return 0;
}

int GPRS_closeTCP(void)
{
    SIM800_sendCmd("AT+CIPCLOSE\r\n");
    return 0;
}

int GPRS_shutTCP(void)
{
    SIM800_sendCmd("AT+CIPSHUT\r\n");
    return 0;
}