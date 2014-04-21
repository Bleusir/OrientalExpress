/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    recMutex.h
 *
 * �ݹ黥��������ͷ�ļ�
 *
 * @version $Id
 * @since   2014/04/11
 * @author  Wu Zheng
 */

/**
MODIFICATION HISTORY:
<pre>
================================================================================
DD-MMM-YYYY INIT.    SIR    Modification Description
----------- -------- ------ ----------------------------------------------------
11-APR-2014 ZHENGWU         ����
================================================================================
</pre>
*/

#ifndef EPS_RECMUTEX_H
#define EPS_RECMUTEX_H


#ifdef __cplusplus
extern "C" {
#endif

/**
 * ����ͷ�ļ�
 */

#include <pthread.h>


/**
 * ���Ͷ���
 */

/*
 * �ݹ黥�����ṹ
 */
typedef struct EpsRecMutexTag
{
    pthread_mutex_t     mutex;
} EpsRecMutexT;


/**
 * �ӿں�������
 */

/*
 * ��ʼ���ݹ黥����
 */
void InitRecMutex(EpsRecMutexT* pMutex);

/*
 * ����ʼ���ݹ黥����
 */
void UninitRecMutex(EpsRecMutexT* pMutex);

/*
 * �Եݹ黥��������
 */
void LockRecMutex(EpsRecMutexT* pMutex);

/**
 * �Եݹ黥��������
 */
void UnlockRecMutex(EpsRecMutexT* pMutex);


#ifdef __cplusplus
}
#endif

#endif /* EPS_RECMUTEX_H */
