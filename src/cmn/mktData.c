/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    mktData.c
 *
 * 行情数据实现文件
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
04-MAR-2014 ZHENGWU         创建
================================================================================
</pre>
*/

/**
 * 包含头文件
 */

#include <stdio.h>
#include <string.h>

#include "mktData.h"


/**
 * 接口函数实现
 */
 
ResCodeT InitMktDatabase(EpsMktDatabaseT* pDatabase)
{
    TRY
    {
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

ResCodeT UninitMktDatabase(EpsMktDatabaseT* pDatabase)
{
    TRY
    {
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
 * 将行情消息从STEP格式转换成行情数据结构
 *
 * @param   pDatabase           in  - 
 * @param   pMsg                in  - STEP格式行情
 *
 * @return  转换成功返回TRUE，否则返回FALSE
 */
ResCodeT AcceptMktData(EpsMktDatabaseT* pDatabase, const StepMessageT* pMsg)
{
    TRY
    {
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
 * 将行情消息从STEP格式转换成行情数据结构
 *
 * @param   pMsg                in  - STEP格式行情
 * @param   pData               out - 行情数据格式
 *
 * @return  转换成功返回TRUE，否则返回FALSE
 */
ResCodeT ConvertMktData(const StepMessageT* pMsg, EpsMktDataT* pData)
{
    TRY
    {
        MDSnapshotFullRefreshRecordT* pRecord = (MDSnapshotFullRefreshRecordT*)pMsg->body;

        memset(pData, 0x00, sizeof(EpsMktDataT));

        memcpy(pData->mktTime, pMsg->sendingTime, EPS_TIME_LEN);

        pData->mktType = EPS_MKTTYPE_DEV;
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
