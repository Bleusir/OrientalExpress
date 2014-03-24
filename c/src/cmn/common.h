/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/*
 * @file    common.h
 *
 * 通用定义头文件
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
 17-02-2014  ZHENGWU         创建
 ================================================================================
  </pre>
*/

#ifndef EPS_COMMON_H
#define EPS_COMMON_H


#ifdef __cplusplus
extern "C" {
#endif

/** 
 *  宏定义
 */
 
#define EPS_IP_MAX_LEN                      32

#define EPS_SOCKET_RECVBUFFER_LEN           4096    /* 单位: 字节 */
#define EPS_SOCKET_RECV_TIMEOUT             (1*1000)/* 单位: 毫秒 */
#define EPS_SOCKET_RECONNECT_INTERVAL       (5*1000)/* 单位: 毫秒 */


#ifdef __cplusplus
}
#endif

#endif /* EPS_COMMON_H */
