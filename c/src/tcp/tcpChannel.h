/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    tcpChannel.h
 *
 * TCP�շ�ͨ������ͷ�ļ�
 *
 * @version $Id
 * @since   2014/03/11
 * @author  Wu Zheng
 */

/**
MODIFICATION HISTORY:
<pre>
================================================================================
DD-MMM-YYYY INIT.    SIR    Modification Description
----------- -------- ------ ----------------------------------------------------
11-MAR-2014 ZHENGWU         ����
================================================================================
</pre>
*/

#ifndef EPS_TCP_CHANNEL_H
#define EPS_TCP_CHANNEL_H

/**
 * ����ͷ�ļ�
 */

#include <glib.h>
#include <sys/socket.h>
#include <pthread.h>

#include "cmn/common.h"
#include "eps/epsTypes.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
 * ���Ͷ���
 */

/*
 * �첽�ص��ӿ�
 */
typedef void (*EpsTcpChannelConnectedCallback)(void* pListener);
typedef void (*EpsTcpChannelDisconnectedCallback)(void* pListener, ResCodeT result, const char* reason);
typedef void (*EpsTcpChannelReceivedCallback)(void* pListener, ResCodeT result, const char* data, uint32 dataLen);
typedef void (*EpsTcpChannelSendedCallback)(void* pListener, ResCodeT result, const char* data, uint32 dataLen);

/*
 * UDPͨ�������߽ӿ�
 */
typedef struct EpsTcpChannelListenerTag
{
    void*                               pListener;          /* �����߶��� */
    EpsTcpChannelConnectedCallback      connectedNotify;    /* ���ӳɹ�֪ͨ */
    EpsTcpChannelDisconnectedCallback   disconnectedNotify; /* ���ӶϿ�֪ͨ */
    EpsTcpChannelReceivedCallback       receivedNotify;     /* ���ݽ���֪ͨ */
    EpsTcpChannelSendedCallback         sendedNotify;       /* ���ݷ���֪ͨ */
} EpsTcpChannelListenerT;


/*
 * TCPͨ���ṹ
 */
typedef struct EpsTcpChannelTag
{
    char        srvAddr[EPS_IP_MAX_LEN+1];  /* ��������ַ */
    uint16      srvPort;                    /* �������˿� */

    int         socket;                     /* ͨѶ�׽��� */
    pthread_t   tid;                        /* �߳�id */
    GAsyncQueue* pSendQueue;                /* ���Ͷ��� */
    char        recvBuffer[EPS_SOCKET_RECVBUFFER_LEN];/* ���ջ����� */
    BOOL        canStop;                    /* ����ֹͣ�߳����б�� */

    EpsTcpChannelListenerT listener;        /* �����߽ӿ� */
} EpsTcpChannelT;


/**
 * ��������
 */

/*
 * ��ʼ��TCPͨ��
 */
ResCodeT InitTcpChannel(EpsTcpChannelT* pChannel);

/*
 * ����ʼ��TCPͨ��
 */
ResCodeT UninitTcpChannel(EpsTcpChannelT* pChannel);

/*
 * ����TCPͨ��
 */
ResCodeT StartupTcpChannel(EpsTcpChannelT* pChannel);

/*
 * ֹͣTCPͨ��
 */
ResCodeT ShutdownTcpChannel(EpsTcpChannelT* pChannel);

/*
 * ��TCPͨ����������
 */
ResCodeT SendTcpChannel(EpsTcpChannelT* pChannel, const char* data, uint32 dataLen);

/*
 * ע��ͨ�������߽ӿ�
 */
ResCodeT RegisterTcpChannelListener(EpsTcpChannelT* pChannel, const EpsTcpChannelListenerT* pListener);


#ifdef __cplusplus
}
#endif

#endif /* EPS_UDP_CHANNEL_H */
