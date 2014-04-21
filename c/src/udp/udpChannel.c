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
 * �궨��
 */

#define EPS_EVENTQUEUE_SIZE                      128


/**
 * �ڲ���������
 */

static void* ChannelTask(void* arg);

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
        if (IsChannelInited(pChannel))
        {
            THROW_ERROR(ERCD_EPS_DUPLICATE_INITED, "channel");
        }

        pChannel->socket = -1;
        pChannel->tid = 0;
        pChannel->canStop = TRUE;
        pChannel->status  = EPS_UDPCHANNEL_STATUS_STOP;
        InitUniQueue(&pChannel->eventQueue, EPS_EVENTQUEUE_SIZE);
 
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
        if (! IsChannelInited(pChannel))
        {
            THROW_RESCODE(NO_ERR);
        } 

        if (pChannel->socket != -1)
        {
            shutdown(pChannel->socket, SHUT_RDWR);
            close(pChannel->socket);
            pChannel->socket = -1;
        }

        ClearEventQueue(pChannel);
        UninitUniQueue(&pChannel->eventQueue);
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
        if (IsChannelStarted(pChannel))
        {
            if (pChannel->status == EPS_UDPCHANNEL_STATUS_IDLE)
            {
                pChannel->status = EPS_UDPCHANNEL_STATUS_WORK;
                THROW_RESCODE(NO_ERR);
            }
            else
            {
                THROW_ERROR(ERCD_EPS_DUPLICATE_CONNECT);
            }
        }
       
        pChannel->canStop = FALSE;
        pChannel->status = EPS_UDPCHANNEL_STATUS_WORK;
        
        pthread_t tid;
        int result = pthread_create(&tid, NULL, ChannelTask, (void*)pChannel);
        if (result != 0)
        {
            pChannel->status = EPS_UDPCHANNEL_STATUS_STOP;
            
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
        if (! IsChannelStarted(pChannel))
        {
            THROW_RESCODE(NO_ERR);
        }

        if (pChannel->tid != pthread_self())
        {
            pChannel->canStop = TRUE;
        }
        else
        {
            if (pChannel->status == EPS_UDPCHANNEL_STATUS_WORK)
            {
                CloseUdpChannel(pChannel);
            }

            pChannel->status = EPS_UDPCHANNEL_STATUS_IDLE;
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
 * �ȴ�UDPͨ������
 *
 * @param   pChannel            in  - UDPͨ������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
 ResCodeT JoinUdpChannel(EpsUdpChannelT* pChannel)
{
    TRY
    {
        if (pChannel->tid != 0 && pChannel->tid != pthread_self())
        {
            int result = pthread_join(pChannel->tid, NULL);
            if (result != 0)
            {
                THROW_ERROR(ERCD_EPS_OPERSYSTEM_ERROR, strerror(result));
            }            

            pChannel->tid = 0;
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
 * @param   event               in  - �¼�����
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT TriggerUdpChannelEvent(EpsUdpChannelT* pChannel, const EpsUdpChannelEventT event)
{
    TRY
    {
        if(! IsChannelInited(pChannel))
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "channel");
        }

        EpsUdpChannelEventT* pEvent = (EpsUdpChannelEventT*)calloc(1, sizeof(EpsUdpChannelEventT));
        if (pEvent == NULL)
        {
            THROW_ERROR(ERCD_EPS_OPERSYSTEM_ERROR, strerror(errno));
        }
        *pEvent = event;

        THROW_ERROR(PushUniQueue(&pChannel->eventQueue, (void*)pEvent));
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
        if (pChannel->status == EPS_UDPCHANNEL_STATUS_IDLE)
        {
            usleep(EPS_CHANNEL_IDLE_INTL * 1000);
            continue;
        }

        /* ��UDPͨ�� */
        if (! IsChannelConnected(pChannel))
        {
            if (NOTOK(OpenUdpChannel(pChannel)))
            {
                pChannel->listener.disconnectedNotify(pChannel->listener.pListener,
                    ErrGetErrorCode(), ErrGetErrorDscr());

                ErrClearError();
          
                usleep(EPS_CHANNEL_RECONNECT_INTL * 1000);
                continue;
            }
            else
            {
                pChannel->listener.connectedNotify(pChannel->listener.pListener);
            }
        }
        
        /* ���ȴ����첽�¼� */
        if (NOTOK(HandleEvent(pChannel)))
        {
            ErrClearError();
         
            continue;
        }

        /* ��������������� */
        if (NOTOK(ReceiveData(pChannel)))
        {
            ErrClearError();
         
            continue;
        }
    }
    
    CloseUdpChannel(pChannel);

    pChannel->status = EPS_UDPCHANNEL_STATUS_STOP;
    
    return 0;
}

/**
 * ��UDPͨ��
 *
 * @param   pChannel            in  - UDPͨ������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT OpenUdpChannel(EpsUdpChannelT* pChannel)
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

        ClearEventQueue(pChannel);
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
ResCodeT CloseUdpChannel(EpsUdpChannelT* pChannel)
{
    TRY
    {
        if (pChannel->socket != -1)
        {
            shutdown(pChannel->socket, SHUT_RDWR);
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
        EpsUdpChannelEventT* pEvent = NULL;
        THROW_ERROR(PopUniQueue(&pChannel->eventQueue, (void**)&pEvent));
      
        if (pEvent != NULL)
        {
            pChannel->listener.eventOccurredNotify(pChannel->listener.pListener, pEvent);
            free(pEvent);
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
        timeout.tv_sec = EPS_SOCKET_RECV_TIMEOUT / 1000;
        timeout.tv_usec = (EPS_SOCKET_RECV_TIMEOUT % 1000) * 1000;

        int result = select(pChannel->socket+1, &fdset, 0, 0, &timeout);
        if (result == -1)
        {
            THROW_ERROR(ERCD_EPS_SOCKET_ERROR, strerror(errno));
        }
        
        if (FD_ISSET(pChannel->socket, &fdset))
        {
            int len = recvfrom(pChannel->socket, pChannel->recvBuffer, EPS_SOCKET_RECVBUFFER_LEN, 
                0, (struct sockaddr*)&srcAddr, &addrlen);
            if (len > 0)
            {
                pChannel->listener.receivedNotify(pChannel->listener.pListener, 
                        NO_ERR, pChannel->recvBuffer, (uint32)len);
            }
            else if(len == 0)
            {
                THROW_ERROR(ERCD_EPS_SOCKET_ERROR, "Connection closed by remote");
            }
            else
            {
                THROW_ERROR(ERCD_EPS_SOCKET_ERROR, strerror(errno));
            }
        }
        else
        {
            pChannel->listener.receivedNotify(pChannel->listener.pListener, 
                    ERCD_EPS_SOCKET_TIMEOUT, pChannel->recvBuffer, (uint32)0);
        }
    }
    CATCH
    {
        CloseUdpChannel(pChannel);
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
            THROW_ERROR(PopUniQueue(&pChannel->eventQueue, (void**)&pEvent));

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
    return (IsUniQueueInited(&pChannel->eventQueue));
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
