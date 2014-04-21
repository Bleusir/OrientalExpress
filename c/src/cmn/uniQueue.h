/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    uniQueue.h
 *
 * ������ж���ͷ�ļ�
 *
 * @version $Id
 * @since   2014/04/14
 * @author  Wu Zheng
 */

/**
MODIFICATION HISTORY:
<pre>
================================================================================
DD-MMM-YYYY INIT.    SIR    Modification Description
----------- -------- ------ ----------------------------------------------------
14-APR-2014 ZHENGWU         ����
================================================================================
</pre>
*/

#ifndef EPS_UNIQUEUE_H
#define EPS_UNIQUEUE_H


#ifdef __cplusplus
extern "C" {
#endif

/**
 * ����ͷ�ļ�
 */

#include "common.h"
#include "eps/epsTypes.h"


/**
 * ���Ͷ���
 */

/*
 * �ݹ黥�����ṹ
 */
typedef struct EpsUniQueueTag
{
    uint32      size;       /* ���д�С */
    void**      container;  /* �������� */
    uint32      header;     /* ����ͷ��λ�� */
    uint32      tailer;     /* ����β��λ�� */
} EpsUniQueueT;


/**
 * �ӿں�������
 */

/*
 * ��ʼ���������
 */
ResCodeT InitUniQueue(EpsUniQueueT* pQueue, uint32 size);

/*
 * ����ʼ���������
 */
ResCodeT UninitUniQueue(EpsUniQueueT* pQueue);

/*
 * ���������β�����������
 */
ResCodeT PushUniQueue(EpsUniQueueT* pQueue, void* pItem);

/*
 * �ӵ������ͷ����ȡ������
 */
ResCodeT PopUniQueue(EpsUniQueueT* pQueue, void** ppItem);

/*
 * �жϵ�������Ƿ��Ѿ���ʼ��
 */
BOOL IsUniQueueInited(EpsUniQueueT* pQueue);


#ifdef __cplusplus
}
#endif

#endif /* EPS_UNIQUEUE_H */
