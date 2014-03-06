/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    udpChannel.c
 *
 * UDP�鲥ͨ��ʵ���ļ�
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
14-FEB-2014 ZHENGWU         ����
================================================================================
</pre>
*/

/**
 * ����ͷ�ļ�
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
 * �ڲ���������
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
 * ����ʵ��
 */

/**
 * ��ʼ��UDPͨ��
 *
 * @param   pChannel            in  - ͨ������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
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
 * ����ʼ��UDPͨ��
 *
 * @param   pChannel            in  - UDPͨ������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
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
 * ����UDPͨ��
 *
 * @param   pChannel            in  - UDPͨ������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
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
 * ֹͣUDPͨ��
 *
 * @param   pChannel            in  - UDPͨ������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
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
 * ע��UDPͨ�������߽ӿ�
 *
 * @param   pChannel            in  - UDPͨ������
 * @param   pListener           in  - ��ע���ͨ��������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
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
 * ����UDPͨ���첽�¼�
 *
 * @param   pChannel            in  - UDPͨ������
 * @param   pEvent              in  - �¼�����
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
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
 * UDPͨ�������̺߳���
 *
 * @param   arg                 in  - �̲߳���(UDPͨ������)
 */
static void* ChannelTask(void* arg)
{
    EpsUdpChannelT* pChannel = (EpsUdpChannelT*)arg;

    while (! pChannel->canStop)
    {
        /* ��UDPͨ�� */
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
        
        /* ���ȴ����첽�¼� */
        HandleEvent(pChannel);

        /* ��������������� */
        ReceiveData(pChannel);
    }
    
    CloseChannel(pChannel);

    return 0;
}

/**
 * ��UDPͨ��
 *
 * @param   pChannel            in  - UDPͨ������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
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
 * �ر�UDPͨ��
 *
 * @param   pChannel            in  - UDPͨ������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
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
 * ����ͨ���¼�
 *
 * @param   pChannel            in  - UDPͨ������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
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
 * ����ͨ������
 *
 * @param   pChannel            in  - UDPͨ������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
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
 * ����¼�����
 *
 * @param   pChannel            in  - UDPͨ������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
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
 * �ж�ͨ���Ƿ��ʼ��
 *
 * @param   pChannel            in  - UDPͨ������
 *
 * @return  �Ѿ���ʼ������TRUE�����򷵻�FALSE
 */
static BOOL IsChannelInited(EpsUdpChannelT* pChannel)
{
    return (pChannel->pEventQueue != NULL);
}

/**
 * �ж�ͨ���Ƿ�����
 *
 * @param   pChannel            in  - UDPͨ������
 *
 * @return  �Ѿ���������TRUE�����򷵻�FALSE
 */
static BOOL IsChannelStarted(EpsUdpChannelT* pChannel)
{
    return (pChannel->tid != 0);
}

/**
 * �ж�ͨ���Ƿ����ӳɹ�
 *
 * @param   pChannel            in  - UDPͨ������
 *
 * @return  �Ѿ����ӳɹ�����TRUE�����򷵻�FALSE
 */
static BOOL IsChannelConnected(EpsUdpChannelT * pChannel)
{
    return (pChannel->socket != -1);
}

/*
 * ͨ�������ص�ռλ����
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