/*
 * Copyright (c) 2010 by Cristian Maglie <c.maglie@bug.st>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>

#include "utility/ms1000_mif.h"

// MIF controller instance
MIFClass gMIFInfo;

#define SPI_SSpin 10

uint8_t MIFClass::cmdComplete = 0;
uint8_t MIFClass::cmd = 0;
uint8_t MIFClass::is_read = 0;
uint8_t MIFClass::bMifInit = 0;

void MIFClass::Write(uint8_t w)
{
//	setSS();
    digitalWrite(SPI_SSpin, LOW);
    SPI.transfer(w);
//	resetSS();
	digitalWrite(SPI_SSpin, HIGH);
	
}
uint8_t MIFClass::Read(void)
{
	uint8_t ret = 0;
	//setSS();  
	digitalWrite(SPI_SSpin, LOW);
	ret = SPI.transfer(0x0);
	digitalWrite(SPI_SSpin, HIGH);
	//resetSS();
	return ret;
}
int MIFClass::WaitCmd(uint8_t waitcmd)
{
	uint32_t retry = IF_RETRY_CNT; // 60 sec
	uint8_t  rep_cmd = 0;

	while(1) {
		rep_cmd = Read();
		if(rep_cmd == (waitcmd)) {
			if(waitcmd == IF_SENDRECV_END) {
				Write(IF_SENDRECV_END);
			}
			else if(waitcmd == IF_SENDRECV_READY /*&& gMIFInfo.is_read == false*/) {
				Write(IF_SENDRECV_READY);
			}
			break;
		}

		if(retry-- <= 0) {
			Serial.println("MIFClass::WaitCmd time over!! fail\n");
			return -1;
		}
		delay(1);
	}

	return 0;
}

int MIFClass::StartCmd(uint16_t cmd, uint8_t ctrlb, uint16_t datalen)
{
    // start byte
    Write(IF_START_VALUE);

	// read/write data length
	Write((datalen & 0xFF00) >> 8);
	Write((datalen & 0x00FF) >> 0);

	// bsb [4 : 0]
	// [00000] : common reg
	// [01000] : socket reg
	// [10000] : tx buf reg
	// [11000] : rx buf reg
	Write((cmd & 0xFF00) >> 8);
	Write((cmd & 0x00FF) >> 0);
	Write(ctrlb);

	return WaitCmd(IF_SENDRECV_READY);
}

int MIFClass::EndCmd(void)
{
    return WaitCmd(IF_SENDRECV_END);
}

void MIFClass::Init(void)
{
//  initSS();
	uint32_t data = 0xaffae99e;
	int i;
    bMifInit = 1;
    SPI.begin();
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV4);
    SPI.setDataMode(SPI_MODE3);
    digitalWrite(SPI_SSpin, HIGH);

	delay(100);
	for(i = 4; i > 0; i--)
		Write((uint8_t)((data>>(8*(i-1))) & 0xff));
	delay(100);

}

int MIFClass::WriteData(uint16_t addr, uint8_t ctrlb, const uint8_t* pWBuf, uint16_t len, bool IsPMEM)
{
	int i = 0;
	int ret = 0;

	is_read = false;

	if(pWBuf == NULL || len == 0)
		return -1;

	ret = StartCmd(addr, ctrlb, len);
	if(ret != 0) {
		return -1;
	}

	if(IsPMEM) {
		for(i = 0; i < len; i++)
			Write(pgm_read_byte(pWBuf+i));
	} else {
		for(i = 0; i < len; i++)
			Write(pWBuf[i]);
	}
	
	ret = EndCmd();
	if(ret != 0) {
		return -1;
	}

	return 0;
}


int MIFClass::ReadData(uint16_t addr, uint8_t ctrlb, uint8_t* pRBuf, uint16_t len)
{
	int i = 0;
	int ret = 0;

	is_read = true;

	if(pRBuf == NULL || len == 0)
		return -1;

	ret = StartCmd(addr, ctrlb, len);
	if(ret != 0) {
		return -1;
	}

	for(i = 0; i < len; i++)
	   pRBuf[i] = Read();

	ret = EndCmd();
	if(ret != 0) {
		return -1;
	}

	return 0;
}

bool MIFClass::IsReady(void)
{
	return (bMifInit) ? true : false;
}

