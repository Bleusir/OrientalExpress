/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    udpChannel.h
 *
 * UDP�鲥ͨ������ͷ�ļ�
 *
 * @version $Id
 * @since   2014/02/14
 * @author  Wu Zheng
 */

/**
MODIFICATION HISTORY:
<pre>
================================================================================
DD-MMM-YYYY INIT.    SIR    Modification Description
----------- -------- ------ ----------------------------------------------------
14-FEB-2014 ZHENGWU         ����
================================================================================
</pre>
*/

#ifndef EPS_UDP_CHANNEL_H
#define EPS_UDP_CHANNEL_H

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
 * �첽�¼�
 */
typedef struct EpsUdpChannelEventTag
{
    uint32  eventType;      /* �¼����� */
    uint32  eventParam;     /* �¼����� */     
} EpsUdpChannelEventT;

/*
 * �첽�ص��ӿ�
 */
typedef void (*EpsUdpChannelConnectedCallback)(void* pListener);
typedef void (*EpsUdpChannelDisconnectedCallback)(void* pListener, ResCodeT result, const char* reason);
typedef void (*EpsUdpChannelReceivedCallback)(void* pListener, ResCodeT result, const char* data, uint32 dataLen);
typedef void (*EpsUdpChannelEventOccurredCallback)(void* pListener, EpsUdpChannelEventT* pEvent);

/*
 * UDPͨ�������߽ӿ�
 */
typedef struct EpsUdpChannelListenerTag
{
    void*                               pListener;          /* �����߶��� */
    EpsUdpChannelConnectedCallback      connectedNotify;    /* ���ӳɹ�֪ͨ */
    EpsUdpChannelDisconnectedCallback   disconnectedNotify; /* ���ӶϿ�֪ͨ */
    EpsUdpChannelReceivedCallback       receivedNotify;     /* ���ݽ���֪ͨ */
    EpsUdpChannelEventOccurredCallback  eventOccurredNotify;/* �¼�����֪ͨ */
} EpsUdpChannelListenerT;


/*
 * UDPͨ���ṹ
 */
typedef struct EpsUdpChannelTag
{
    char        localAddr[EPS_IP_MAX_LEN+1];/* ���ص�ַ */
    char        mcAddr[EPS_IP_MAX_LEN+1];   /* �鲥��ַ */
    uint16      mcPort;                     /* �鲥�˿� */

    int         socket;                     /* ͨѶ�׽��� */
    pthread_t   tid;                        /* �߳�id */
    GAsyncQueue* pEventQueue;               /* �¼����� */
    char        recvBuffer[EPS_SOCKET_RECVBUFFER_LEN];/* ���ջ����� */
    BOOL        canStop;                    /* ����ֹͣ�߳����б�� */  

    EpsUdpChannelListenerT listener;        /* �����߽ӿ� */
} EpsUdpChannelT;


/**
 * ��������
 */

/*
 * ��ʼ��UDPͨ��
 */
ResCodeT InitUdpChannel(EpsUdpChannelT* pChannel);

/*
 * ����ʼ��UDPͨ��
 */
ResCodeT UninitUdpChannel(EpsUdpChannelT* pChannel);

/*
 * ����UDPͨ��
 */
ResCodeT StartupUdpChannel(EpsUdpChannelT* pChannel);

/*
 * ֹͣUDPͨ��
 */
ResCodeT ShutdownUdpChannel(EpsUdpChannelT* pChannel);

/*
 * �����첽�¼�
 */
ResCodeT TriggerUdpChannelEvent(EpsUdpChannelT* pChannel, const EpsUdpChannelEventT event);

/*
 * ע��ͨ�������߽ӿ�
 */
ResCodeT RegisterUdpChannelListener(EpsUdpChannelT* pChannel, const EpsUdpChannelListenerT* pListener);


#ifdef __cplusplus
}
#endif

#endif /* EPS_UDP_CHANNEL_H */
