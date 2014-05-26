/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/*
 * @file    common.h
 *
 * ͨ�ö���ͷ�ļ�
 *
 * @version $Id
 * @since   2014/02/17
 * @author  Wu Zheng
 *    
 */

/*
 MODIFICATION HISTORY:
 <pre>
 ================================================================================
 DD-MMM-YYYY INIT.    SIR    Modification Description
 ----------- -------- ------ ----------------------------------------------------
 17-02-2014  ZHENGWU         ����
 ================================================================================
  </pre>
*/

#ifndef EPS_COMMON_H
#define EPS_COMMON_H

/** 
 * ƽ̨��ص�Ԥ���������
 */

/* 
 * ƽ̨���ͼ��
 */
#if ! defined (__LINUX__) && (defined (__linux__) || defined (__KERNEL__) \
        || defined(_LINUX) || defined(LINUX))
#   define  __LINUX__               (1)
#elif ! defined (__HPUX__) && (defined (__hpux) || defined (__HPUX) \
        || defined(__hpux__) || defined(hpux) || defined(HPUX))
#   define  __HPUX__                (1)
#elif ! defined (__AIX__) && defined (_AIX)
#   define  __AIX__                 (1)
#elif ! defined (__SOLARIS__) && (defined (__solaris__) || defined (__sun__) \
        || defined(sun))
#   define  __SOLARIS__             (1)
#elif ! defined (__BSD__) && (defined (__FreeBSD__) || defined (__OpenBSD__) \
        || defined (__NetBSD__))
#   define  __BSD__                 (1)
#elif ! defined (__APPLE__) && defined (__MacOSX__)
#   define  __APPLE__               (1)
#elif ! defined (__WINDOWS__) && (defined (_WIN32) || defined (WIN32) \
        || defined(__WIN32__) || defined(__Win32__))
#   define  __WINDOWS__             (1)
#elif ! defined (__CYGWIN__) && (defined (__CYGWIN32__) || defined (CYGWIN))
#   define  __CYGWIN__              (1)
#endif


/**
 * ����ͷ�ļ�
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>

#if defined(__LINUX__) || defined(__HPUX__) 
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

#if defined(__WINDOWS__)
#include <time.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * NULL ����
 */
#if ! defined(NULL)
#   ifdef __cplusplus
#       define NULL                 (0L)
#   else
#       define NULL                 ((void*) 0)
#   endif
#endif

/*
 * BOOL ���Ͷ���
 */
#if defined(__LINUX__) || defined(__HPUX__)
#undef  BOOL
#define BOOL                        int
#endif

#undef  TRUE
#define TRUE                        (1)

#undef  FALSE
#define FALSE                       (0)

/** 
 *  �궨��
 */


#if defined(__LINUX__) || defined(__HPUX__) 
#define NET_ERRNO   errno
#define SYS_ERRNO   errno

#define INVALID_SOCKET  (-1)
#define SOCKET_ERROR	(-1)

typedef int         SOCKET;

#endif


#if defined(__WINDOWS__)
#define NET_ERRNO   WSAGetLastError()
#define SYS_ERRNO   GetLastError()

#define SHUT_RDWR   2
typedef int 		socklen_t;
#endif


#define EPS_IP_MAX_LEN                      (32)        /* IP�ַ����ֶ���󳤶� */

#define EPS_SOCKET_RECVBUFFER_LEN           (4096)      /* �׽��ֽ��ջ�������С����λ: �ֽ� */
#define EPS_SOCKET_RECV_TIMEOUT             (1*1000)    /* �׽��ֽ��ճ�ʱ����λ: ���� */

#define EPS_CHANNEL_RECONNECT_INTL          (1*1000)    /* ����ͨ������ʱ��������λ: ���� */
#define EPS_CHANNEL_IDLE_INTL               (500)       /* ����ͨ������ʱ��������λ: ���� */

#define EPS_DRIVER_KEEPALIVE_TIME           (35*1000)   /* ��������������Ծʱ�䷧ֵ����λ: ���� */


/**
 *  �ӿں�������
 */

/*
 * ��ȡ����ϵͳ��������
 */
const char* EpsGetSystemError(int errCode);

#ifdef __cplusplus
}
#endif

#endif /* EPS_COMMON_H */
