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

#include "utility/w5100.h"
#include "utility/ms1000_mif.h"

#include "Ethernet.h"
#include "Dns.h"
#include "SSL.h"

SSL_Status SSLClass::state = SSL_IDLE_STATE;
bool SSLClass::peerVerify = false;

#define CHECK_SEND_RECV(b, l, s) \
								if(b == NULL || l == 0 || l >= SSL_SLAVE_MAX_BUF) {\
									SSLDEBUG(" !SSL_INVALID_PARAM_ERR!"); \
									return SSL_INVALID_PARAM_ERR; }\
								if(s != SSL_AVAIL_SENDRECV_STATE) {\
									SSLDEBUG(" !SSL_STATE_ERR!"); \
									return SSL_STATE_ERR; }


#if 0
int SSLClass::begin(uint8_t *mac_address)
{
	uint8_t dhcp_sn = 5;
	int retry = 5;
    uint8_t data = 0;
	int ret;
	int i;
	uint8_t dhcp_data[20]; // _dhcpLocalIp, _dhcpSubnetMask, _dhcpGatewayIp, _dhcpDhcpServerIp, _dhcpDnsServerIp
	SSLDEBUG("SSLClass::begin");

#if 0 // for test dhcp performace
	W5100.init();
	W5100.setMACAddress(mac_address);

	for (i = dhcp_sn; i < MAX_SOCK_NUM; i++)
	{
		data = W5100.readSnSR(i);
		if (data == SnSR::CLOSED || data == SnSR::FIN_WAIT) {
		  dhcp_sn = i;
		  break;
		}
	}

	if(i >= MAX_SOCK_NUM) {
		SSLDEBUG("SSLClass::begin socket fail!");
		return SSL_FAIL;
	}
	
	Serial.print("sn : ");
	Serial.println(dhcp_sn, DEC);

	//dhcp init
	ret = gMIFInfo.WriteData(API_DHCPINIT, (SSL_SPI_CMD|SSL_SPI_WRITE), &dhcp_sn, 1);
	if(ret != 0) {
		SSLDEBUG("SSLClass::begin API_DHCPINIT fail!");
		return SSL_FAIL;
	}

	while(retry) {
		ret = gMIFInfo.ReadData(API_DHCPALLOC, (SSL_SPI_CMD|SSL_SPI_READ), &data, 1);
		if(ret != 0 || data != SUCCESS_SLAVEDATA) {
			retry--;
		} else
			break;
	}

	if(retry > 0) {// dhcp allocated
		if(gMIFInfo.ReadData(API_DHCPGETINFO, (SSL_SPI_CMD|SSL_SPI_READ), dhcp_data, 20) != 0) {
			return SSL_FAIL;
		} else {
			Serial.print("localIP:");
			Serial.println(Ethernet.localIP());
			Serial.print("_dhcpSubnetMask:");
			Serial.println(Ethernet.subnetMask());
			Serial.print("_dhcpGatewayIp:");
			Serial.println(Ethernet.gatewayIP());
			Serial.print("dnsServerIP:");
			Serial.println(Ethernet.dnsServerIP());
		}
	} else {
		SSLDEBUG("SSLClass::begin API_DHCPALLOC fail!");
		return SSL_FAIL;
	}
#endif
	return SSL_SUCCESS;
}
#endif

int SSLClass::Open(void)
{
    uint8_t data = 0;
	int ret;

	SSLDEBUG("SSLClass::Open");

	// check mif ready
	if(!gMIFInfo.IsReady() || (state!=SSL_IDLE_STATE)) {
		return SSL_STATE_ERR;
	}

	ret = gMIFInfo.ReadData(SSL_OPEN_CMD, SSL_SPI_READ,&data, 1);
	if(ret != 0 || data != SUCCESS_SLAVEDATA) {
		return SSL_INIT_ERR;
	}

	state = SSL_OPENED_STATE;

	return SSL_SUCCESS;
}

int SSLClass::Close(void)
{
	SSL_Status oldstate;
	int ret;
	uint8_t data = 0;

	SSLDEBUG("SSLClass::Close");

	oldstate = state;

	if((state == SSL_IDLE_STATE) || (state == SSL_CONNECTING_STATE)) {
		ret = SSL_STATE_ERR;
		goto SSLCloseError;
	}

#if 0
	ret = gMIFInfo.ReadData(SSL_CLOSE_CMD, SSL_SPI_READ, &data, 1);
	if(ret != 0 || data != SUCCESS_SLAVEDATA) {
		ret = SSL_CLOSE_ERR;
		goto SSLCloseError;
	}
#else
	if(gMIFInfo.WriteData(SSL_CLOSE_CMD, SSL_SPI_WRITE, &data, 1) != 0) {
		ret = SSL_CLOSE_ERR;
		goto SSLCloseError;
	}
#endif

	state = SSL_IDLE_STATE;

	return SSL_SUCCESS;

SSLCloseError:
	SSLDEBUG(" !SSLClass::Close Error!");
	state = oldstate;
	return ret;
}

