/*
 * lispd.h
 *
 * This file is part of LISP Mobile Node Implementation.
 * Definitions for lispd.
 * 
 * Copyright (C) 2011 Cisco Systems, Inc, 2011. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Please send any bug reports or fixes you make to the email address(es):
 *    LISP-MN developers <devel@lispmob.org>
 *
 * Written or modified by:
 *    David Meyer       <dmm@cisco.com>
 *    Preethi Natarajan <prenatar@cisco.com>
 *    Lorand Jakab      <ljakab@ac.upc.edu>
 *    Alberto Rodriguez Natal <arnatal@ac.upc.edu>
 *
 */
#ifndef LISPD_H_
#define LISPD_H_



#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#ifdef ANDROID
#include <android/log.h>
#else
#include <ifaddrs.h>
#endif
#include <inttypes.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip6.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <syslog.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <endian.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include "lispd_log.h"


#define LISPD_VERSION					"v0.4.1"


/*
 *  Protocols constants related with timeouts
 *
 */
#define LISPD_INITIAL_MRQ_TIMEOUT       2  // Initial expiration timer for the first MRq
#define LISPD_INITIAL_DDT_MRQ_TIMEOUT   2  // Initial expiration timer for the first DDT MRq
#define LISPD_INITIAL_SMR_TIMEOUT       3  // Initial expiration timer for the first MRq SMR
#define LISPD_INITIAL_PROBE_TIMEOUT     3  // Initial expiration timer for the first MRq RLOC probe
#define LISPD_INITIAL_MR_TIMEOUT        3  // Initial expiration timer for the first Map Register
#define LISPD_SMR_TIMEOUT               6  // Time since interface status change until balancing arrays and SMR is done
#define LISPD_INF_REQ_HANDOVER_TIMEOUT	 1  // Timeout to init info request When NAT aware is true
#define LISPD_EXPIRE_TIMEOUT            1  // Time interval in which events are expired
#define LISPD_MAX_MR_RETRANSMIT         2  // Maximum amount of Map Request retransmissions
#define LISPD_MAX_SMR_RETRANSMIT        2  // Maximum amount of SMR MRq retransmissions
#define LISPD_MAX_PROBE_RETRANSMIT      1  // Maximum amount of RLOC probe MRq retransmissions
#define LISPD_MAX_RETRANSMITS           5  // Maximum amount of retransmits of a message
#define LISPD_MIN_RETRANSMIT_INTERVAL   1  // Minimum time between retransmits of control messages

/*
 *  Determine endianness
 */

#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN
#define BIG_ENDIANS  2
#elif defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN
#define LITTLE_ENDIANS 1
#elif defined(_BYTE_ORDER) && _BYTE_ORDER == _BIG_ENDIAN
#define BIG_ENDIANS  2
#elif defined(_BYTE_ORDER) && _BYTE_ORDER == _LITTLE_ENDIAN
#define LITTLE_ENDIANS 1
#elif defined(BYTE_ORDR) && BYTE_ORDER == BIG_ENDIAN
#define BIG_ENDIANS  2
#elif defined(BYTE_ORDER) && BYTE_ORDER == LITTLE_ENDIAN
#define LITTLE_ENDIANS 1
#elif defined(__386__)
#define LITTLE_ENDIANS 1
#else
# error "Can't determine endianness"
#endif



#define FIELD_AFI_LEN                    2
#define FIELD_PORT_LEN                   2


/*
 *  CO --
 *
 *  Calculate Offset
 *
 *  Try not to make dumb mistakes with
 *  pointer arithmetic
 *
 */

#define CO(addr,len) (((uint8_t *) addr + len))

/*
 *  CNO --
 *
 *  Calculate Negative Offset
 *
 *  Try not to make dumb mistakes with
 *  pointer arithmetic
 *
 */

#define CNO(addr,len) (((uint8_t *) addr - len))


/*
 *  SA_LEN --
 *
 *  sockaddr length
 *
 */

#define SA_LEN(a) ((a == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6))

/*
 *  names for where the udp checksum goes
 */

#ifdef BSD
#define udpsum(x) x->uh_sum
#else
#define udpsum(x) x->check
#endif

/*
 *  lispd constants
 */

#define EVER            ;;
#define LISPD           "lispd"
#define PID_FILE        "/var/run/lispd.pid"

/*
 *  misc parameters
 */

#define IP6VERSION      6   /* what's the symbol? */
#define PACKED          __attribute__ ((__packed__))
#define uchar           u_char

int err;
#define GOOD                1
#define BAD                 0
#define ERR_SRC_ADDR        -1
#define ERR_DST_ADDR        -2
#define ERR_AFI             -3
#define ERR_AFI_LCAF_TYPE   -4
#define ERR_DB              -5
#define ERR_MALLOC          -6
#define ERR_EXIST           -7
#define ERR_NO_EXIST        -8
#define ERR_CTR_IFACE       -9

