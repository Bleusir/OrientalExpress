/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    udpChannel.c
 *
 * UDP组播通道实现文件
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
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#include "eps/epsTypes.h"
#include "cmn/errlib.h"
#include "udpChannel.h"


/**
 * 内部函数声明
 */

static void* ChannelTask(void* arg);

static ResCodeT OpenChannel(EpsUdpChannelT* pChannel);
static ResCodeT CloseChannel(EpsUdpChannelT* pChannel);
static ResCodeT HandleEvent(EpsUdpChannelT* pChannel);
static ResCodeT ReceiveData(EpsUdpChannelT* pChannel);
static ResCodeT ClearEventQueue(EpsUdpChannelT* pChannel);

static BOOL IsChannelInited(EpsUdpChannelT * pChannel);
static BOOL IsChannelStarted(EpsUdpChannelT* pChannel);
static BOOL IsChannelConnected(EpsUdpChannelT * pChannel);

static void OnChannelConnected(void* pListener);
static void OnChannelDisconnected(void* pListener, ResCodeT result, const char* reason);
static void OnChannelReceived(void* pListener, ResCodeT result, const char* data, uint32 dataLen);
static void OnChannelEventOccurred(void* pListener, EpsUdpChannelEventT* pEvent);


/**
 * 函数实现
 */

/**
 * 初始化UDP通道
 *
 * @param   pChannel            in  - 通道对象
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT InitUdpChannel(EpsUdpChannelT* pChannel)
{
    TRY
    {
        if (pChannel == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pChannel");
        }

        if (IsChannelInited(pChannel))
        {
            THROW_ERROR(ERCD_EPS_DUPLICATE_INITED, "channel");
        }

        pChannel->socket = -1;
        pChannel->pEventQueue = g_async_queue_new();
        pChannel->canStop = FALSE;

        EpsUdpChannelListenerT listener = 
        {
            NULL,
            OnChannelConnected,
            OnChannelDisconnected,
            OnChannelReceived,
            OnChannelEventOccurred
        };
        pChannel->listener = listener;
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
 * 反初始化UDP通道
 *
 * @param   pChannel            in  - UDP通道对象
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT UninitUdpChannel(EpsUdpChannelT* pChannel)
{
    TRY
    {
        if (pChannel == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pChannel");
        }

        if (! IsChannelInited(pChannel))
        {
            THROW_RESCODE(NO_ERR);
        } 

        if (pChannel->pEventQueue != NULL)
        {
            ClearEventQueue(pChannel);

            g_async_queue_unref(pChannel->pEventQueue);
            pChannel->pEventQueue = NULL;
        }

        if (pChannel->socket != -1)
        {
            close(pChannel->socket);
            pChannel->socket = -1;
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
 * 启动UDP通道
 *
 * @param   pChannel            in  - UDP通道对象
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT StartupUdpChannel(EpsUdpChannelT* pChannel)
{
    TRY
    {
        if (pChannel == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pChannel");
        }

        if (IsChannelStarted(pChannel))
        {
            THROW_ERROR(ERCD_EPS_DUPLICATE_CONNECT);
        }
       
        pChannel->canStop = FALSE;
        
        pthread_t tid;
        int result = pthread_create(&tid, NULL, ChannelTask, (void*)pChannel);
        if (result != 0)
        {
            THROW_ERROR(ERCD_EPS_OPERSYSTEM_ERROR, strerror(result));
        }
        pChannel->tid = tid;
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
 * 停止UDP通道
 *
 * @param   pChannel            in  - UDP通道对象
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT ShutdownUdpChannel(EpsUdpChannelT* pChannel)
{
    TRY
    {
        if (pChannel == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pChannel");
        }

        if (! IsChannelStarted(pChannel))
        {
            THROW_RESCODE(NO_ERR);
        }

        pChannel->canStop = TRUE;
        int result = pthread_join(pChannel->tid, NULL);
        if (result != 0)
        {
            THROW_ERROR(ERCD_EPS_OPERSYSTEM_ERROR, strerror(result));
        }
        pChannel->tid = 0;
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
 * 注册UDP通道监听者接口
 *
 * @param   pChannel            in  - UDP通道对象
 * @param   pListener           in  - 待注册的通道监听者
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT RegisterUdpChannelListener(EpsUdpChannelT* pChannel, const EpsUdpChannelListenerT* pListener)
{
    TRY
    {
        if (pChannel == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pChannel");
        }

        if (pListener == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pListener");
        }

        if(! IsChannelInited(pChannel))
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "channel");
        }

        pChannel->listener.pListener = pListener->pListener;
        if (pListener->connectedNotify != NULL)
        {
            pChannel->listener.connectedNotify = pListener->connectedNotify;
        }
        if (pListener->disconnectedNotify != NULL)
        {
            pChannel->listener.disconnectedNotify = pListener->disconnectedNotify;
        }
        if (pListener->receivedNotify != NULL)
        {
            pChannel->listener.receivedNotify = pListener->receivedNotify;
        }
        if (pListener->eventOccurredNotify != NULL)
        {
            pChannel->listener.eventOccurredNotify = pListener->eventOccurredNotify;
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


/*
 * 触发UDP通道异步事件
 *
 * @param   pChannel            in  - UDP通道对象
 * @param   pEvent              in  - 事件对象
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT TriggerUdpChannelEvent(EpsUdpChannelT* pChannel, EpsUdpChannelEventT* pEvent)
{
    TRY
    {
        if (pChannel == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pChannel");
        }

        if (pEvent == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pEvent");
        }

        if(! IsChannelInited(pChannel))
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "channel");
        }

        g_async_queue_push(pChannel->pEventQueue, (gpointer)pEvent);
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
 * UDP通道任务线程函数
 *
 * @param   arg                 in  - 线程参数(UDP通道对象)
 */
