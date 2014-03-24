/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    epsClient.c
 *
 * Express接口API实现文件
 *
 * @version $Id
 * @since   2014/02/14
 * @author  Wu Zheng
 */

/**
MODIFICATION HISTORY:
<pre>
================================================================================
DD-MMM-YYYY INIT.    SIR    Modification Description
----------- -------- ------ ----------------------------------------------------
17-FEB-2014 ZHENGWU         创建
================================================================================
</pre>
*/

/**
 * 包含头文件
 */

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "cmn/errlib.h"
#include "udp/udpDriver.h"
#include "tcp/tcpDriver.h"

#include "epsClient.h"


/**
 * 类型定义
 */

/*
 * 句柄类型
 */
typedef struct EpsHandleTag
{
    uint32          hid;        /* 句柄ID */
    EpsConnModeT    connMode;   /* 连接模式 */
    union EpsDriverTag
    {
        EpsUdpDriverT udpDriver;
        EpsTcpDriverT tcpDriver;
    } driver;                   /* 驱动器 */
} EpsHandleT;


/**
 * 全局定义
 */
 
static volatile gint   g_isLibInited = FALSE;   /* 库初始化标记 */
static volatile uint32 g_maxHid = 0;            /* 当前最大句柄ID */
static GHashTable*     g_handlePool = NULL;     /* 句柄库 */
static GStaticRecMutex g_libLock;               /* 库同步对象 */


/**
 * 内部函数申明
 */

static BOOL IsLibInited();

static void DisconnectHandle(EpsHandleT* pHandle);
static void DestroyHandle(EpsHandleT* pHandle);

static void _disposeHandle(gpointer data);


/**
 * 接口函数定义
 */

