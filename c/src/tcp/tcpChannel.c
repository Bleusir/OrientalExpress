/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    tcpChannel.c
 *
 * TCP�շ�ͨ��ʵ���ļ�
 *
 * @version $Id
 * @since   2014/03/11
 * @author  Wu Zheng
 */

/**
MODIFICATION HISTORY:
<pre>
================================================================================
DD-MMM-YYYY INIT.    SIR    Modification Description
----------- -------- ------ ----------------------------------------------------
11-MAR-2014 ZHENGWU         ����
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
#include "tcpChannel.h"


/**
 * �궨��
 */

#define EPS_SENDDATA_MAX_LEN                    8192


/** 
 * ��������
 */

/*
 * �������ݽṹ
 */
typedef struct EpsSendDataTag
{
    char    data[EPS_SENDDATA_MAX_LEN];
    uint32  dataLen;
} EpsSendDataT;


/**
 * �ڲ���������
 */

static void* ChannelTask(void* arg);

static ResCodeT OpenChannel(EpsTcpChannelT* pChannel);
static ResCodeT CloseChannel(EpsTcpChannelT* pChannel);
static ResCodeT SendData(EpsTcpChannelT* pChannel);
static ResCodeT ReceiveData(EpsTcpChannelT* pChannel);
static ResCodeT ClearSendQueue(EpsTcpChannelT* pChannel);

static BOOL IsChannelInited(EpsTcpChannelT * pChannel);
static BOOL IsChannelStarted(EpsTcpChannelT* pChannel);
static BOOL IsChannelConnected(EpsTcpChannelT * pChannel);

static void OnChannelConnected(void* pListener);
static void OnChannelDisconnected(void* pListener, ResCodeT result, const char* reason);
static void OnChannelReceived(void* pListener, ResCodeT result, const char* data, uint32 dataLen);
static void OnChannelSended(void* pListener, ResCodeT result, const char* data, uint32 dataLen);


/**
 * ����ʵ��
 */

/**
 * ��ʼ��TCPͨ��
 *
 * @param   pChannel            in  - TCPͨ������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT InitTcpChannel(EpsTcpChannelT* pChannel)
{
    TRY
    {
        if (IsChannelInited(pChannel))
        {
            THROW_ERROR(ERCD_EPS_DUPLICATE_INITED, "channel");
        }

        pChannel->socket = -1;
        pChannel->tid = 0;
        pChannel->pSendQueue = g_async_queue_new();
        pChannel->canStop = FALSE;

        EpsTcpChannelListenerT listener = 
        {
            NULL,
            OnChannelConnected,
            OnChannelDisconnected,
            OnChannelReceived,
            OnChannelSended
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
 * ����ʼ��TCPͨ��
 *
 * @param   pChannel            in  - TCPͨ������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT UninitTcpChannel(EpsTcpChannelT* pChannel)
{
    TRY
    {
        if (! IsChannelInited(pChannel))
        {
            THROW_RESCODE(NO_ERR);
        } 

        if (pChannel->pSendQueue != NULL)
        {
            ClearSendQueue(pChannel);

            g_async_queue_unref(pChannel->pSendQueue);
            pChannel->pSendQueue = NULL;
        }

        if (pChannel->socket != -1)
        {
            shutdown(pChannel->socket, SHUT_RDWR);
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
 * ����TCPͨ��
 *
 * @param   pChannel            in  - TCPͨ������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT StartupTcpChannel(EpsTcpChannelT* pChannel)
{
    TRY
    {
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
 * ֹͣTCPͨ��
 *
 * @param   pChannel            in  - TCPͨ������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT ShutdownTcpChannel(EpsTcpChannelT* pChannel)
{
    TRY
    {
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

/*
 * ��TCPͨ����������
 *
 * @param   pChannel            in  - TCPͨ������
 * @param   data                in  - ����������
 * @param   dataLen             in  - ���������ݳ���
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT SendTcpChannel(EpsTcpChannelT* pChannel, const char* data, uint32 dataLen)
{
    TRY
    {
        if (dataLen == 0 || dataLen > EPS_SENDDATA_MAX_LEN)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "dataLen");
        }
        
        if(! IsChannelInited(pChannel))
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "channel");
        }

        EpsSendDataT* pData = (EpsSendDataT*)calloc(1, sizeof(EpsSendDataT));
        if (pData == NULL)
        {
            THROW_ERROR(ERCD_EPS_OPERSYSTEM_ERROR, strerror(errno));
        }
        memcpy(pData->data, data, dataLen);
        pData->dataLen = dataLen;
        
        g_async_queue_push(pChannel->pSendQueue, (gpointer)pData);
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
 * ע��TCPͨ�������߽ӿ�
 *
 * @param   pChannel            in  - TCPͨ������
 * @param   pListener           in  - ��ע���ͨ��������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT RegisterTcpChannelListener(EpsTcpChannelT* pChannel, const EpsTcpChannelListenerT* pListener)
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
        if (pListener->sendedNotify != NULL)
        {
            pChannel->listener.sendedNotify = pListener->sendedNotify;
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
 * TCPͨ�������̺߳���
 *
 * @param   arg                 in  - �̲߳���(TCPͨ������)
 */
