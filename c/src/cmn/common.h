/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/*
 * @file    common.h
 *
 * ͨ�ö���ͷ�ļ�
 *
 * @version $Id
 * @since   2014/02/17
 * @author  Wu Zheng
 *    
 */

/*
 MODIFICATION HISTORY:
 <pre>
 ================================================================================
 DD-MMM-YYYY INIT.    SIR    Modification Description
 ----------- -------- ------ ----------------------------------------------------
 17-02-2014  ZHENGWU         ����
 ================================================================================
  </pre>
*/

#ifndef EPS_COMMON_H
#define EPS_COMMON_H

#include "memwatch.h"


#ifdef __cplusplus
extern "C" {
#endif

/** 
 *  �궨��
 */
 
#define EPS_IP_MAX_LEN                      (32)        /* IP�ַ����ֶ���󳤶� */

#define EPS_SOCKET_RECVBUFFER_LEN           (4096)      /* �׽��ֽ��ջ�������С����λ: �ֽ� */
#define EPS_SOCKET_RECV_TIMEOUT             (1*1000)    /* �׽��ֽ��ճ�ʱ����λ: ���� */

#define EPS_CHANNEL_RECONNECT_INTL          (5*1000)    /* ����ͨ������ʱ��������λ: ���� */
#define EPS_CHANNEL_IDLE_INTL               (500)       /* ����ͨ������ʱ��������λ: ���� */

#define EPS_DRIVER_KEEPALIVE_TIME           (35*1000)   /* ��������������Ծʱ�䷧ֵ����λ: ���� */

#ifdef __cplusplus
}
#endif

#endif /* EPS_COMMON_H */
