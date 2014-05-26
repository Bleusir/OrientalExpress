/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    atomic.h
 *
 * ԭ�Ӳ�������ͷ�ļ�
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

#ifndef EPS_ATOMIC_H
#define EPS_ATOMIC_H


#ifdef __cplusplus
extern "C" {
#endif

/**
 * ����ͷ�ļ�
 */

#include "common.h"


/**
 * �ӿں�������
 */

/*
 * ԭ�Ӳ����Ƚϲ��滻
 */
BOOL EpsAtomicIntCompareAndExchange (volatile int *atomic, int oldVal, int newVal);

#ifdef __cplusplus
}
#endif

#endif /* EPS_ATOMIC_H */