static void* ChannelTask(void* arg)
{
    EpsUdpChannelT* pChannel = (EpsUdpChannelT*)arg;

    while (! pChannel->canStop)
    {
        /* 打开UDP通道 */
        if (! IsChannelConnected(pChannel))
        {
            if (NOTOK(OpenChannel(pChannel)))
            {
                pChannel->listener.disconnectedNotify(pChannel->listener.pListener,
                    ErrGetErrorCode(), ErrGetErrorDscr());
                    
                usleep(EPS_UDPCHANNEL_RECONNECT_INTERVAL * 1000);
                continue;
            }
            else
            {
                pChannel->listener.connectedNotify(pChannel->listener.pListener);
            }
        }
        
        /* 优先处理异步事件 */
        HandleEvent(pChannel);

        /* 处理网络接收数据 */
        ReceiveData(pChannel);
    }
    
    CloseChannel(pChannel);

    return 0;
}

/**
 * 打开UDP通道
 *
 * @param   pChannel            in  - UDP通道对象
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT OpenChannel(EpsUdpChannelT* pChannel)
{
    int fd = -1;
    TRY
    {
        int result = -1;
      
        fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd == -1)
        {
            THROW_ERROR(ERCD_EPS_SOCKET_ERROR, strerror(errno));
        }
   
        BOOL reuseaddr = TRUE;
        result = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseaddr, sizeof(reuseaddr));
        if (result == -1)
        {
            THROW_ERROR(ERCD_EPS_SOCKET_ERROR, strerror(errno));
        }

        struct sockaddr_in mcAddr;
        memset(&mcAddr, 0x00, sizeof(mcAddr));
            
        mcAddr.sin_family      = AF_INET;
        mcAddr.sin_addr.s_addr = inet_addr(pChannel->mcAddr);
        mcAddr.sin_port        = htons(pChannel->mcPort);

        result = bind(fd, (struct sockaddr*)&mcAddr, sizeof(mcAddr));
        if (result == -1)
        {
            THROW_ERROR(ERCD_EPS_SOCKET_ERROR, strerror(errno));
        }   

        struct ip_mreq mreq = { {inet_addr(pChannel->mcAddr)}, {inet_addr(pChannel->localAddr)} };
        result = setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
        if (result == -1)
        {
            THROW_ERROR(ERCD_EPS_SOCKET_ERROR, strerror(errno));
        }

        int flags = fcntl(fd, F_GETFL, 0);
        result = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
        if (result == -1)
        {
            THROW_ERROR(ERCD_EPS_OPERSYSTEM_ERROR, strerror(errno));
        }

        pChannel->socket = fd;
    }
    CATCH
    {
        if (fd != -1)
        {
            close(fd);
            fd = -1;
        }
    }
    FINALLY
    {
        RETURN_RESCODE;      
    }
}


/**
 * 关闭UDP通道
 *
 * @param   pChannel            in  - UDP通道对象
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT CloseChannel(EpsUdpChannelT* pChannel)
{
    TRY
    {
        if (pChannel->socket != -1)
        {
            close(pChannel->socket);

            pChannel->socket = -1;
        }

        ClearEventQueue(pChannel);
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
 * 处理通道事件
 *
 * @param   pChannel            in  - UDP通道对象
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT HandleEvent(EpsUdpChannelT* pChannel)
{
    TRY
    {
        EpsUdpChannelEventT* pEvent = 
            (EpsUdpChannelEventT*)g_async_queue_try_pop_unlocked(pChannel->pEventQueue);

        pChannel->listener.eventOccurredNotify(pChannel->listener.pListener, pEvent);
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
 * 接收通道数据
 *
 * @param   pChannel            in  - UDP通道对象
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT ReceiveData(EpsUdpChannelT* pChannel)
{
    TRY
    {
        struct sockaddr_in srcAddr;
        socklen_t addrlen = sizeof(srcAddr);

        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(pChannel->socket, &fdset);

        struct timeval timeout;
        timeout.tv_sec = EPS_UDPCHANNEL_RECV_TIMEOUT / 1000;
        timeout.tv_usec = (EPS_UDPCHANNEL_RECV_TIMEOUT % 1000) * 1000;

        int result = select(pChannel->socket+1, &fdset, 0, 0, &timeout);
        if (result == -1)
        {
            THROW_ERROR(ERCD_EPS_SOCKET_ERROR, strerror(errno));
        }
        
        if (FD_ISSET(pChannel->socket, &fdset))
        {
            int len = recvfrom(pChannel->socket, pChannel->recvBuffer, EPS_UDPCHANNEL_RECVBUFFER_LEN, 
                0, (struct sockaddr*)&srcAddr, &addrlen);
            if (len == -1)
            {
                THROW_ERROR(ERCD_EPS_SOCKET_ERROR, strerror(errno));
            }
            else if (len == 0)
            {
                CloseChannel(pChannel);
            }
            else
            {
                pChannel->listener.receivedNotify(pChannel->listener.pListener, 
                        NO_ERR, pChannel->recvBuffer, (uint32)len);
            }
        }
    }
    CATCH
    {
        CloseChannel(pChannel);
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * 清除事件队列
 *
 * @param   pChannel            in  - UDP通道对象
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT ClearEventQueue(EpsUdpChannelT* pChannel)
{
    TRY
    {
        EpsUdpChannelEventT* pEvent = NULL;
        while (TRUE)
        {
            pEvent = (EpsUdpChannelEventT*)g_async_queue_try_pop_unlocked(pChannel->pEventQueue);
            if (pEvent != NULL)
            {
                free(pEvent);
            }
            else
            {
                break;
            }
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
 * 判断通道是否初始化
 *
 * @param   pChannel            in  - UDP通道对象
 *
 * @return  已经初始化返回TRUE，否则返回FALSE
 */
static BOOL IsChannelInited(EpsUdpChannelT* pChannel)
{
    return (pChannel->pEventQueue != NULL);
}

/**
 * 判断通道是否启动
 *
 * @param   pChannel            in  - UDP通道对象
 *
 * @return  已经启动返回TRUE，否则返回FALSE
 */
static BOOL IsChannelStarted(EpsUdpChannelT* pChannel)
{
    return (pChannel->tid != 0);
}

/**
 * 判断通道是否连接成功
 *
 * @param   pChannel            in  - UDP通道对象
 *
 * @return  已经连接成功返回TRUE，否则返回FALSE
 */
static BOOL IsChannelConnected(EpsUdpChannelT * pChannel)
{
    return (pChannel->socket != -1);
}

/*
 * 通道监听回调占位函数
 */
static void OnChannelConnected(void* pListener)
{
}
static void OnChannelDisconnected(void* pListener, ResCodeT result, const char* reason)
{
}
static void OnChannelReceived(void* pListener, ResCodeT result, const char* data, uint32 dataLen)
{
}
static void OnChannelEventOccurred(void* pListener, EpsUdpChannelEventT* pEvent)
{
}