static void* ChannelTask(void* arg)
{
    EpsTcpChannelT* pChannel = (EpsTcpChannelT*)arg;

    while (! pChannel->canStop)
    {
        /* ��TCPͨ�� */
        if (! IsChannelConnected(pChannel))
        {
            if (NOTOK(OpenChannel(pChannel)))
            {
                pChannel->listener.disconnectedNotify(pChannel->listener.pListener,
                    ErrGetErrorCode(), ErrGetErrorDscr());
                    
                usleep(EPS_SOCKET_RECONNECT_INTERVAL * 1000);
                continue;
            }
            else
            {
                pChannel->listener.connectedNotify(pChannel->listener.pListener);
            }
        }
        
        /* ���ȴ������ݷ��� */
        SendData(pChannel);

        /* ��������������� */
        ReceiveData(pChannel);
    }
    
    CloseChannel(pChannel);

    return 0;
}

/**
 * ��TCPͨ��
 *
 * @param   pChannel            in  - TCPͨ������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
static ResCodeT OpenChannel(EpsTcpChannelT* pChannel)
{
    int fd = -1;
    TRY
    {
        int result = -1;
      
        fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd == -1)
        {
            ErrSetError(ERCD_EPS_SOCKET_ERROR, strerror(errno));
            THROW_RESCODE(ERCD_EPS_SOCKET_ERROR);
        }
   
        BOOL reuseaddr = TRUE;
        result = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseaddr, sizeof(reuseaddr));
        if (result == -1)
        {
            ErrSetError(ERCD_EPS_SOCKET_ERROR, strerror(errno));
            THROW_RESCODE(ERCD_EPS_SOCKET_ERROR);
        }

        struct sockaddr_in srvAddr;
        memset(&srvAddr, 0x00, sizeof(srvAddr));
            
        srvAddr.sin_family      = AF_INET;
        srvAddr.sin_addr.s_addr = inet_addr(pChannel->srvAddr);
        srvAddr.sin_port        = htons(pChannel->srvPort);

        result = connect(fd, (struct sockaddr *)&srvAddr,
                    sizeof(struct sockaddr));
        if (result == -1)
        {  
            ErrSetError(ERCD_EPS_SOCKET_ERROR, strerror(errno));
            THROW_RESCODE(ERCD_EPS_SOCKET_ERROR);
        }
            
        pChannel->socket = fd;

        ClearSendQueue(pChannel);
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
 * �ر�TCPͨ��
 *
 * @param   pChannel            in  - TCPͨ������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
static ResCodeT CloseChannel(EpsTcpChannelT* pChannel)
{
    TRY
    {
        if (pChannel->socket != -1)
        {
            shutdown(pChannel->socket, SHUT_RDWR);
            close(pChannel->socket);

            pChannel->socket = -1;
        }

        ClearSendQueue(pChannel);
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
 * @param   pChannel            in  - TCPͨ������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
static ResCodeT SendData(EpsTcpChannelT* pChannel)
{
    TRY
    {
        EpsSendDataT* pData = NULL;
        uint32 sendLen = 0;
        int result = 0;
        
        while (TRUE)
        {
            pData = (EpsSendDataT*)g_async_queue_try_pop_unlocked(pChannel->pSendQueue);

            if (pData == NULL)
            {
                break;
            }

            sendLen = 0;
            while (sendLen < pData->dataLen)
            {
                result = send(pChannel->socket, pData->data+sendLen, pData->dataLen-sendLen, 0);
                if (result <= 0)
                {
                    free(pData);
                    ErrSetError(ERCD_EPS_SOCKET_ERROR, strerror(errno));
                    THROW_RESCODE(ERCD_EPS_SOCKET_ERROR);
                }

                sendLen += result;
            }
            free(pData);
        }
    }
    CATCH
    {
        CloseChannel(pChannel);
        pChannel->listener.disconnectedNotify(pChannel->listener.pListener,
            ErrGetErrorCode(), ErrGetErrorDscr());
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * ����ͨ������
 *
 * @param   pChannel            in  - TCPͨ������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
static ResCodeT ReceiveData(EpsTcpChannelT* pChannel)
{
    TRY
    {
        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(pChannel->socket, &fdset);

        struct timeval timeout;
        timeout.tv_sec = EPS_SOCKET_RECV_TIMEOUT / 1000;
        timeout.tv_usec = (EPS_SOCKET_RECV_TIMEOUT % 1000) * 1000;

        int result = select(pChannel->socket+1, &fdset, 0, 0, &timeout);
        if (result == -1)
        {
            ErrSetError(ERCD_EPS_SOCKET_ERROR, strerror(errno));
            THROW_RESCODE(ERCD_EPS_SOCKET_ERROR);
        }
        
        if (FD_ISSET(pChannel->socket, &fdset))
        {
            int len = recv(pChannel->socket, pChannel->recvBuffer, EPS_SOCKET_RECVBUFFER_LEN, 0);
            if (len > 0)
            {
                pChannel->listener.receivedNotify(pChannel->listener.pListener, 
                        NO_ERR, pChannel->recvBuffer, (uint32)len);
            }
            else if(len == 0)
            {
                ErrSetError(ERCD_EPS_SOCKET_ERROR, "Connection closed by remote");
                THROW_RESCODE(ERCD_EPS_SOCKET_ERROR);
            }
            else
            {
                ErrSetError(ERCD_EPS_SOCKET_ERROR, strerror(errno));
                THROW_RESCODE(ERCD_EPS_SOCKET_ERROR);
            }
        }
    }
    CATCH
    {
        CloseChannel(pChannel);
        pChannel->listener.disconnectedNotify(pChannel->listener.pListener,
                    ErrGetErrorCode(), ErrGetErrorDscr());
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * ������Ͷ���
 *
 * @param   pChannel            in  - TCPͨ������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
static ResCodeT ClearSendQueue(EpsTcpChannelT* pChannel)
{
    TRY
    {
        EpsSendDataT* pData = NULL;
        while (TRUE)
        {
            pData = (EpsSendDataT*)g_async_queue_try_pop_unlocked(pChannel->pSendQueue);
            if (pData != NULL)
            {
                free(pData);
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
 * @param   pChannel            in  - TCPͨ������
 *
 * @return  �Ѿ���ʼ������TRUE�����򷵻�FALSE
 */
static BOOL IsChannelInited(EpsTcpChannelT* pChannel)
{
    return (pChannel->pSendQueue != NULL);
}

/**
 * �ж�ͨ���Ƿ�����
 *
 * @param   pChannel            in  - TCPͨ������
 *
 * @return  �Ѿ���������TRUE�����򷵻�FALSE
 */
static BOOL IsChannelStarted(EpsTcpChannelT* pChannel)
{
    return (pChannel->tid != 0);
}

/**
 * �ж�ͨ���Ƿ����ӳɹ�
 *
 * @param   pChannel            in  - TCPͨ������
 *
 * @return  �Ѿ����ӳɹ�����TRUE�����򷵻�FALSE
 */
static BOOL IsChannelConnected(EpsTcpChannelT * pChannel)
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
static void OnChannelSended(void* pListener, ResCodeT result, const char* data, uint32 dataLen)
{
}
