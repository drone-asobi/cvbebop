/**
 * @file arstream2_rtp.h
 * @brief Parrot Streaming Library - RTP definitions
 * @date 04/17/2015
 * @author aurelien.barre@parrot.com
 */

#ifndef _ARSTREAM2_RTP_H_
#define _ARSTREAM2_RTP_H_

#include <inttypes.h>

/*
 * Macros
 */

#define ARSTREAM2_RTP_IP_HEADER_SIZE 20
#define ARSTREAM2_RTP_UDP_HEADER_SIZE 8

#define ARSTREAM2_RTP_SSRC 0x41525354

#define ARSTREAM2_RTP_NALU_TYPE_STAPA 24
#define ARSTREAM2_RTP_NALU_TYPE_FUA 28

#define R_OK    4       /* Test for read permission.  */
#define W_OK    2       /* Test for write permission.  */
//#define   X_OK    1       /* execute permission - unsupported in windows*/
#define F_OK    0       /* Test for existence.  */

/*
 * Types
 */

/**
 * @brief RTP Header (see RFC3550)
 */
typedef struct {
    uint16_t flags;
    uint16_t seqNum;
    uint32_t timestamp;
    uint32_t ssrc;
} ARSTREAM2_RTP_Header_t; /* __attribute__ ((packed)) */

#define ARSTREAM2_RTP_TOTAL_HEADERS_SIZE (sizeof(ARSTREAM2_RTP_Header_t) + ARSTREAM2_RTP_UDP_HEADER_SIZE + ARSTREAM2_RTP_IP_HEADER_SIZE)
#define ARSTREAM2_RTP_MAX_PAYLOAD_SIZE (0xFFFF - ARSTREAM2_RTP_TOTAL_HEADERS_SIZE)

/**
 * @brief Format of v2 stream clock frames
 */
typedef struct {
    uint32_t originateTimestampH;
    uint32_t originateTimestampL;
    uint32_t receiveTimestampH;
    uint32_t receiveTimestampL;
    uint32_t transmitTimestampH;
    uint32_t transmitTimestampL;
} ARSTREAM2_RTP_ClockFrame_t; /* __attribute__ ((packed)) */

#endif /* _ARSTREAM2_RTP_H_ */
