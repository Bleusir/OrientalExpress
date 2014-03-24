/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    udpDriver.c
 *
 * UDP行情驱动器实现文件
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
14-FEB-2014 ZHENGWU         创建
================================================================================
</pre>
*/

/**
 * 包含头文件
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "step/stepCodec.h"
#include "udpDriver.h"


/**
 * 类型定义
 */

/*
 * UDP事件类型枚举
 */
typedef enum EpsUdpEventTypeTag
{
    EPS_UDP_EVENTTYPE_LOGIN         = 1,    /* 登陆事件 */
    EPS_UDP_EVENTTYPE_LOGOUT        = 2,    /* 登出事件 */
    EPS_UDP_EVENTTYPE_SUBSCRIBED    = 3,    /* 订阅事件 */
} EpsUdpEventTypeT;


/**
 * 内部函数申明
 */

static void OnChannelConnected(void* pListener);
static void OnChannelDisconnected(void* pListener, ResCodeT result, const char* reason);
static void OnChannelReceived(void* pListener, ResCodeT rc, const char* data, uint32 dataLen);
static void OnChannelEventOccurred(void* pListener, EpsUdpChannelEventT* pEvent);

static void OnEpsConnected(uint32 hid);
static void OnEpsDisconnected(uint32 hid, ResCodeT result, const char* reason);
static void OnEpsLoginRsp(uint32 hid, uint16 heartbeatIntl, ResCodeT result, const char* reason);
static void OnEpsLogoutRsp(uint32 hid, ResCodeT result, const char* reason);
static void OnEpsMktDataSubRsp(uint32 hid, EpsMktTypeT mktType, ResCodeT result, const char* reason);
static void OnEpsMktDataArrived(uint32 hid, const EpsMktDataT* pMktData);
static void OnEpsEventOccurred(uint32 hid, EpsEventTypeT eventType, ResCodeT eventCode, const char* eventText);

static ResCodeT ParseAddress(const char* address, char* mcAddr, uint16* mcPort, char* localAddr);


/**
 * 函数实现
 */

