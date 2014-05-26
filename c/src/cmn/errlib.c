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


#include "common.h"
#include "epsTypes.h"

#include "errlib.h"

/**
 * ȫ�ֶ���
 */

__thread char __errDscr[1024] = {0};        /* ����Ĵ�����Ϣ���� */
__thread ResCodeT __errCode = NO_ERR;       /* ����Ĵ����� */

static const ErrorInfoT* g_pErrorTable = NULL;
static uint32 g_errorTableSize = 0;

/**
 * ����ʵ��
 */

/**
 * ���ش�����Ϣ��
 * @param   pTable               in  - ������Ϣ��
 * @param   tableSize            in  - ������Ϣ����
 */
void ErrLoadErrorTable(const ErrorInfoT* pTable, uint32 tableSize)
{
    g_pErrorTable = pTable;
    g_errorTableSize = tableSize;
}

/**
 * ����ָ���������Ӧ�Ĵ�����Ϣ����
 *
 * @param   errCode               in  - ������
 *
 * @return  �ҵ��򷵻ض�Ӧ������Ϣ���������򷵻�NULL
 */
static const ErrorInfoT* ErrLookupError(ResCodeT errCode)
{
    int32 begin, end, middle;
    const ErrorInfoT* pInfo = NULL;

    if(g_pErrorTable == NULL)
    {
        return pInfo;
    }
    
    begin = 0;
    end = g_errorTableSize - 1;
    while (begin <= end)
    {
        middle = (begin + end) / 2;
        if ((g_pErrorTable[middle].errCode) == errCode)
        {
            pInfo = &g_pErrorTable[middle];
            break;
        }

        if((g_pErrorTable[middle].errCode) < errCode)
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
    
    const ErrorInfoT* pErrorInfo = ErrLookupError(errCode);
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
