/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    epsTypes.h
 *
 * 东方快车接口API数据类型定义头文件
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

#ifndef EPS_TYPES_H
#define EPS_TYPES_H


#ifdef __cplusplus
extern "C" {
#endif

/** 
 * 平台相关的预编译宏设置
 */

/*
 * 64位整型类型检测
 */
#if ! defined(__INT64_TYPE)
#   if defined (_WIN32) || defined (__vms)
#       define  __INT64_TYPE        __int64
#   elif defined (__alpha)
#       define  __INT64_TYPE        long
#   else
#       define  __INT64_TYPE        long long
#   endif
#endif

#if defined (_WIN32)
#   define __thread
#endif


/**
 * 数据类型定义
 */

/*
 * 基础数据类型定义
 */
typedef signed char                 int8;
typedef unsigned char               uint8;

typedef short                       int16;
typedef unsigned short              uint16;

typedef int                         int32;
typedef unsigned int                uint32;

typedef __INT64_TYPE                int64;
typedef unsigned __INT64_TYPE       uint64;

typedef float                       float32;
typedef double                      float64;
typedef long double                 float128;

/*
 * NULL 定义
 */
#if ! defined(NULL)
#   ifdef __cplusplus
#       define NULL                 (0L)
#   else
#       define NULL                 ((void*) 0)
#   endif
#endif

/*
 * BOOL 类型定义
 */
#undef  BOOL
#define BOOL                        int

#undef  TRUE
#define TRUE                        (1)

#undef  FALSE
#define FALSE                       (0)

/* 
 * 错误码数据类型定义 
 */
#ifndef ResCodeT
#define ResCodeT    int32
#endif

#ifdef __cplusplus
}
#endif

#endif /* EPS_TYPES_H */
