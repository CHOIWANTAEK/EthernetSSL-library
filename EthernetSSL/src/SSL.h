
#ifndef	SSL_H_INCLUDED
#define	SSL_H_INCLUDED

//#include <avr/pgmspace.h>
#include <SPI.h>
#include <IPAddress.h>

typedef enum {
    SSL_SUCCESS = 0,
    SSL_STATE_ERR = -1,
    SSL_INIT_ERR = -2,
    SSL_CONNECT_ERR = -3,
    SSL_CLOSE_ERR = -4,
    SSL_INVALID_PARAM_ERR = -5,
    SSL_FAIL = -6
}SSL_ErrorType;

typedef enum {
    SSL_IDLE_STATE,
    SSL_OPENED_STATE,
    SSL_CONNECTING_STATE,
    SSL_AVAIL_SENDRECV_STATE
}SSL_Status;

#define SSL_SLAVE_MAX_BUF   512

//#define ENABLE_SSL_DEBUG
#if defined(ENABLE_SSL_DEBUG)
#define SSLDEBUG(msg) {Serial.print("[SSLDBG]: "); Serial.println(msg);}
#else
#define SSLDEBUG(msg)
#endif

class SSLClass {
public:
    int Open(void);
    int Close(void);
    int Connect(IPAddress ip, uint16_t port);
    int Connect(const char *host, uint16_t port);
    int WriteData(uint8_t* buf, uint16_t len, bool IsPMEM = false);
    int ReadData(uint8_t* buf, uint16_t len, uint16_t* OutReadSz);

    int SetPeerVerify(bool verify);
    int SetRootCA(const unsigned char *buf, uint16_t len, bool IsPMEM = false);
    int GetVersion(uint8_t* buf, uint16_t len);
    int GetCipherName(uint8_t* buf, uint16_t len);
    int GetX509IssuerName(uint8_t* buf, uint16_t len);
    int GetX509SubjectName(uint8_t* buf, uint16_t len);
    int GetX509NextAltName(uint8_t* buf, uint16_t len);
    int GetX509SerialNum(uint8_t* buf, uint16_t len, uint16_t* OutNumSz);
//    int GetX509Version(uint8_t* buf, uint16_t len);
    int SetDate(uint8_t* buf, uint16_t len);
    int SetTime(uint8_t* buf, uint16_t len);

private:
    static SSL_Status state;
    static bool peerVerify;
};

#endif