int SSLClass::Connect(IPAddress ip, uint16_t port)
{
	DNSClient dns;
	IPAddress remote_addr;
	SSL_Status oldstate;
	uint8_t data = 0;
	uint8_t senddata[6] = {0,};
	int ret = 0;
	
	SSLDEBUG("SSLClass::Connect(ip)");
	oldstate = state;

	if(state != SSL_OPENED_STATE) {
		ret = SSL_STATE_ERR;
		goto SSLConnectError;
	}
	
	state = SSL_CONNECTING_STATE;

	for(int i=0; i<4; i++) {
		senddata[i] = ip[i];
	}
	senddata[4] = (0xff & (port >> 8));
	senddata[5] = (0xff & port);

	if(gMIFInfo.WriteData(SSL_CONNECT_CMD, SSL_SPI_WRITE, senddata, 6) != 0) {
		ret = SSL_CONNECT_ERR;
		goto SSLConnectError;
	}

	ret = gMIFInfo.ReadData(SSL_IS_CONNECT_CMD, SSL_SPI_READ, &data, 1);
	if(ret != 0 || data != SUCCESS_SLAVEDATA) {
		ret = SSL_CONNECT_ERR;
		goto SSLConnectError;
	}

	state = SSL_AVAIL_SENDRECV_STATE;

	return SSL_SUCCESS;

SSLConnectError:
	SSLDEBUG(" !SSLClass::Connect Error!");
	state = oldstate;
	return ret;
}

int SSLClass::Connect(const char *host, uint16_t port)
{
	DNSClient dns;
	IPAddress remote_addr;
	int ret = 0;

	SSLDEBUG("SSLClass::Connect(host)");

	if(state != SSL_OPENED_STATE)
		return SSL_STATE_ERR;

	dns.begin(Ethernet.dnsServerIP());
	ret = dns.getHostByName(host, remote_addr);
	if(ret != 1) {
		SSLDEBUG(" !chagne domain -> ip fail!");
		return SSL_CONNECT_ERR;
	}

#ifdef ENABLE_SSL_DEBUG
	Serial.print("  connect host->ip : ");
	Serial.println(remote_addr);
#endif

	return Connect(remote_addr, port);
}

int SSLClass::WriteData(uint8_t* buf, uint16_t len, bool IsPMEM)
{
	CHECK_SEND_RECV(buf, len, state);

	SSLDEBUG("SSLClass::WriteData");

	if(gMIFInfo.WriteData(SSL_WRITE_CMD, SSL_SPI_WRITE, buf, len, IsPMEM) != 0) {
		return SSL_FAIL;
	}

	return SSL_SUCCESS;
}
int SSLClass::ReadData(uint8_t* buf, uint16_t len, uint16_t* OutReadSz)
{
	uint8_t tmpbuf[2] = {0,};

	CHECK_SEND_RECV(buf, len, state);

	SSLDEBUG("SSLClass::ReadData");

	*OutReadSz = 0;
	if(gMIFInfo.ReadData(SSL_READ_CMD, SSL_SPI_READ, buf, len) != 0) {
		return SSL_FAIL;
	}

	if(gMIFInfo.ReadData(SSL_READ_LEN_CMD, SSL_SPI_READ, tmpbuf, 2) != 0) {
		return SSL_FAIL;
	}

	*OutReadSz = (tmpbuf[0] << 8 | tmpbuf[1]);

	return SSL_SUCCESS;
}

int SSLClass::SetPeerVerify(bool verify)
{
	uint8_t data = 0;

	if(state != SSL_OPENED_STATE)
		return SSL_STATE_ERR;

	SSLDEBUG("SSLClass::SetPeerVerify");

	data = verify;
	if(gMIFInfo.WriteData(SSL_SET_VERIFY_CMD, SSL_SPI_WRITE, &data, 1) != 0) {
		return SSL_FAIL;
	}

	return SSL_SUCCESS;
}

