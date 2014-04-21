/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    recMutex.c
 *
 * �ݹ黥��������ʵ���ļ�
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

/**
 * ����ͷ�ļ�
 */

#include "common.h"
#include "recMutex.h"


/**
 * �ӿں���ʵ��
 */

/**
 * ��ʼ���ݹ黥����
 *
 * @param   pMutex                  in  - �ݹ黥����
 */
void InitRecMutex(EpsRecMutexT* pMutex)
{
    if (pMutex == NULL)
    {
        return;
    }

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&pMutex->mutex, &attr);
}

/**
 * ����ʼ���ݹ黥����
 *
 * @param   pMutex                  in  - �ݹ黥����
 */
void UninitRecMutex(EpsRecMutexT* pMutex)
{
    if (pMutex == NULL)
    {
        return;
    }

    pthread_mutex_destroy(&pMutex->mutex);
}

/**
 * �Եݹ黥��������
 *
 * @param   pMutex                  in  - �ݹ黥����
 */
void LockRecMutex(EpsRecMutexT* pMutex)
{
    if (pMutex == NULL)
    {
        return;
    }

    pthread_mutex_lock(&pMutex->mutex);
}

/**
 * �Եݹ黥��������
 *
 * @param   pMutex                  in  - �ݹ黥����
 */
void UnlockRecMutex(EpsRecMutexT* pMutex)
{
    if (pMutex == NULL)
    {
        return;
    }

    pthread_mutex_unlock(&pMutex->mutex);
}
