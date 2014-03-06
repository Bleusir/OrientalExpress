/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    udpDriver.h
 *
 * UDP�鲥����������ͷ�ļ�
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

#ifndef EPS_UDP_DRIVER_H
#define EPS_UDP_DRIVER_H

/**
 * ����ͷ�ļ�
 */

#include "cmn/errlib.h"
#include "cmn/mktData.h"
#include "eps/epsTypes.h"
#include "eps/epsData.h"

#include "udpChannel.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
 * ���Ͷ���
 */

/*
 * UDP�������ṹ
 */
typedef struct EpsUdpDriverTag
{
    uint32          hid;                    /* ���ID */
    EpsUdpChannelT  channel;                /* ����ͨ�� */
    EpsMktDatabaseT database;               /* �������ݿ� */
    EpsClientSpiT   spi;                    /* �û��ص��ӿ� */

    char   username[EPS_USERNAME_MAX_LEN+1]; /* �û��˺� */
    char   password[EPS_PASSWORD_MAX_LEN+1]; /* �û����� */
    uint16 heartbeatIntl;                    /* �������� */
} EpsUdpDriverT;


/**
 * ��������
 */

/*
 *  ��ʼ��UDP������
 */
ResCodeT InitUdpDriver(EpsUdpDriverT* pDriver);

/*
 *  ����ʼ��UDP������
 */
ResCodeT UninitUdpDriver(EpsUdpDriverT* pDriver);

/*
 *  ע��UDP�������ص�������
 */
ResCodeT RegisterUdpDriverSpi(EpsUdpDriverT* pDriver, const EpsClientSpiT* pSpi);

/*
 *  ����UDP����������
 */
ResCodeT ConnectUdpDriver(EpsUdpDriverT* pDriver, const char* address);

/*
 *  �Ͽ�UDP����������
 */
ResCodeT DisconnectUdpDriver(EpsUdpDriverT* pDriver);

/*
 *  ��½UDP������ 
 */
ResCodeT LoginUdpDriver(EpsUdpDriverT* pDriver, 
        const char* username, const char* password, uint16 heartbeatIntl);

/*
 *  �ǳ�UDP������ 
 */
ResCodeT LogoutUdpDriver(EpsUdpDriverT* pDriver, const char* reason);

/*
 *  ����UDP������ 
 */
ResCodeT SubscribeUdpDriver(EpsUdpDriverT* pDriver, EpsMktTypeT mktType);


#ifdef __cplusplus
}
#endif

#endif /* EPS_UDP_DRIVER_H */
