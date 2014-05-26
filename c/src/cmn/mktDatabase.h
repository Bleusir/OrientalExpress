/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    mktDatabase.h
 *
 * �������ݿⶨ��ͷ�ļ�
 *
 * @version $Id
 * @since   2014/03/04
 * @author  Wu Zheng
 */

/**
MODIFICATION HISTORY:
<pre>
================================================================================
DD-MMM-YYYY INIT.    SIR    Modification Description
----------- -------- ------ ----------------------------------------------------
04-MAR-2014 ZHENGWU         ����
================================================================================
</pre>
*/

#ifndef EPS_MKTDATABASE_H
#define EPS_MKTDATABASE_H


#ifdef __cplusplus
extern "C" {
#endif

/**
 * ����ͷ�ļ�
 */

#include "common.h"
#include "epsTypes.h"
#include "errlib.h"
#include "epsData.h"
#include "stepMessage.h"


/**
 * ���Ͷ���
 */
 
/* 
 * �������ݿⶨ��ṹ�� 
 */
typedef struct EpsMktDatabaseTag
{
    BOOL            isSubscribed[EPS_MKTTYPE_NUM + 1];  
    uint32          applID;
    uint64          applSeqNum[EPS_MKTTYPE_NUM + 1];
} EpsMktDatabaseT;


/**
 * �ӿں�������
 */

/*
 * ��ʼ���������ݿ�
 */
ResCodeT InitMktDatabase(EpsMktDatabaseT* pDatabase);

/*
 * ����ʼ���������ݿ�
 */
ResCodeT UninitMktDatabase(EpsMktDatabaseT* pDatabase);

/*
 * ������������
 */
ResCodeT SubscribeMktData(EpsMktDatabaseT* pDatabase, EpsMktTypeT mktType);

/**
 * ȡ�������������ݶ���
 */
ResCodeT UnsubscribeAllMktData(EpsMktDatabaseT* pDatabase);

/*
 * �ж��Ƿ���ܸ�����������
 */
ResCodeT AcceptMktData(EpsMktDatabaseT* pDatabase, const StepMessageT* pMsg);

/*
 * ת���������ݸ�ʽ
 */
ResCodeT ConvertMktData(const StepMessageT* pMsg, EpsMktDataT* pData);


#ifdef __cplusplus
}
#endif

#endif /* EPS_MKTDATABASE_H */
