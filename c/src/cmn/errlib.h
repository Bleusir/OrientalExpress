/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/*
 * @file    errlib.h
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

#ifndef EPS_ERRLIB_H
#define EPS_ERRLIB_H

/**
 * ����ͷ�ļ�
 */

#include "errcode.h"
#include "epsTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * �����ӿڶ���
 */

/* 
 * �������������Ͷ��� 
 */
#ifndef ResCodeT
#define ResCodeT    int32
#endif


/* 
 * ������Ϣ�ṹ�� 
 */

#define EPS_ERRDESC_MAX_LEN         256         /* ����������󳤶� */

typedef struct ErrorInfoTag
{
    ResCodeT    errCode;                        /* ������ */
    char        errDscr[EPS_ERRDESC_MAX_LEN+1]; /* �������� */
} ErrorInfoT;

/* ȫ�ִ��������� */
extern __thread ResCodeT __errCode;

/* ִ�гɹ������� */
#define NO_ERR                                  1

/* �����������̿� */
#define TRY                                     \
    ResCodeT __rc = NO_ERR;                     \
    
/* �����쳣�������̿� */
#define CATCH                                   \
    goto el_finally;                            \
    goto el_catch;                              \
    el_catch:
    
/* ��β�������̿� */
#define FINALLY                                 \
    el_finally:


/* �жϷ������Ƿ�ɹ� */
#define OK(_rc_)                                \
    ((_rc_) == NO_ERR)

/* �жϷ������Ƿ���� */
#define NOTOK(_rc_)                             \
    ((_rc_) != NO_ERR)

/* ���÷����� */
#define SET_RESCODE(_rc_)                       \
    __rc = (_rc_);

/* ��ȡ������ */
#define GET_RESCODE() (__rc)

/* ���ط����� */
#define RETURN_RESCODE                          \
    return __rc;


/* ���÷����룬����ת��CATCH */
#define THROW_RESCODE(_rc_)                     \
do                                              \
{                                               \
    if(OK(__rc = (_rc_))) {goto el_finally;}    \
    else                   {goto el_catch;}     \
} while (0);

/* THROW_ERROR ���ô����뼰������Ϣ������ת��CATCH��������ѡ */
#define THROW_ERROR(_errCode, _params...)       \
do                                              \
{                                               \
    if ((_errCode) != NO_ERR)                   \
    {                                           \
        if (__errCode != (_errCode))            \
        {                                       \
            ErrSetError(_errCode, ##_params);   \
        }                                       \
        __rc = (_errCode);                      \
        goto el_catch;                          \
    }                                           \
} while (0)

/**
 * ���ش�����Ϣ��
 * @param   pTable               in  - ������Ϣ��
 * @param   tableSize            in  - ������Ϣ����
 */
void ErrLoadErrorTable(const ErrorInfoT* pTable, uint32 tableSize);
        
/**
 * ���ô�����Ϣ
 *
 * @param   errCode                 in  - ������
 * @param   ...                     in  - ������Ϣ����(��ѡ���)
 */
void ErrSetError(ResCodeT errCode, ...);

/**
 * ��ȡ������Ϣ����
 *
 * @return  ���һ�δ�����Ϣ����
 */
const char* ErrGetErrorDscr();

/**
 * ��ȡ������
 *
 * @return  ���һ�δ�����
 */
ResCodeT ErrGetErrorCode();

/**
 * ��մ�����Ϣ
 */
void ErrClearError();

#ifdef __cplusplus
}
#endif

#endif  /* EPS_ERRLIB_H */
