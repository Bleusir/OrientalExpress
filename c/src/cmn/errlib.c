/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/*
 * @file    errlib.c
 *
 * ����ⶨ��ͷ�ļ�
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
 17-02-2014  CXJIN           ����
 ================================================================================
  </pre>
*/

/**
 * ����ͷ�ļ�
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
 * ȫ�ֶ���
 */

__thread char __errDscr[1024] = {0};        /* ����Ĵ�����Ϣ���� */
__thread ResCodeT __errCode = NO_ERR;       /* ����Ĵ����� */


/**
 * ����ʵ��
 */
 
/**
 * ���ô�����Ϣ
 *
 * @param   errCode                 in  - ������
 * @param   ...                     in  - ������Ϣ����(��ѡ���) 
 *
 * @return  NO_ERR-�ɹ�; ����-ʧ��
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
 * ��ȡ������Ϣ����
 *
 * @return  ������Ϣ����
 */
const char* ErrGetErrorDscr()
{
    return __errDscr;
}

/**
 * ��ȡ������
 *
 * @return  ���һ�δ�����
 */
ResCodeT ErrGetErrorCode()
{
    return __errCode;
}

/**
 * ��մ�����Ϣ
 */
void ErrClearError()
{
    __errCode    = NO_ERR;
    __errDscr[0] = '\0';
}
