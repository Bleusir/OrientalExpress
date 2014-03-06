/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    errtable.h
 *
 * 错误码表定义头文件
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

#ifndef EPS_ERRTABLE_H
#define EPS_ERRTABLE_H


#ifdef __cplusplus
extern "C" {
#endif

/**
 * 宏定义 
 */

#define EPS_ERRDESC_MAX_LEN         256

/**
 * 类型定义
 */
 
/* 
 * 错误信息结构体 
 */
typedef struct ErrorInfoTag
{
    ResCodeT    errCode;                        /* 错误码 */
    char        errDscr[EPS_ERRDESC_MAX_LEN+1]; /* 错误描述 */
} ErrorInfoT;

/**
 * 接口函数定义
 */
 
/*
 * 查找指定错误码的错误信息
 */
ErrorInfoT* ErrLookupError(ResCodeT errCode);

#ifdef __cplusplus
}
#endif

#endif /* EPS_ERRTABLE_H */