/**
 *  初始化UDP驱动器
 *
 * @param   pDriver             in  - UDP驱动器
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT InitUdpDriver(EpsUdpDriverT* pDriver)
{
    TRY
    {
        THROW_ERROR(InitUdpChannel(&pDriver->channel));
        THROW_ERROR(InitMktDatabase(&pDriver->database));
        
        EpsUdpChannelListenerT listener =
        {
            pDriver,
            OnChannelConnected,
            OnChannelDisconnected,
            OnChannelReceived,
            OnChannelEventOccurred
        };
        THROW_ERROR(RegisterUdpChannelListener(&pDriver->channel, &listener));

        EpsClientSpiT spi =
        {
            OnEpsConnected,
            OnEpsDisconnected,
            OnEpsLoginRsp,
            OnEpsLogoutRsp,
            OnEpsMktDataSubRsp,
            OnEpsMktDataArrived,
            OnEpsEventOccurred
        };
        pDriver->spi = spi;

        g_static_rec_mutex_init(&pDriver->lock);
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/*
 * 反初始化UDP驱动器
 *
 * @param   pDriver             in  - UDP驱动器
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT UninitUdpDriver(EpsUdpDriverT* pDriver)
{
    TRY
    {
        g_static_rec_mutex_lock(&pDriver->lock);
            
        UninitUdpChannel(&pDriver->channel);
        UninitMktDatabase(&pDriver->database);

        g_static_rec_mutex_unlock(&pDriver->lock);
 
        g_static_rec_mutex_free(&pDriver->lock);
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
 *  注册UDP驱动器回调函数集
 *
 * @param   pDriver             in  - UDP驱动器
 * @param   pSpi                in  - 用户回调接口
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT RegisterUdpDriverSpi(EpsUdpDriverT* pDriver, const EpsClientSpiT* pSpi)
{
    TRY
    {
        g_static_rec_mutex_lock(&pDriver->lock);

        if (pSpi->connectedNotify != NULL)
        {
            pDriver->spi.connectedNotify = pSpi->connectedNotify;
        }
        if (pSpi->disconnectedNotify != NULL)
        {
            pDriver->spi.disconnectedNotify = pSpi->disconnectedNotify;
        }
        if (pSpi->loginRspNotify != NULL)
        {
            pDriver->spi.loginRspNotify = pSpi->loginRspNotify;
        }
        if (pSpi->logoutRspNotify != NULL)
        {
            pDriver->spi.logoutRspNotify = pSpi->logoutRspNotify;
        }
        if (pSpi->mktDataSubRspNotify != NULL)
        {
            pDriver->spi.mktDataSubRspNotify = pSpi->mktDataSubRspNotify;
        }
        if (pSpi->mktDataArrivedNotify != NULL)
        {
            pDriver->spi.mktDataArrivedNotify = pSpi->mktDataArrivedNotify;
        }
        if (pSpi->eventOccuredNotify != NULL)
        {
            pDriver->spi.eventOccuredNotify = pSpi->eventOccuredNotify;
        }
    }
    CATCH
    {
    }
    FINALLY
    {
        g_static_rec_mutex_unlock(&pDriver->lock);

        RETURN_RESCODE;
    }
}

/**
 * 建立UDP服务器连接
 *
 * @param   pDriver             in  - UDP驱动器
 * @param   address             in  - 连接地址
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT ConnectUdpDriver(EpsUdpDriverT* pDriver, const char* address)
{
    TRY
    {
        g_static_rec_mutex_lock(&pDriver->lock);
   
        THROW_ERROR(ParseAddress(address, pDriver->channel.mcAddr, 
            &pDriver->channel.mcPort, pDriver->channel.localAddr));
        THROW_ERROR(StartupUdpChannel(&pDriver->channel));
    }
    CATCH
    {
    }
    FINALLY
    {
        g_static_rec_mutex_unlock(&pDriver->lock);

        RETURN_RESCODE;
    }
}

/**
 * 断开UDP服务器连接
 *
 * @param   pDriver             in  - UDP驱动器
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT DisconnectUdpDriver(EpsUdpDriverT* pDriver)
{
    TRY
    {
        g_static_rec_mutex_lock(&pDriver->lock);
        THROW_ERROR(ShutdownUdpChannel(&pDriver->channel));
    }
    CATCH
    {
    }
    FINALLY
    {
        g_static_rec_mutex_unlock(&pDriver->lock);
 
        RETURN_RESCODE;
    }
}

/**
 * 登陆UDP驱动器 
 * 
 * @param   pDriver             in  - UDP驱动器
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT LoginUdpDriver(EpsUdpDriverT* pDriver, const char* username, 
    const char* password, uint16 heartbeatIntl)
{
    TRY
    {
        g_static_rec_mutex_lock(&pDriver->lock);

        EpsUdpChannelEventT event = 
        {
            EPS_UDP_EVENTTYPE_LOGIN, 0
        };

        THROW_ERROR(TriggerUdpChannelEvent(&pDriver->channel, event));
    }
    CATCH
    {
    }
    FINALLY
    {
        g_static_rec_mutex_unlock(&pDriver->lock);

        RETURN_RESCODE;
    }
}

/*
 * 登出UDP驱动器
 *
 * @param   pDriver             in  - UDP驱动器
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT LogoutUdpDriver(EpsUdpDriverT* pDriver, const char* reason)
{
    TRY
    {
        g_static_rec_mutex_lock(&pDriver->lock);

        EpsUdpChannelEventT event = 
        {
            EPS_UDP_EVENTTYPE_LOGOUT, 0
        };

        THROW_ERROR(TriggerUdpChannelEvent(&pDriver->channel, event));
    }
    CATCH
    {
    }
    FINALLY
    {
        g_static_rec_mutex_unlock(&pDriver->lock);

        RETURN_RESCODE;
    }
}

/**
 * 订阅UDP驱动器
 *
 * @param   pDriver             in  - UDP驱动器
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT SubscribeUdpDriver(EpsUdpDriverT* pDriver, EpsMktTypeT mktType)
{
    TRY
    {
        g_static_rec_mutex_lock(&pDriver->lock);

        THROW_ERROR(SubscribeMktData(&pDriver->database, mktType));

        EpsUdpChannelEventT event = 
        {
            EPS_UDP_EVENTTYPE_SUBSCRIBED, (uint32)mktType
        };
        THROW_ERROR(TriggerUdpChannelEvent(&pDriver->channel, event));
    }
    CATCH
    {
    }
    FINALLY
    {
        g_static_rec_mutex_unlock(&pDriver->lock);

        RETURN_RESCODE;
    }
}

/**
 * UDP通道连接成功通知
 *
 * @param   pListener             in  - UDP驱动器
 */
static void OnChannelConnected(void* pListener)
{
    EpsUdpDriverT* pDriver = (EpsUdpDriverT*)pListener;
    pDriver->spi.connectedNotify(pDriver->hid);
}

/**
 * UDP通道连接断开通知
 *
 * @param   pListener           in  - UDP驱动器
 * @param   result              in  - 断开错误码
 * @param   reason              in  - 断开原因描述
 */
static void OnChannelDisconnected(void* pListener, ResCodeT result, const char* reason)
{
    EpsUdpDriverT* pDriver = (EpsUdpDriverT*)pListener;
    UnSubscribeAllMktData(&pDriver->database);
    pDriver->spi.disconnectedNotify(pDriver->hid, result, reason);
}

