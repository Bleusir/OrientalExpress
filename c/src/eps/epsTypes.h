/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    epsTypes.h
 *
 * �����쳵�ӿ�API�������Ͷ���ͷ�ļ�
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
17-FEB-2014 CXJIN           ����
================================================================================
</pre>
*/

#ifndef EPS_TYPES_H
#define EPS_TYPES_H


#ifdef __cplusplus
extern "C" {
#endif

/** 
 * ƽ̨��ص�Ԥ���������
 */

/*
 * 64λ�������ͼ��
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
 * �������Ͷ���
 */

/*
 * �����������Ͷ���
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
 * NULL ����
 */
#if ! defined(NULL)
#   ifdef __cplusplus
#       define NULL                 (0L)
#   else
#       define NULL                 ((void*) 0)
#   endif
#endif

/*
 * BOOL ���Ͷ���
 */
#undef  BOOL
#define BOOL                        int

#undef  TRUE
#define TRUE                        (1)

#undef  FALSE
#define FALSE                       (0)

/* 
 * �������������Ͷ��� 
 */
#ifndef ResCodeT
#define ResCodeT    int32
#endif

#ifdef __cplusplus
}
#endif

#endif /* EPS_TYPES_H */
