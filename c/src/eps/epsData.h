/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    epsData.h
 *
 * �����쳵�ӿ�API���ݶ���ͷ�ļ�
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
17-FEB-2014 ZHENGWU         ����
================================================================================
</pre>
*/

#ifndef EPS_DATA_H
#define EPS_DATA_H

/**
 * ����ͷ�ļ�
 */
 
#include "epsTypes.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * �궨��
 */

/*
 * �ֶγ��ȶ���
 */
#define EPS_TIME_LEN                    8
#define EPS_DATE_LEN                    8
#define EPS_UPDATETYPE_LEN              3
#define EPS_MKTDATA_MAX_LEN             4096
#define EPS_USERNAME_MAX_LEN            10
#define EPS_PASSWORD_MAX_LEN            10


/**
 * ���Ͷ���
 */

/*
 * ����ģʽö��
 */
typedef enum EpsConnModeTag
{
    EPS_CONNMODE_UDP        = 1,        /* UDP����ģʽ */
    EPS_CONNMODE_TCP        = 2,        /* TCP����ģʽ */
} EpsConnModeT;

/*
 * �г�����ö��
 */
typedef enum EpsMktTypeTag
{
    EPS_MKTTYPE_ALL         = 0,        /* �����г� */
    EPS_MKTTYPE_STK         = 1,        /* ��Ʊ�г�(��ָ��) */
    EPS_MKTTYPE_DEV         = 2,        /* ����Ʒ�г� */
    EPS_MKTTYPE_NUM         = 2,        /* ֧���г����� */
} EpsMktTypeT;

/*
 * ����ģʽö��
 */
typedef enum EpsTrdSesModeTag
{
    EPS_TRDSES_MODE_TESTING     = 1,    /* ϵͳ����ģʽ */
    EPS_TRDSES_MODE_SIMULATED   = 2,    /* ģ�⽻��ģʽ */
    EPS_TRDSES_MODE_PRODUCTION  = 3,    /* ����ģʽ */
} EpsTrdSesModeT;

/*
 * �¼�����ö��
 */
typedef enum EpsEventTypeTag
{
    EPS_EVENTTYPE_INFORMATION   = 1,    /* ��ʾ��Ϣ���� */
    EPS_EVENTTYPE_WARNING       = 2,    /* ������Ϣ���� */
    EPS_EVENTTYPE_ERROR         = 3,    /* ������Ϣ���� */
    EPS_EVENTTYPE_FATAL         = 4,    /* ���ش�����Ϣ���� */
} EpsEventTypeT;

/*
 * �������ݽṹ
 */
typedef struct EpsMktDataTag
{
    char    mktTime[EPS_TIME_LEN+1];    /* HHMMSSss��ʽ */
    EpsMktTypeT mktType;                /* �г����� */
    EpsTrdSesModeT tradSesMode;         /* ����ģʽ */                
    uint32  applID;                     /* ����ԴID */
    uint64  applSeqNum;                 /* ����������� */
    char    tradeDate[EPS_DATE_LEN+1];  /* YYYYMMDD��ʽ */
    char    mdUpdateType[EPS_UPDATETYPE_LEN+1];/* �������ģʽ */
    uint32  mdCount;                    /* ������Ŀ���� */
    uint32  mdDataLen;                  /* �������ݳ��� */
    char    mdData[EPS_MKTDATA_MAX_LEN];/* �������� */
} EpsMktDataT;


/*
 * �û��ص��ӿں�������
 */
typedef void (*EpsConnectedCallback)(uint32 hid);
typedef void (*EpsDisconnectedCallback)(uint32 hid, ResCodeT result, const char* reason);
typedef void (*EpsLoginRspCallback)(uint32 hid, uint16 heartbeatIntl, ResCodeT result, const char* reason);
typedef void (*EpsLogoutRspCallback)(uint32 hid, ResCodeT result, const char* reason);
typedef void (*EpsMktDataSubRspCallback)(uint32 hid, EpsMktTypeT mktType, ResCodeT result, const char* reason);
typedef void (*EpsMktDataArrivedCallback)(uint32 hid, const EpsMktDataT* pMktData);
typedef void (*EpsEventOccurredCallback)(uint32 hid, EpsEventTypeT eventType, ResCodeT eventCode, const char* eventText);

/*
 * �û��ص��ӿ�
 */
typedef struct EpsClientSpiTag
{
    EpsConnectedCallback        connectedNotify;     /* ���ӳɹ�֪ͨ */
    EpsDisconnectedCallback     disconnectedNotify;  /* ���ӶϿ�֪ͨ */
    EpsLoginRspCallback         loginRspNotify;      /* ��½Ӧ��֪ͨ */
    EpsLogoutRspCallback        logoutRspNotify;     /* �ǳ�Ӧ��֪ͨ */
    EpsMktDataSubRspCallback    mktDataSubRspNotify; /* ���鶩��Ӧ��֪ͨ */
    EpsMktDataArrivedCallback   mktDataArrivedNotify;/* �������ݵ���֪ͨ */
    EpsEventOccurredCallback    eventOccuredNotify;  /* �¼�����֪ͨ */
} EpsClientSpiT;

#ifdef __cplusplus
}
#endif

#endif /* EPS_DATA_H */