/**
 * 初始化Express库
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT EpsInitLib()
{
    TRY
    {
        if (g_atomic_int_compare_and_exchange(&g_isLibInited, FALSE, TRUE))
        {
        	if (!g_thread_get_initialized())
        	{
        		g_thread_init(NULL);
        	}

            g_handlePool = g_hash_table_new_full(g_int_hash, g_int_equal, NULL, _disposeHandle);
            g_maxHid = 1;
            g_static_rec_mutex_init(&g_libLock);
        }
        else
        {
            THROW_ERROR(ERCD_EPS_DUPLICATE_INITED, "library");
        }
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * 反初始化Express库
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT EpsUninitLib()
{
    TRY
    {
        if (g_atomic_int_compare_and_exchange(&g_isLibInited, TRUE, FALSE))
        {
            g_static_rec_mutex_lock(&g_libLock);

            g_hash_table_destroy(g_handlePool);
            g_handlePool = NULL;
            g_maxHid = 0;

            g_static_rec_mutex_unlock(&g_libLock);

            g_static_rec_mutex_free(&g_libLock);
        }
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * 创建指定连接模式的操作句柄
 *
 * @param   pHid            out - 创建的句柄ID
 * @param   mode            in  - 连接模式
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT EpsCreateHandle(uint32* pHid, EpsConnModeT mode)
{
    EpsHandleT* pHandle = NULL;
    
    TRY
    {
        if (pHid == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pHid");
        }

        if (mode != EPS_CONNMODE_UDP && mode != EPS_CONNMODE_TCP)
        {
            THROW_ERROR(ERCD_EPS_INVALID_CONNMODE);
        }

        if (! IsLibInited())
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "library");
        }
      
        pHandle = (EpsHandleT*)calloc(1, sizeof(EpsHandleT));
        if (pHandle == NULL)
        {
            THROW_ERROR(ERCD_EPS_OPERSYSTEM_ERROR, strerror(errno));
        }
        pHandle->connMode = mode;

        pHandle->hid = g_atomic_int_exchange_and_add((gint*)&g_maxHid, 1);
        if (mode == EPS_CONNMODE_UDP)
        {
            EpsUdpDriverT* pDriver = &pHandle->driver.udpDriver;
            pDriver->hid = pHandle->hid;
            THROW_ERROR(InitUdpDriver(pDriver));
        }
        else /* mode == EPS_CONNMODE_TCP */
        {
            EpsTcpDriverT* pDriver = &pHandle->driver.tcpDriver;
            pDriver->hid = pHandle->hid;
            THROW_ERROR(InitTcpDriver(pDriver));
        }

        g_static_rec_mutex_lock(&g_libLock);
        g_hash_table_insert(g_handlePool, (void*)(&pHandle->hid), pHandle);
        g_static_rec_mutex_unlock(&g_libLock);

        *pHid = pHandle->hid;
    }
    CATCH
    {
        if (pHandle != NULL)
        {
            DestroyHandle(pHandle);
        }
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * 销毁句柄
 *
 * @param   hid             in  - 待销毁的句柄ID
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT EpsDestroyHandle(uint32 hid)
{
    TRY
    {
        if (! IsLibInited())
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "library");
        }

        g_static_rec_mutex_lock(&g_libLock);
        EpsHandleT* pHandle = (EpsHandleT*)g_hash_table_lookup(g_handlePool, (gconstpointer)&hid);
        if (pHandle != NULL)
        {
            g_hash_table_remove(g_handlePool, (gconstpointer)&hid);
        }
        g_static_rec_mutex_unlock(&g_libLock);

        if (pHandle == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_HID);
        }
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * 注册用户回调接口
 *
 * @param   hid             in  - 待执行注册操作的句柄ID
 * @param   pSpi            in  - 待执行注册的用户回调接口
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT EpsRegisterSpi(uint32 hid, const EpsClientSpiT* pSpi)
{
    TRY
    {
        if (pSpi == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pSpi");
        }

        if (! IsLibInited())
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "library");
        }

        g_static_rec_mutex_lock(&g_libLock);
        EpsHandleT* pHandle = (EpsHandleT*)g_hash_table_lookup(g_handlePool, (gconstpointer)&hid);
        g_static_rec_mutex_unlock(&g_libLock);

        if (pHandle == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_HID);
        }

        if (pHandle->connMode == EPS_CONNMODE_UDP)
        {
            EpsUdpDriverT* pDriver = &pHandle->driver.udpDriver;
            THROW_ERROR(RegisterUdpDriverSpi(pDriver, pSpi));
        }
        else /* connMode == EPS_CONNMODE_TCP */
        {
            EpsTcpDriverT* pDriver = &pHandle->driver.tcpDriver;
            THROW_ERROR(RegisterTcpDriverSpi(pDriver, pSpi));
        }
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * 连接服务器
 *
 * @param   hid             in  - 待执行连接操作的句柄ID
 * @param   address         in  - 主机地址字符串，例如
 *                                TCP: 196.123.1.1:8000
 *                                UDP: 230.11.1.1:3333;196.123.71.1
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT EpsConnect(uint32 hid, const char* address)
{
    TRY
    {
        if (address == NULL || address[0] == 0x00)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "address");
        }
        
        if (! IsLibInited())
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "library");
        }

        g_static_rec_mutex_lock(&g_libLock);
        EpsHandleT* pHandle = (EpsHandleT*)g_hash_table_lookup(g_handlePool, (gconstpointer)&hid);
        g_static_rec_mutex_unlock(&g_libLock);
        
        if (pHandle == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_HID);
        }

        if (pHandle->connMode == EPS_CONNMODE_UDP)
        {
            EpsUdpDriverT* pDriver = &pHandle->driver.udpDriver;
            THROW_ERROR(ConnectUdpDriver(pDriver, address));
        }
        else /* connMode == EPS_CONNMODE_TCP */
        {
            EpsTcpDriverT* pDriver = &pHandle->driver.tcpDriver;
            THROW_ERROR(ConnectTcpDriver(pDriver, address));
        }
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * 断开连接服务器
 *
 * @param   hid             in  - 待执行断开连接操作的句柄ID
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT EpsDisconnect(uint32 hid)
{
    TRY
    {
        if (! IsLibInited())
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "library");
        }

        g_static_rec_mutex_lock(&g_libLock);
        EpsHandleT* pHandle = (EpsHandleT*)g_hash_table_lookup(g_handlePool, (gconstpointer)&hid);
        g_static_rec_mutex_unlock(&g_libLock);

        if (pHandle == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_HID);
        }

        if (pHandle->connMode == EPS_CONNMODE_UDP)
        {
            EpsUdpDriverT* pDriver = &pHandle->driver.udpDriver;
            THROW_ERROR(DisconnectUdpDriver(pDriver));
        }
        else /* connMode == EPS_CONNMODE_TCP */
        {
            EpsTcpDriverT* pDriver = &pHandle->driver.tcpDriver;
            THROW_ERROR(DisconnectTcpDriver(pDriver));
        }
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * 登陆服务器
 *
 * @param   hid             in  - 待执行登陆操作的句柄ID
 * @param   username        in  - 登陆用户名
 * @param   password        in  - 登录密码
 * @param   hearbeatIntl    in  - 心跳间隔
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT EpsLogin(uint32 hid, const char* username, const char* password, uint16 heartbeatIntl)
{
    TRY
    {
        if (username == NULL || username[0] == 0x00)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "username");
        }

        if (password == NULL || password[0] == 0x00)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "password");
        }

        if (heartbeatIntl == 0)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, heartbeatIntl);
        }

        if (! IsLibInited())
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "library");
        }

        g_static_rec_mutex_lock(&g_libLock);
        EpsHandleT* pHandle = (EpsHandleT*)g_hash_table_lookup(g_handlePool, (gconstpointer)&hid);
        g_static_rec_mutex_unlock(&g_libLock);

        if (pHandle == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_HID);
        }

        if (pHandle->connMode == EPS_CONNMODE_UDP)
        {
            EpsUdpDriverT* pDriver = &pHandle->driver.udpDriver;
            THROW_ERROR(LoginUdpDriver(pDriver, username, password, heartbeatIntl));
        }
        else /* connMode == EPS_CONNMODE_TCP */
        {
            EpsTcpDriverT* pDriver = &pHandle->driver.tcpDriver;
            THROW_ERROR(LoginTcpDriver(pDriver, username, password, heartbeatIntl));
        }
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * 登出服务器
 *
 * @param   hid             in  - 待执行登出操作的句柄ID
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT EpsLogout(uint32 hid, const char* reason)
{
    TRY
    {
        if (reason == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "reason");
        }

        if (! IsLibInited())
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "library");
        }

        g_static_rec_mutex_lock(&g_libLock);
        EpsHandleT* pHandle = (EpsHandleT*)g_hash_table_lookup(g_handlePool, (gconstpointer)&hid);
        g_static_rec_mutex_unlock(&g_libLock);
        
        if (pHandle == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_HID);
        }

        if (pHandle->connMode == EPS_CONNMODE_UDP)
        {
            EpsUdpDriverT* pDriver = &pHandle->driver.udpDriver;
            THROW_ERROR(LogoutUdpDriver(pDriver, reason));
        }
        else /* connMode == EPS_CONNMODE_TCP */
        {
            EpsTcpDriverT* pDriver = &pHandle->driver.tcpDriver;
            THROW_ERROR(LogoutTcpDriver(pDriver, reason));
        }
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * 订阅指定市场类型的行情数据
 *
 * @param   hid             in  - 待执行订阅操作的句柄ID
 * @param   mktType         in  - 待订阅的市场类型
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT EpsSubscribeMarketData(uint32 hid, EpsMktTypeT mktType)
{
    TRY
    {
        if (! IsLibInited())
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "library");
        }

        g_static_rec_mutex_lock(&g_libLock);
        EpsHandleT* pHandle = (EpsHandleT*)g_hash_table_lookup(g_handlePool, (gconstpointer)&hid);
        g_static_rec_mutex_unlock(&g_libLock);

        if (pHandle == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_HID);
        }

        if (pHandle->connMode == EPS_CONNMODE_UDP)
        {
            EpsUdpDriverT* pDriver = &pHandle->driver.udpDriver;
            THROW_ERROR(SubscribeUdpDriver(pDriver, mktType));
        }
        else /* connMode == EPS_CONNMODE_TCP */
        {
            EpsTcpDriverT* pDriver = &pHandle->driver.tcpDriver;
            THROW_ERROR(SubscribeTcpDriver(pDriver, mktType));
        }
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * 获取最后一条错误信息
 *
 * @return  返回错误信息地址
 */
