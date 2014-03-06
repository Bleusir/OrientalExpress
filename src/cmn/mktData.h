/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    mktData.h
 *
 * �������ݶ���ͷ�ļ�
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

#ifndef EPS_MKTDATA_H
#define EPS_MKTDATA_H


#ifdef __cplusplus
extern "C" {
#endif

/**
 * ����ͷ�ļ�
 */

#include "errlib.h"
#include "eps/epsData.h"
#include "step/stepMessage.h"


/**
 * ���Ͷ���
 */
 
/* 
 * ������Ϣ�ṹ�� 
 */
typedef struct EpsMktDatabaseTag
{
    EpsMktTypeT     mktType[EPS_MKTTYPE_NUM];
    uint32          applID[EPS_MKTTYPE_NUM];
    uint64          applSeqNum[EPS_MKTTYPE_NUM];
} EpsMktDatabaseT;


/**
 * �ӿں�������
 */
 
ResCodeT InitMktDatabase(EpsMktDatabaseT* pDatabase);

ResCodeT UninitMktDatabase(EpsMktDatabaseT* pDatabase);

ResCodeT AcceptMktData(EpsMktDatabaseT* pDatabase, const StepMessageT* pMsg);

ResCodeT ConvertMktData(const StepMessageT* pMsg, EpsMktDataT* pData);


#ifdef __cplusplus
}
#endif

#endif /* EPS_MKTDATA_H */