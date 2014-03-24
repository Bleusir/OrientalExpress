/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    errtable.c
 *
 * 错误码表实现文件
 *
 * @version $Id
 * @since   2014/02/17
 * @author  Jin Chengxun
 */

/**
MODIFICATION HISTORY:
<pre>
================================================================================
DD-MMM-YYYY INIT.    SIR    Modification Description
----------- -------- ------ ----------------------------------------------------
17-FEB-2014 CXJIN           创建
================================================================================
</pre>
*/

/**
 * 包含头文件
 */

#include "eps/epsTypes.h"
#include "cmn/errcode.h"
#include "errtable.h"


/**
 * 全局定义
 */
 
/* 
 * 错误码表 
 */
static ErrorInfoT  g_errorTable[] =
{
    {ERCD_EPS_OPERSYSTEM_ERROR, "operation system error, %s"},
    {ERCD_EPS_SOCKET_ERROR, "socket error, %s"},
    {ERCD_EPS_SOCKET_TIMEOUT, "socket timeout"},
    {ERCD_EPS_INVALID_PARM, "invalid param (%s)"},
    {ERCD_EPS_DUPLICATE_INITED, "%s already initialized"},
    {ERCD_EPS_UNINITED, "%s uninitialized"},
    {ERCD_EPS_INVALID_CONNMODE, "invalid connection mode"},
    {ERCD_EPS_INVALID_HID, "invalid hid"},
    {ERCD_EPS_DUPLICATE_REGISTERED, "%s already registered"},
    {ERCD_EPS_INVALID_MKTTYPE, "invalid market type"},
    {ERCD_EPS_UNEXPECTED_MSGTYPE, "unexpected message type(%s)"},
    {ERCD_EPS_INVALID_ADDRESS, "invalid address"},
    {ERCD_EPS_DUPLICATE_CONNECT, "connect already"},
    {ERCD_EPS_MKTTYPE_UNSUBSCRIBED, "market not subscribed"},
    {ERCD_EPS_MKTDATA_BACKFLOW, "market data backflow"},
    {ERCD_EPS_DATASOURCE_CHANGED, "data source changed"},
    {ERCD_EPS_MKTTYPE_DUPSUBSCRIBED, "market duplicate subscribed"},
    {ERCD_EPS_INVALID_OPERATION, "invalid operation, %s"},
    {ERCD_EPS_LOGIN_FAILED, "login failed"},
    {ERCD_EPS_SUBMARKETDATA_FAILED, "subscribe market data failed"},
    
    {ERCD_STEP_INVALID_FLDVALUE, "Invalid field value(%d=%.*s), %s"},
    {ERCD_STEP_BUFFER_OVERFLOW, "Step message buffer overflow"},
    {ERCD_STEP_INVALID_FLDFORMAT, "Invalid field format"},
    {ERCD_STEP_INVALID_TAG, "Invalid field tag(%*.s))"},
    {ERCD_STEP_INVALID_MSGTYPE, "Invalid message type(%s)"},
    {ERCD_STEP_FLD_NOTFOUND, "Field(tag=%s) not found"},
    {ERCD_STEP_STREAM_NOT_ENOUGH, "Message stream not enough"},
    {ERCD_STEP_INVALID_MSGFORMAT, "Invalid message format, %s"},
    {ERCD_STEP_CHECKSUM_FAILED, "Checksum validate failed, %3s(got) != %3s(expected)"},
    {ERCD_STEP_UNEXPECTED_TAG, "Unexpected field tag(%d))"},
};

/**
 * 接口函数实现
 */
 
/**
 * 查找指定错误码对应的错误信息描述
 *
 * @param   errCode               in  - 错误码
 *
 * @return  找到则返回对应错误信息描述，否则返回NULL
 */
ErrorInfoT* ErrLookupError(ResCodeT errCode)
{
    int32 begin, end, middle;
    ErrorInfoT* pInfo = NULL;

    begin = 0;
    end = sizeof(g_errorTable)/sizeof(g_errorTable[0]) - 1;
    while (begin <= end)
    {
        middle = (begin + end) / 2;
        if ((g_errorTable[middle].errCode) == errCode)
        {
            pInfo = &g_errorTable[middle];
            break;
        }

        if((g_errorTable[middle].errCode) < errCode)
        {
            begin = middle + 1;
        }
        else
        {
            end = middle - 1;
        }
    }
    
    return pInfo;
}