const char* GetLastError()
{
    return ErrGetErrorDscr();
}

/**
 * 判断库是否被初始化
 *
 * @return 已经初始化返回TRUE，否则返回FALSE
 */
static BOOL IsLibInited()
{
    return (g_atomic_int_get(&g_isLibInited) == TRUE);
}

/**
 * 断开句柄
 *
 * @param   pHandle             in  - 待执行断开操作的句柄ID
 */
static void DisconnectHandle(EpsHandleT* pHandle)
{
    if (pHandle->connMode == EPS_CONNMODE_UDP)
    {
        EpsUdpDriverT* pDriver = &pHandle->driver.udpDriver;
        DisconnectUdpDriver(pDriver);
    }
    else /* connMode == EPS_CONNMODE_TCP */
    {
        EpsTcpDriverT* pDriver = &pHandle->driver.tcpDriver;
        DisconnectTcpDriver(pDriver);
    }
}

/**
 * 销毁句柄
 *
 * @param   pHandle             in  - 待执行销毁操作的句柄ID
 */
static void DestroyHandle(EpsHandleT* pHandle)
{
    if (pHandle->connMode == EPS_CONNMODE_UDP)
    {
        EpsUdpDriverT* pDriver = &pHandle->driver.udpDriver;
        UninitUdpDriver(pDriver);
    }
    else /* connMode == EPS_CONNMODE_TCP */
    {
        EpsTcpDriverT* pDriver = &pHandle->driver.tcpDriver;
        UninitTcpDriver(pDriver);
    }

    free(pHandle);
}

/**
 * 释放句柄(用于GHashTable自动调用)
 *
 * @param   data             in  - 待释放的句柄ID
 */
static void _disposeHandle(gpointer data)
{
    EpsHandleT* pHandle = (EpsHandleT*)data;

    DisconnectHandle(pHandle);
    DestroyHandle(pHandle);
}