int SSLClass::SetRootCA(const unsigned char *buf, uint16_t len, bool IsPMEM)
{
	uint8_t data = 0;
	int ret = 0;

	if(state != SSL_OPENED_STATE)
		return SSL_STATE_ERR;

	if(buf == NULL || len == 0)
		return SSL_FAIL;

	SSLDEBUG("SSLClass::SetRootCA");

	if(gMIFInfo.WriteData(SSL_SET_ROOTCA_CMD, SSL_SPI_WRITE, buf, len, IsPMEM) != 0) {
		return SSL_FAIL;
	}

	ret = gMIFInfo.ReadData(SSL_IS_LOADVERIFY_CMD, SSL_SPI_READ, &data, 1);
	if(ret != 0 || data != SUCCESS_SLAVEDATA) {
		return SSL_FAIL;
	}

	return SSL_SUCCESS;
}

int SSLClass::GetVersion(uint8_t* buf, uint16_t len)
{
	CHECK_SEND_RECV(buf, len, state);
	SSLDEBUG("SSLClass::GetVersion");

	if(gMIFInfo.ReadData(SSL_GET_VERSION_CMD, SSL_SPI_READ, buf, len) != 0) {
		return SSL_FAIL;
	}
	return SSL_SUCCESS;
}
int SSLClass::GetCipherName(uint8_t* buf, uint16_t len)
{
	CHECK_SEND_RECV(buf, len, state);
	SSLDEBUG("SSLClass::GetCipherName");

	if(gMIFInfo.ReadData(SSL_GET_CIPHERNAME_CMD, SSL_SPI_READ, buf, len) != 0) {
		return SSL_FAIL;
	}
	return SSL_SUCCESS;
}

int SSLClass::GetX509IssuerName(uint8_t* buf, uint16_t len)
{
	CHECK_SEND_RECV(buf, len, state);

	SSLDEBUG("SSLClass::GetIssuerName");

	if(gMIFInfo.ReadData(SSL_GET_ISSUERNAME_CMD, SSL_SPI_READ, buf, len) != 0) {
		return SSL_FAIL;
	}

	return SSL_SUCCESS;
}
int SSLClass::GetX509SubjectName(uint8_t* buf, uint16_t len)
{
	CHECK_SEND_RECV(buf, len, state);

	SSLDEBUG("SSLClass::GetSubjectName");

	if(gMIFInfo.ReadData(SSL_GET_SUBJECTNAME_CMD, SSL_SPI_READ, buf, len) != 0) {
		return SSL_FAIL;
	}

	return SSL_SUCCESS;
}
int SSLClass::GetX509NextAltName(uint8_t* buf, uint16_t len)
{
	CHECK_SEND_RECV(buf, len, state);

	SSLDEBUG("SSLClass::GetSubjectName");

	if(gMIFInfo.ReadData(SSL_GET_NEXTALTNAME_CMD, SSL_SPI_READ, buf, len) != 0) {
		return SSL_FAIL;
	}

	return SSL_SUCCESS;
}
int SSLClass::GetX509SerialNum(uint8_t* buf, uint16_t len, uint16_t* OutNumSz)
{
	uint8_t tmpbuf[2] = {0,};

	CHECK_SEND_RECV(buf, len, state);

	SSLDEBUG("SSLClass::GetSubjectName");

	*OutNumSz = 0;
	if(gMIFInfo.ReadData(SSL_GET_SERIALNUM_CMD, SSL_SPI_READ, buf, len) != 0) {
		return SSL_FAIL;
	}

	if(gMIFInfo.ReadData(SSL_GET_SERIALNUM_SZ_CMD, SSL_SPI_READ, tmpbuf, 2) != 0) {
		return SSL_FAIL;
	}

	*OutNumSz = (tmpbuf[0] << 8 | tmpbuf[1]);

	return SSL_SUCCESS;
}

int SSLClass::SetDate(uint8_t* buf, uint16_t len)
{
	if(!gMIFInfo.IsReady() || (state!=SSL_IDLE_STATE)) {
		return SSL_STATE_ERR;
	}
    
	SSLDEBUG("SSLClass::SetDate");

	if(gMIFInfo.WriteData(SSL_SET_DATE_CMD, SSL_SPI_WRITE, buf, len, 0) != 0) {
		return SSL_FAIL;
	}

	return SSL_SUCCESS;
}

int SSLClass::SetTime(uint8_t* buf, uint16_t len)
{
	if(!gMIFInfo.IsReady() || (state!=SSL_IDLE_STATE)) {
		return SSL_STATE_ERR;
	}
    
	SSLDEBUG("SSLClass::SetTime");

	if(gMIFInfo.WriteData(SSL_SET_TIME_CMD, SSL_SPI_WRITE, buf, len, 0) != 0) {
		return SSL_FAIL;
	}

	return SSL_SUCCESS;
}