/***** Negative Map-Reply actions ***/
#define MAPPING_ACT_NO_ACTION           0
#define MAPPING_ACT_NATIVELY_FORWARD    1
#define MAPPING_ACT_SEND_MAP_REQUEST    2
#define MAPPING_ACT_DROP                3


#define TRUE                1
#define FALSE               0
#define UP                  1
#define DOWN                0
#define UNKNOWN           255

/***** NAT status *******/
//#define UNKNOWN         255
#define NO_NAT              0
#define NAT                 1
#define NO_INFO_REPLY       2   /* NAT status of a locator change from UNKNOWN to NO_INFO_REPLY when
                                 * reached the maximum number of retransmits of info request */

#define MAX_IP_PACKET       4096


#define DEFAULT_MAP_REQUEST_RETRIES             3
#define DEFAULT_RLOC_PROBING_RETRIES            2
#define DEFAULT_MAP_REGISTER_TIMEOUT            5  /* PN: expected to be in minutes; however,
                                                     * lisp_mod treats this as seconds instead of
                                                     * minutes
                                                     */
#define MAP_REGISTER_INTERVAL                   60  /* LJ: sets the interval at which periodic
                                                     * map register messages are sent (seconds).
                                                     * The spec recommends 1 minute
                                                     */
#define RLOC_PROBING_INTERVAL                   30  /* LJ: sets the interval at which periodic
                                                     * RLOC probes are sent (seconds) */
#define DEFAULT_RLOC_PROBING_RETRIES_INTERVAL   5   /* Interval in seconds between RLOC probing retries  */
#define DEFAULT_DATA_CACHE_TTL                  60  /* seconds */
#define DEFAULT_SELECT_TIMEOUT                  1000/* ms */


/*
 * LISP Types
 */

#define LISP_MAP_REQUEST                1
#define LISP_MAP_REPLY                  2
#define LISP_MAP_REGISTER               3
#define LISP_MAP_NOTIFY                 4
#define LISP_MAP_REFERRAL               6
#define LISP_INFO_NAT                   7
#define LISP_ENCAP_CONTROL_TYPE         8
#define LISP_CONTROL_PORT               4342
#define LISP_DATA_PORT                  4341

/*
 * Mapping type
 */
#define LOCAL_MAPPING                   0
#define REMOTE_MAPPING                  1


/*
 *  locator_types
 */

#define STATIC_LOCATOR                  0
#define DYNAMIC_LOCATOR                 1
#define PETR_LOCATOR                    2
#define LOCAL_LOCATOR                   3


/*
 * Map register Key type
 */
#define NO_KEY               0
#define HMAC_SHA_1_96        1
#define HMAC_SHA_256_128     2


/*
 * Indicate supported AFIs connectivity
 */

#define NO_AFI_SUPPORT      0
#define AFI_SUPPORT_4       1
#define AFI_SUPPORT_6       2
#define AFI_SUPPORT_4_6     3

/*
 * Netlink mcast groups lispd is interested in
 * for interface management
 */
#define LISPD_IFACE_NLMGRPS     (RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV4_ROUTE) 
/* #define LISPD_IFACE_NLMGRPS     (RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR \
                                                | RTMGRP_IPV4_ROUTE | RTMGRP_IPV6_ROUTE )
*/


/*
 * Maximum length (in bytes) of an IP address
 */
#define MAX_INET_ADDRSTRLEN INET6_ADDRSTRLEN


#define MAX_PRIORITY 0
#define MIN_PRIORITY 254
#define UNUSED_RLOC_PRIORITY 255
#define MIN_WEIGHT 0
#define MAX_WEIGHT 255

/* LISP data packet header */

typedef struct lisphdr {
    #ifdef __LITTLE_ENDIAN_BITFIELD
    uint8_t rflags:3;
    uint8_t instance_id:1;
    uint8_t map_version:1;
    uint8_t echo_nonce:1;
    uint8_t lsb:1;
    uint8_t nonce_present:1;
    #else
    uint8_t nonce_present:1;
    uint8_t lsb:1;
    uint8_t echo_nonce:1;
    uint8_t map_version:1;
    uint8_t instance_id:1;
    uint8_t rflags:3;
    #endif
    uint8_t nonce[3];
    uint32_t lsb_bits;
} lisphdr_t;


/*
 * Lisp address structure
 */
typedef struct {
  union {
    struct in_addr ip;
    struct in6_addr ipv6;
  } address;
  int afi;
} lisp_addr_t;


/*
 *  generic list of addresses
 */

typedef struct _lispd_addr_list_t {
    lisp_addr_t                 *address;
    struct _lispd_addr_list_t   *next;
} lispd_addr_list_t;


/*
 *  generic list of addresses with priority and weight
 */

typedef struct _lispd_weighted_addr_list_t {
    lisp_addr_t                         *address;
    uint8_t                             priority;
    uint8_t                             weight;
    struct _lispd_weighted_addr_list_t  *next;
} lispd_weighted_addr_list_t;



