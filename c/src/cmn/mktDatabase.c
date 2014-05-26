/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    mktDatabase.c
 *
 * �������ݿ�ʵ���ļ�
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

/**
 * ����ͷ�ļ�
 */

#include "common.h"

#include "mktDatabase.h"


/**
 * �ӿں���ʵ��
 */

/**
 * ��ʼ���������ݿ�
 *
 * @param   pDatabase               in  - �������ݿ�
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT InitMktDatabase(EpsMktDatabaseT* pDatabase)
{
    TRY
    {
        memset(pDatabase, 0x00, sizeof(EpsMktDatabaseT));
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
 * ����ʼ���������ݿ�
 *
 * @param   pDatabase               in  - �������ݿ�
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT UninitMktDatabase(EpsMktDatabaseT* pDatabase)
{
    TRY
    {
        memset(pDatabase, 0x00, sizeof(EpsMktDatabaseT));
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
 * ������������
 *
 * @param   pDatabase               in  - �������ݿ�
 * @param   mktType                 in  - �������г�����
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT SubscribeMktData(EpsMktDatabaseT* pDatabase, EpsMktTypeT mktType)
{
    TRY
    {
        if (mktType > EPS_MKTTYPE_NUM)
        {
            THROW_ERROR(ERCD_EPS_INVALID_MKTTYPE);
        }

        if (mktType == EPS_MKTTYPE_ALL)
        {
            BOOL hasSubscribeAll = TRUE;
            
            int32 mktType1;
            for (mktType1 = EPS_MKTTYPE_ALL + 1; mktType1 <= EPS_MKTTYPE_NUM; mktType1++)
            {
                if (! pDatabase->isSubscribed[mktType1])
                {
                    pDatabase->isSubscribed[mktType1] = TRUE;
                    hasSubscribeAll = FALSE;
                }
            }

            if (hasSubscribeAll)
            {
                THROW_ERROR(ERCD_EPS_MKTTYPE_DUPSUBSCRIBED);
            }
        }
        else
        {
            if (pDatabase->isSubscribed[mktType])
            {
                THROW_ERROR(ERCD_EPS_MKTTYPE_DUPSUBSCRIBED);
            }
            else
            {
                pDatabase->isSubscribed[mktType] = TRUE;
            }
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
 * ȡ�������������ݶ���
 *
 * @param   pDatabase               in  - �������ݿ�
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT UnsubscribeAllMktData(EpsMktDatabaseT* pDatabase)
{
    TRY
    {
        memset(pDatabase->isSubscribed, 0x00, sizeof(pDatabase->isSubscribed));
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
 * �ж��Ƿ���ܸ�����������
 *
 * @param   pDatabase           in  - �������ݿ�
 * @param   pMsg                in  - STEP������Ϣ
 *
 * @return  ���ܷ���TRUE�����򷵻�FALSE
 */
ResCodeT AcceptMktData(EpsMktDatabaseT* pDatabase, const StepMessageT* pMsg)
{
    TRY
    {
        MDSnapshotFullRefreshRecordT* pRecord = (MDSnapshotFullRefreshRecordT*)pMsg->body;

        EpsMktTypeT mktType = (EpsMktTypeT)(atoi(pRecord->securityType));
        if (mktType == EPS_MKTTYPE_ALL || mktType > EPS_MKTTYPE_NUM)
        {
            THROW_ERROR(ERCD_EPS_INVALID_MKTTYPE);
        }

        if (pDatabase->isSubscribed[mktType])
        {
            if (pRecord->applID == pDatabase->applID)
            {
                if (pRecord->applSeqNum > pDatabase->applSeqNum[mktType])
                {
                    pDatabase->applSeqNum[mktType] = pRecord->applSeqNum;
                    THROW_RESCODE(NO_ERR);
                }
                else
                {
                    THROW_RESCODE(ERCD_EPS_MKTDATA_BACKFLOW);
                }
            }
            else
            {
                uint32 applID = pDatabase->applID;
                
                pDatabase->applID = pRecord->applID;
                pDatabase->applSeqNum[mktType] = pRecord->applSeqNum;

                if (applID != 0)
                {
                    THROW_ERROR(ERCD_EPS_DATASOURCE_CHANGED);
                }
            }
        }
        else
        {
            THROW_RESCODE(ERCD_EPS_MKTTYPE_UNSUBSCRIBED);
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
 * ��������Ϣ��STEP��ʽת�����������ݽṹ
 *
 * @param   pMsg                in  - STEP��ʽ����
 * @param   pData               out - �������ݸ�ʽ
 *
 * @return  ת���ɹ�����TRUE�����򷵻�FALSE
 */
ResCodeT ConvertMktData(const StepMessageT* pMsg, EpsMktDataT* pData)
{
    TRY
    {
        MDSnapshotFullRefreshRecordT* pRecord = (MDSnapshotFullRefreshRecordT*)pMsg->body;

        memset(pData, 0x00, sizeof(EpsMktDataT));

        memcpy(pData->mktTime, pMsg->sendingTime, EPS_TIME_LEN);
        pData->mktType = (EpsMktTypeT)(atoi(pRecord->securityType));
        pData->tradSesMode = (EpsTrdSesModeT)pRecord->tradSesMode;
        pData->applID = pRecord->applID;
        pData->applSeqNum = pRecord->applSeqNum;
        memcpy(pData->tradeDate, pRecord->tradeDate, EPS_DATE_LEN);
        memcpy(pData->mdUpdateType, pRecord->mdUpdateType, EPS_UPDATETYPE_LEN);
        pData->mdCount = pRecord->mdCount;
        pData->mdDataLen = pRecord->mdDataLen;
        memcpy(pData->mdData, pRecord->mdData, EPS_MKTDATA_MAX_LEN);
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}
