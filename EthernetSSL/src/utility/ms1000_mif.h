
#ifndef	MIF_H_INCLUDED
#define	MIF_H_INCLUDED

#include <SPI.h>

#define IF_RETRY_CNT       60000 // 60sec
#define IF_SENDRECV_READY  0xF1
#define IF_SENDRECV_END    0xF4
#define IF_START_VALUE     0xfe

/* INTERFACE SSL COMMADND */
#define SSL_SPI_CMD          (0x01 << 0)
#define SSL_SPI_READ	     ((0x00 << 2) | SSL_SPI_CMD)
#define SSL_SPI_WRITE		 ((0x01 << 2) | SSL_SPI_CMD)//

#define ENABLE_SSL_DHCP
#ifdef ENABLE_SSL_DHCP
#define DHCPINIT_CMD         (0x11)
#define DHCPALLOC_CMD        (0x12)
#define DHCPGETINFO_CMD      (0x13)
#define DHCPDEINIT_CMD       (0x14)

#define DNSRUN_CMD           (0x15)
#define GETHOSTBYNAME_CMD    (0x16)
#define DNSSERVER_SET_CMD    (0x17)
#endif

/* SSL API Command set */
#define SSL_OPEN_CMD             (0x20)
#define SSL_CLOSE_CMD            (0x21)
#define SSL_CONNECT_CMD          (0x22)
#define SSL_IS_CONNECT_CMD       (0x23)

#define SSL_WRITE_CMD            (0x30)
#define SSL_READ_CMD             (0x31)
#define SSL_READ_LEN_CMD         (0x32)

#define SSL_SET_VERIFY_CMD       (0x40)
#define SSL_SET_ROOTCA_CMD       (0x41)
#define SSL_IS_LOADVERIFY_CMD    (0x42)

#define SSL_GET_VERSION_CMD      (0x50)
#define SSL_GET_CIPHERNAME_CMD   (0x51)
#define SSL_GET_ISSUERNAME_CMD   (0x52)
#define SSL_GET_SUBJECTNAME_CMD  (0x53)
#define SSL_GET_NEXTALTNAME_CMD  (0x54)
#define SSL_GET_SERIALNUM_CMD    (0x55)
#define SSL_GET_SERIALNUM_SZ_CMD (0x56)
#define SSL_SET_DATE_CMD         (0x57)
#define SSL_SET_TIME_CMD         (0x58)

#define FAIL_SLAVEDATA	0
#define SUCCESS_SLAVEDATA	1

class MIFClass {

public:
  void Init(void);
  bool IsReady(void);
  int WriteData(uint16_t addr, uint8_t ctrlb, const uint8_t* pWBuf, uint16_t len, bool IsPMEM=false);
  int ReadData(uint16_t addr, uint8_t ctrlb, uint8_t* pRBuf, uint16_t len);

private:
  static inline void Write(uint8_t w);
  static inline uint8_t Read(void);
  static inline int WaitCmd(uint8_t waitcmd);
  static inline int StartCmd(uint16_t cmd, uint8_t ctrlb, uint16_t datalen);
  static inline int EndCmd(void);

private:
    static uint8_t cmdComplete;
    static uint8_t cmd;
    static uint8_t is_read;
    static uint8_t bMifInit;

//public:
//  static const uint16_t SSIZE = 2048; // Max Tx buffer size
};

extern MIFClass gMIFInfo;

#endif
