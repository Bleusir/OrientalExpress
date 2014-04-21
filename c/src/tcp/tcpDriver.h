/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    tcpDriver.h
 *
 * TCP��������������ͷ�ļ�
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

#ifndef EPS_TCP_DRIVER_H
#define EPS_TCP_DRIVER_H

/**
 * ����ͷ�ļ�
 */

#include "cmn/errlib.h"
#include "cmn/recMutex.h"
#include "cmn/mktDatabase.h"
#include "eps/epsTypes.h"
#include "eps/epsData.h"

#include "tcpChannel.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
 * ���Ͷ���
 */

/*
 * TCP������״̬ö��
 */
typedef enum EpsTcpStatusTag
{
    EPS_TCP_STATUS_DISCONNECTED     = 0,    /* ���ӶϿ� */
    EPS_TCP_STATUS_CONNECTED        = 1,    /* ���ӳɹ� */
    EPS_TCP_STATUS_LOGGING          = 2,    /* ��½�� */
    EPS_TCP_STATUS_LOGINED          = 3,    /* ��½�ɹ� */
    EPS_TCP_STATUS_PUBLISHING       = 4,    /* ���鷢���� */
    EPS_TCP_STATUS_LOGOUTING        = 5,    /* �ǳ��� */
    EPS_TCP_STATUS_LOGOUT           = 6,    /* �ѵǳ� */
} EpsTcpStatusT;

/*
 * TCP�������ṹ
 */
typedef struct EpsTcpDriverTag
{
    uint32          hid;                    /* ���ID */
    
    EpsTcpChannelT  channel;                /* ����ͨ�� */
    EpsMktDatabaseT database;               /* �������ݿ� */
    EpsClientSpiT   spi;                    /* �û��ص��ӿ� */
    
    EpsTcpStatusT   status;                 /* ������״̬ */
    uint64          msgSeqNum;              /* ��Ϣ��� */
    char            recvBuffer[STEP_MSG_MAX_LEN*2];/* ���ջ����� */
    uint32          recvBufferLen;          /* ���ջ��������� */
    EpsRecMutexT    lock;                   /* �������� */
    
    char   username[EPS_USERNAME_MAX_LEN+1]; /* �û��˺� */
    char   password[EPS_PASSWORD_MAX_LEN+1]; /* �û����� */
    uint16 heartbeatIntl;                    /* �������� */
    uint16 recvIdleTimes;                    /* ���տ��м��� */  
    uint16 commIdleTimes;                    /* ͨѶ���м��� */
} EpsTcpDriverT;


/**
 * ��������
 */

/*
 *  ��ʼ��TCP������
 */
ResCodeT InitTcpDriver(EpsTcpDriverT* pDriver);

/*
 *  ����ʼ��TCP������
 */
ResCodeT UninitTcpDriver(EpsTcpDriverT* pDriver);

/*
 *  ע��TCP�������ص�������
 */
ResCodeT RegisterTcpDriverSpi(EpsTcpDriverT* pDriver, const EpsClientSpiT* pSpi);

/*
 *  ����TCP����������
 */
ResCodeT ConnectTcpDriver(EpsTcpDriverT* pDriver, const char* address);

/*
 *  �Ͽ�TCP����������
 */
ResCodeT DisconnectTcpDriver(EpsTcpDriverT* pDriver);

/*
 *  ��½TCP������ 
 */
ResCodeT LoginTcpDriver(EpsTcpDriverT* pDriver, 
        const char* username, const char* password, uint16 heartbeatIntl);

/*
 *  �ǳ�TCP������ 
 */
ResCodeT LogoutTcpDriver(EpsTcpDriverT* pDriver, const char* reason);

/*
 *  ����TCP������ 
 */
ResCodeT SubscribeTcpDriver(EpsTcpDriverT* pDriver, EpsMktTypeT mktType);


#ifdef __cplusplus
}
#endif

#endif /* EPS_TCP_DRIVER_H */
