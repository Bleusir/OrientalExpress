/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    errtable.h
 *
 * ���������ͷ�ļ�
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

#ifndef EPS_ERRTABLE_H
#define EPS_ERRTABLE_H


#ifdef __cplusplus
extern "C" {
#endif

/**
 * �궨�� 
 */

#define EPS_ERRDESC_MAX_LEN         256

/**
 * ���Ͷ���
 */
 
/* 
 * ������Ϣ�ṹ�� 
 */
typedef struct ErrorInfoTag
{
    ResCodeT    errCode;                        /* ������ */
    char        errDscr[EPS_ERRDESC_MAX_LEN+1]; /* �������� */
} ErrorInfoT;

/**
 * �ӿں�������
 */
 
/*
 * ����ָ��������Ĵ�����Ϣ
 */
ErrorInfoT* ErrLookupError(ResCodeT errCode);

#ifdef __cplusplus
}
#endif

#endif /* EPS_ERRTABLE_H */