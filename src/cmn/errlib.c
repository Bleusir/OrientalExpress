/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/*
 * @file    errlib.c
 *
 * 错误库定义头文件
 *
 * @version $Id
 * @since   2014/02/17
 * @author  Jin Chengxun
 *
 */

/*
 MODIFICATION HISTORY:
 <pre>
 ================================================================================
 DD-MMM-YYYY INIT.    SIR    Modification Description
 ----------- -------- ------ ----------------------------------------------------
 17-02-2014  CXJIN           创建
 ================================================================================
  </pre>
*/

/**
 * 包含头文件
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "eps/epsTypes.h"
#include "errlib.h"
#include "errtable.h"


/**
 * 全局定义
 */

__thread char __errDscr[1024] = {0};        /* 最近的错误信息描述 */
__thread ResCodeT __errCode = NO_ERR;       /* 最近的错误码 */


/**
 * 函数实现
 */
 
/**
 * 设置错误信息
 *
 * @param   errCode                 in  - 错误码
 * @param   ...                     in  - 错误信息参数(可选变参) 
 *
 * @return  NO_ERR-成功; 其他-失败
 */
void ErrSetError(ResCodeT errCode, ...)
{
    __errCode = errCode;
    
    ErrorInfoT* pErrorInfo = ErrLookupError(errCode);
    if (pErrorInfo == NULL)
    {
        snprintf(__errDscr, sizeof(__errDscr), "Unknown error code: %d", errCode);
        return;
    }
 
    va_list valist;
    va_start(valist, errCode);
    vsnprintf(__errDscr, sizeof(__errDscr), pErrorInfo->errDscr, valist);
    va_end(valist);
}

/**
 * 获取错误信息描述
 *
 * @return  错误信息描述
 */
const char* ErrGetErrorDscr()
{
    return __errDscr;
}

/**
 * 获取错误码
 *
 * @return  最近一次错误码
 */
ResCodeT ErrGetErrorCode()
{
    return __errCode;
}

/**
 * 清空错误信息
 */
void ErrClearError()
{
    __errDscr[0] = '\0';
}