/**
 * UDP通道数据接收通知
 *
 * @param   pListener           in  - UDP驱动器
 * @param   result              in  - 接收错误码
 * @param   data                in  - 接收数据
 * @param   dataLen             in  - 接收数据长度
 */
static void OnChannelReceived(void* pListener, ResCodeT result, const char* data, uint32 dataLen)
{
    EpsUdpDriverT* pDriver = (EpsUdpDriverT*)pListener;

    ResCodeT rc = NO_ERR;
    
    StepMessageT msg;
    int32 decodeSize = 0;
    rc = DecodeStepMessage(data, dataLen, &msg, &decodeSize);
    if (NOTOK(rc))
    {
        pDriver->spi.eventOccuredNotify(pDriver->hid, EPS_EVENTTYPE_WARNING, rc, ErrGetErrorDscr());
    }

    if (msg.msgType != STEP_MSGTYPE_MD_SNAPSHOT)
    {
        return;
    }

    rc = AcceptMktData(&pDriver->database, &msg);
    if (NOTOK(rc))
    {
        if (rc == ERCD_EPS_DATASOURCE_CHANGED)
        {
            pDriver->spi.eventOccuredNotify(pDriver->hid, EPS_EVENTTYPE_WARNING, rc, ErrGetErrorDscr());
        }
        else 
        {
            return;
        }
    }

    EpsMktDataT mktData;
    rc = ConvertMktData(&msg, &mktData);
    if (NOTOK(rc))
    {
        pDriver->spi.eventOccuredNotify(pDriver->hid, EPS_EVENTTYPE_WARNING, rc, ErrGetErrorDscr());
    }

    pDriver->spi.mktDataArrivedNotify(pDriver->hid, &mktData);
}

/**
 * UDP通道数据事件通知
 *
 * @param   pListener               in  - UDP驱动器
 * @param   pEvent                  in  - 事件对象
 */
static void OnChannelEventOccurred(void* pListener, EpsUdpChannelEventT* pEvent)
{
    EpsUdpDriverT* pDriver = (EpsUdpDriverT*)pListener;

    switch (pEvent->eventType)
    {
        case EPS_UDP_EVENTTYPE_LOGIN:
        {
            pDriver->spi.loginRspNotify(pDriver->hid, 
                    pDriver->heartbeatIntl, NO_ERR, "login succeed");
            break;
        }
        case EPS_UDP_EVENTTYPE_LOGOUT:
        {
            UnSubscribeAllMktData(&pDriver->database);
            pDriver->spi.logoutRspNotify(pDriver->hid, 
                    NO_ERR, "logout succeed");
            break;
        }
        case EPS_UDP_EVENTTYPE_SUBSCRIBED:
        {
            pDriver->spi.mktDataSubRspNotify(pDriver->hid, 
                    (EpsMktTypeT)(pEvent->eventParam), NO_ERR, "subscribe succeed");
            break;
        }
        default:
            break;
    }
}

/**
 * 解析地址字符串
 *
 * @param   address                 in  - 地址字符串
 * @param   mcAddr                  out - 组播地址
 * @param   mcPort                  out - 组播端口
 * @param   localAddr               out - 本地地址
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT ParseAddress(const char* address, char* mcAddr, uint16* mcPort, char* localAddr)
{
    TRY
    {
        /* 字符串格式为230.11.1.1:3333;196.123.71.1 */
        const char* p1, *p2;

        int len = strlen(address);
        
        p1 = strstr(address, ":");
        if (p1 == NULL || p1 == address)
        {
            THROW_ERROR(ERCD_EPS_INVALID_ADDRESS);
        }

        p2 = strstr(p1+1, ";");
        if (p2 == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_ADDRESS);
        }

        memcpy(mcAddr, address, (p1-address));
        memcpy(localAddr, p2+1, (address+len-p2-1));
        *mcPort = atoi(p1 + 1);
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/*
 * Express SPI占位函数
 */
static void OnEpsConnected(uint32 hid)
{
}
static void OnEpsDisconnected(uint32 hid, ResCodeT result, const char* reason)
{
}
static void OnEpsLoginRsp(uint32 hid, uint16 heartbeatIntl, ResCodeT result, const char* reason)
{
}
static void OnEpsLogoutRsp(uint32 hid, ResCodeT result, const char* reason)
{
}
static void OnEpsMktDataSubRsp(uint32 hid, EpsMktTypeT mktType, ResCodeT result, const char* reason)
{
}
static void OnEpsMktDataArrived(uint32 hid, const EpsMktDataT* pMktData)
{
}
static void OnEpsEventOccurred(uint32 hid, EpsEventTypeT eventType, ResCodeT eventCode, const char* eventText)
{
}

