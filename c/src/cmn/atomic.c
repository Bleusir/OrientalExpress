/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    atomic.c
 *
 * ԭ�Ӳ���ʵ���ļ�
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

#include "atomic.h"


/**
 * �ӿں���ʵ��
 */

/**
 * ԭ�Ӳ����Ƚϲ��滻
 *
 * @param   atomic          in  - ���Ƚ�ֵ
 *                          out - �ȽϺ���ֵ
 * @param   oldVal          in  - �ȽϾ�ֵ
 * @param   newVal          in  - �Ƚ���ֵ
 *
 * @return  atomic���ֵ��ȷ���TRUE, ���򷵻�FALSE
 */
BOOL EpsAtomicIntCompareAndExchange (volatile int *atomic, int oldVal, int newVal)
{  
    int result;   

    __asm__ __volatile__ ("lock; cmpxchgl %2, %1"
        : "=a" (result), "=m" (*atomic)         
        : "r" (newVal), "m" (*atomic), "0" (oldVal));   

    return result == oldVal;
}
