/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    uniQueue.c
 *
 * �������ʵ���ļ�
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

/**
 * ����ͷ�ļ�
 */

#include "common.h"
#include "epsTypes.h"
#include "errlib.h"
#include "errcode.h"

#include "uniQueue.h"


/**
 * �ӿں���ʵ��
 */

/**
 * ��ʼ���������
 *
 * @param   pQueue          in  - ������ж���
 * @param   size            in  - ������д�С
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT InitUniQueue(EpsUniQueueT* pQueue, uint32 size)
{
    TRY
    {
        if (pQueue == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pQueue");
        }

        if (size == 0)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "size");
        }

        if (pQueue->container != NULL)
        {
            THROW_ERROR(ERCD_EPS_DUPLICATE_INITED, "UniQueue");
        }

        pQueue->container = calloc(size, sizeof(void*));
        if (pQueue->container == NULL)
        {
            int lstErrno = SYS_ERRNO;
            THROW_ERROR(ERCD_EPS_OPERSYSTEM_ERROR, EpsGetSystemError(lstErrno));
        }
        pQueue->size = size;
        pQueue->header = 0;
        pQueue->tailer = 0;
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * ����ʼ���������
 *
 * @param   pQueue          in  - ������ж���
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT UninitUniQueue(EpsUniQueueT* pQueue)
{
    TRY
    {
        if (pQueue == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pQueue");
        }

        if (pQueue->container != NULL)
        {
            free(pQueue->container);
            pQueue->container = NULL;
        }

        pQueue->size = 0;
        pQueue->header = 0;
        pQueue->tailer = 0;
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * ���������β�����������
 *
 * @param   pQueue          in  - ������ж���
 * @param   pItem           in  - ���������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT PushUniQueue(EpsUniQueueT* pQueue, void* pItem)
{
    TRY
    {
        if (pQueue == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pQueue");
        }

        if (pItem == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pItem");
        }

        if (pQueue->container == NULL)
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "UniQueue");
        }

        if (pQueue->header - pQueue->tailer >= pQueue->size)
        {
            THROW_ERROR(ERCD_EPS_INVALID_OPERATION, "UniQueue is full");
        }

        pQueue->container[pQueue->header++ % pQueue->size] = pItem;
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * �ӵ������ͷ����ȡ������
 *
 *
 * @param   pQueue          in  - ������ж���
 * @param   ppItem          out - ���������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT PopUniQueue(EpsUniQueueT* pQueue, void** ppItem)
{
    TRY
    {
        if (pQueue == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pQueue");
        }

        if (ppItem == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "ppItem");
        }

        if (pQueue->container == NULL)
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "UniQueue");
        }

        if (pQueue->tailer >= pQueue->header)
        {
            *ppItem = NULL;
        }
        else
        {
            *ppItem = pQueue->container[pQueue->tailer++ % pQueue->size];
        }
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * �жϵ�������Ƿ��Ѿ���ʼ��
 *
 * @param   pQueue          in  - ������ж���
 *
 * @return  �Ѿ���ʼ������TRUE�����򷵻�FALSE
 */
BOOL IsUniQueueInited(EpsUniQueueT* pQueue)
{
    if (pQueue == NULL)
    {
        return FALSE;
    }

    return (pQueue->container != NULL);
}
