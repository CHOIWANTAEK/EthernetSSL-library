/*
  SSL Gmail Test
  
  This sketch uses SSL extension to the SSL libraries to get an SSL connection.
  using an W5500 Ethernet Shield S.
 */

#include <Ethernet.h>
#include <SSL.h>
#include "RootCA.h"

SSLClass SSLClient;

void initDate() {
  String sDate_Command = "";
  char cDateTemp;
  Serial.println("Please input the today's date: (ex:YYYYMMDD)");   
  sDate_Command = "";
  while( sDate_Command.length() < 8 )
  {
    while( Serial.available() )
    {
      cDateTemp = Serial.read();
      sDate_Command.concat( cDateTemp );
    }
  }
  Serial.println("Set Date : " + sDate_Command);

  int str_len = sDate_Command.length() + 2;
  char str_array[str_len];   
  uint32_t date = 0;
  sDate_Command.toCharArray( str_array , str_len-1 );
  SSLClient.SetDate( (uint8_t*)str_array, str_len );
}

void initTime() {
  String sTime_Command = "";
  char cTimeTemp;
  Serial.println("Please input the time: (ex:HHMMSS)");
  sTime_Command = "";
  while( sTime_Command.length() < 6 )
  {
    while( Serial.available() )
    {
      cTimeTemp = Serial.read();
      sTime_Command.concat( cTimeTemp );
    }
  }
  Serial.println("Set Time : " + sTime_Command);

  int str_len = sTime_Command.length() + 2;
  char str_array[str_len];   
  sTime_Command.toCharArray( str_array , str_len-1 );
  SSLClient.SetTime( (uint8_t*)str_array, str_len );
}

void initEthernet() {
  byte mac[] = { 0x00, 0x08, 0xdc, 0x4f, 0x0f, 0x33 };
  Serial.println(F("[Init ethernet.. ]"));
  Ethernet.begin(mac);
  Serial.print(F(" -LocalIp: "));
  Serial.println(Ethernet.localIP());
}

void sslOpen() {
  int ret = 0;

  Serial.println(F("[SSL Open]"));
  ret = SSLClient.Open();
  if(ret < 0)
  {
    Serial.println(F(" -Open Fail"));
  }
  else
  {
    Serial.println(F(" -Open Success"));
  }
  ret = SSLClient.SetRootCA(certificate, certificateLength, true);
  if(ret < 0)
  {
    Serial.println(F(" -SetRootCA fail"));
  }
  else
  {
    Serial.println(F(" -SetRootCA Success"));
  }

  const bool verifyPeer = true;
  if ( verifyPeer )
  {
  ret = SSLClient.SetPeerVerify(true);
    if(ret < 0)
    {
      Serial.println(" -SetPeerVerify (Verify Peer) fail");
    }
    else
    {
      Serial.println(" -SetPeerVerify (Verify Peer) success");
    }
  }
  else
  {  
    ret = SSLClient.SetPeerVerify(false);
    if(ret < 0)
    {
      Serial.println(" -SetPeerVerify (Verify None) fail");
    }
    else
    {
      Serial.println(" -SetPeerVerify (Verify None) success");
    }
  }
}

void sslConnect() {
  int ret = 0;

  Serial.println(F("[SSL Connecting...]"));
  ret = SSLClient.Connect(sslHostName, sslHostPort);
  if(ret < 0)
  {
    Serial.println(F(" -Connect Fail"));
  }
  else
  {
    Serial.println(F(" -Connect Success"));
  }
}

void sslGetInfo() {
  int ret = 0;
  uint8_t tmpbuf[128];
  uint16_t tmpSz;

  Serial.println(F("[Info]"));
  memset(tmpbuf, 0, sizeof(tmpbuf));
  ret = SSLClient.GetX509IssuerName(tmpbuf, sizeof(tmpbuf));
  if(ret == 0)
  {
    String tmp = (char*)tmpbuf;
    Serial.print(F("  Issuer: "));
    Serial.println(tmp);
  }

  memset(tmpbuf, 0, sizeof(tmpbuf));
  ret = SSLClient.GetX509SubjectName(tmpbuf, sizeof(tmpbuf));
  if(ret == 0)
  { 
    String tmp = (char*)tmpbuf;
    Serial.print(F("  Subject: "));
    Serial.println(tmp);
  }

  do
  {
    ret = SSLClient.GetX509NextAltName(tmpbuf, sizeof(tmpbuf));
    if(ret == 0)
    {
      String tmp = (char*)tmpbuf;
      if(strlen((char*)tmpbuf) == 0)
      {
        break;
      }
      Serial.print(F("  Altname: "));
      Serial.println(tmp);
    }
    else
    {
      break;
    }
  } while (ret != 0);

  ret = SSLClient.GetX509SerialNum(tmpbuf, 32, &tmpSz);
  if(ret == 0)
  {
    char serialMsg[80];
    int i;
    int strLen;
    strLen = sprintf(serialMsg, " Serial Num :");
    for (i = 0; i < tmpSz; i++)
    {
      sprintf(serialMsg + strLen + (i*3), ":%02x ", tmpbuf[i]);
    }

    Serial.println(serialMsg);
  }

  memset(tmpbuf, 0, sizeof(tmpbuf));
  ret = SSLClient.GetVersion(tmpbuf, sizeof(tmpbuf));
  if(ret == 0)
  {
    String tmp = (char*)tmpbuf;
    Serial.print(F("  Version: "));
    Serial.println(tmp);
  }

  memset(tmpbuf, 0, sizeof(tmpbuf));
  ret = SSLClient.GetCipherName(tmpbuf, sizeof(tmpbuf));
  if(ret == 0)
  {
    String tmp = (char*)tmpbuf;
    Serial.print(F("  CipherName: "));
    Serial.println(tmp);
  }
}

void sslReadWrite() {
  int ret = 0;
  uint8_t tmpbuf[128];
  uint16_t msgSz;
  uint16_t readSz;

  Serial.println(F("[SSL R/W]"));

  // SSL WRITE Data
  const char *httpMessage = "GET /index.html HTTP/1.1\r\n\r\n";
  msgSz = strlen(httpMessage);
  ret = SSLClient.WriteData( httpMessage, msgSz );
  if(ret == 0)
  {
    Serial.print(F(" -Write: "));
    Serial.println(httpMessage);

    // SSL READ Data
    memset(tmpbuf, 0, sizeof(tmpbuf));
    ret = SSLClient.ReadData(tmpbuf, sizeof(tmpbuf)-1, &readSz);
    if(ret == 0)
    {
      String tmp = (char*)tmpbuf;
      Serial.print(F(" -Read size: "));
      Serial.println(readSz, DEC);
      Serial.print(F(" -Read data: "));
      Serial.print(tmp);
    }
  }
  else
  {
    Serial.println(F("-Write Fail"));
  }
}

void sslClose() {
  int ret = 0;

  Serial.println(F("\n[SSL Close]"));
  ret = SSLClient.Close();
  if(ret < 0)
  {
    Serial.println(F(" -Close Fail"));
  }
  else
  {
    Serial.println(F(" -Close Success"));
  }
}

void setup() {

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while ( !Serial ) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Initialize the ethernet device:
  initEthernet();
  
  // Initialize date and time:
  initDate();
  initTime();
}

void loop() {
  delay(1000);
  Serial.print(F("==SSL Test Start==\n"));
  sslOpen();
  sslConnect();
  sslGetInfo();
  sslReadWrite();
  sslClose();
  Serial.print(F("SSL Test DONE!\n\n"));
  delay(1000);
}