typedef struct _lispd_map_server_list_t {
    lisp_addr_t                     *address;
    uint8_t                         key_type;
    char                            *key;
    uint8_t                         proxy_reply;
    struct _lispd_map_server_list_t *next;
} lispd_map_server_list_t;

typedef struct packet_tuple_ {
    lisp_addr_t                     src_addr;
    lisp_addr_t                     dst_addr;
    uint16_t                        src_port;
    uint16_t                        dst_port;
    uint8_t                         protocol;
} packet_tuple;

typedef struct lispd_site_ID_
{
	uint8_t	byte[8];

} lispd_site_ID;

typedef struct lispd_xTR_ID_
{
	uint8_t	byte[16];

} lispd_xTR_ID;

/*
 *  for map-register auth data...
 */

#define LISP_SHA1_AUTH_DATA_LEN         20
#define LISP_SHA256_AUTH_DATA_LEN       32


/*
 * Mapping record used in all LISP control messages.
 *
 *  +--->  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |      |                          Record  TTL                          |
 *  |      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  R      | Locator Count | EID mask-len  | ACT |A|       Reserved        |
 *  e      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  c      | Rsvd  |  Map-Version Number   |            EID-AFI            |
 *  o      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  r      |                          EID-prefix                           |
 *  d      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |     /|    Priority   |    Weight     |  M Priority   |   M Weight    |
 *  |    / +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |  Loc |         Unused Flags    |L|p|R|           Loc-AFI             | 
 *  |    \ +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |     \|                             Locator                           |
 *  +--->  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

/*
 * Fixed portion of the mapping record. EID prefix address and
 * locators follow.
 */

typedef struct lispd_pkt_mapping_record_t_ {
    uint32_t ttl;
    uint8_t locator_count;
    uint8_t eid_prefix_length;
#ifdef LITTLE_ENDIANS
    uint8_t reserved1:4;
    uint8_t authoritative:1;
    uint8_t action:3;
#else
    uint8_t action:3;
    uint8_t authoritative:1;
    uint8_t reserved1:4;
#endif
    uint8_t reserved2;
#ifdef LITTLE_ENDIANS
    uint8_t version_hi:4;
    uint8_t reserved3:4;
#else
    uint8_t reserved3:4;
    uint8_t version_hi:4;
#endif
    uint8_t version_low;
    uint16_t eid_prefix_afi;
} PACKED lispd_pkt_mapping_record_t;



/*
 * Fixed portion of the mapping record locator. Variable length
 * locator address follows.
 */
typedef struct lispd_pkt_mapping_record_locator_t_ {
    uint8_t priority;
    uint8_t weight;
    uint8_t mpriority;
    uint8_t mweight;
    uint8_t unused1;
#ifdef LITTLE_ENDIANS
    uint8_t reachable:1;
    uint8_t probed:1;
    uint8_t local:1;
    uint8_t unused2:5;
#else
    uint8_t unused2:5;
    uint8_t local:1;
    uint8_t probed:1;
    uint8_t reachable:1;
#endif
    uint16_t locator_afi;
} PACKED lispd_pkt_mapping_record_locator_t;



/*
 * Structure to simplify netlink processing
 */
typedef struct nlsock_handle
{
    int         fd;       // netlink socket fd
    uint32_t    seq;      // netlink message seq number
} nlsock_handle;


//modified by arnatal for NATT
/*
 * LISP Data header structure
 */

typedef struct lisp_data_hdr {
#ifdef LITTLE_ENDIANS
    uint8_t rflags:3;
    uint8_t instance_id:1;
    uint8_t map_version:1;
    uint8_t echo_nonce:1;
    uint8_t lsb:1;
    uint8_t nonce_present:1;
#else
    uint8_t nonce_present:1;
    uint8_t lsb:1;
    uint8_t echo_nonce:1;
    uint8_t map_version:1;
    uint8_t instance_id:1;
    uint8_t rflags:3;
#endif
    uint8_t nonce[3];
    uint32_t lsb_bits;
} lisp_data_hdr_t;

/*
 * Encapsulated control message header. This is followed by the IP
 * header of the encapsulated LISP control message.
 *
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |Type=8 |S|D|               Reserved                            |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

typedef struct lisp_encap_control_hdr {
#ifdef LITTLE_ENDIANS
    uint8_t reserved:2;
    uint8_t ddt_bit:1;
    uint8_t s_bit:1;
    uint8_t type:4;
#else
    uint8_t type:4;
    uint8_t s_bit:1;
    uint8_t ddt_bit:1;
    uint8_t reserved1:2;
#endif
    uint8_t reserved2[3];
} lisp_encap_control_hdr_t;


/*
 * Structure to set Map Request options/flags
 */
typedef struct {
    uint8_t                 ddt_bit;    // Include DDT bit solicit
    uint8_t                 s_bit;      // Include Security bit
} encap_control_opts;


void exit_cleanup(void);


#endif /*LISPD_H_*/

/*
 * Editor modelines
 *
 * vi: set shiftwidth=4 tabstop=4 expandtab:
 * :indentSize=4:tabSize=4:noTabs=true:
 */
