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

#include "common.h"
#include "epsTypes.h"
#include "errlib.h"

#include "tcpChannel.h"

/**
 * �궨��
 */

#define EPS_SENDDATA_MAX_LEN                    8192
#define EPS_SENDQUEUE_SIZE                      128


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

        pChannel->socket = INVALID_SOCKET;
        pChannel->tid = 0;

        pChannel->canStop = TRUE;
        pChannel->status  = EPS_TCPCHANNEL_STATUS_STOP;
        InitUniQueue(&pChannel->sendQueue, EPS_SENDQUEUE_SIZE);
    
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

        CloseTcpChannel(pChannel);
        UninitUniQueue(&pChannel->sendQueue);
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
            if (pChannel->status == EPS_TCPCHANNEL_STATUS_IDLE)
            {
                pChannel->status = EPS_TCPCHANNEL_STATUS_WORK;
                THROW_RESCODE(NO_ERR);
            }
            else
            {
                THROW_ERROR(ERCD_EPS_DUPLICATE_CONNECT);
            }
        }
       
        pChannel->canStop = FALSE;
        pChannel->status = EPS_TCPCHANNEL_STATUS_WORK;
        
#if defined(__WINDOWS__)
        DWORD tid = 0;
        HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ChannelTask, 
            (LPVOID)pChannel, 0, (LPDWORD)&tid);
        if (tid == 0)
        {
        	pChannel->status = EPS_TCPCHANNEL_STATUS_STOP;
            int lstErrno = SYS_ERRNO;
        	THROW_ERROR(ERCD_EPS_OPERSYSTEM_ERROR, EpsGetSystemError(lstErrno));
        }
        pChannel->thread = thread;
        pChannel->tid = tid;
#endif

#if defined(__LINUX__) || defined(__HPUX__) 
        pthread_t tid;
        int result = pthread_create(&tid, NULL, ChannelTask, (void*)pChannel);
        if (result != 0)
        {
        	pChannel->status = EPS_TCPCHANNEL_STATUS_STOP;
            THROW_ERROR(ERCD_EPS_OPERSYSTEM_ERROR, EpsGetSystemError(result));
        }
        pChannel->tid = tid;
#endif
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

#if defined(__WINDOWS__)
        if (pChannel->tid != GetCurrentThreadId())
#endif

#if defined(__LINUX__) || defined(__HPUX__) 
        if (pChannel->tid != pthread_self())
#endif
        {
            pChannel->canStop = TRUE;
        }
        else
        {
            if (pChannel->status == EPS_TCPCHANNEL_STATUS_WORK)
            {
                CloseTcpChannel(pChannel);
            }

            pChannel->status = EPS_TCPCHANNEL_STATUS_IDLE;
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
 * �ȴ�TCPͨ������
 *
 * @param   pChannel            in  - TCPͨ������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
 ResCodeT JoinTcpChannel(EpsTcpChannelT* pChannel)
{
    TRY
    {
#if defined(__WINDOWS__)
        if (pChannel->tid != 0 && pChannel->tid != GetCurrentThreadId())
        {
        	int result = WaitForSingleObject(pChannel->thread, INFINITE);
        	if (result != WAIT_OBJECT_0)
        	{
        	    int lstErrno = SYS_ERRNO;
        		THROW_ERROR(ERCD_EPS_OPERSYSTEM_ERROR, EpsGetSystemError(lstErrno));
        	}

        	pChannel->thread = NULL;
        	pChannel->tid = 0;
        }
#endif

#if defined(__LINUX__) || defined(__HPUX__) 
        if (pChannel->tid != 0 && pChannel->tid != pthread_self())
        {
            int result = pthread_join(pChannel->tid, NULL);
            if (result != 0)
            {
                THROW_ERROR(ERCD_EPS_OPERSYSTEM_ERROR, EpsGetSystemError(SYS_ERRNO));
            }            

            pChannel->tid = 0;
        }
#endif
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
            int lstErrno = SYS_ERRNO;
            THROW_ERROR(ERCD_EPS_OPERSYSTEM_ERROR, EpsGetSystemError(lstErrno));
        }
        memcpy(pData->data, data, dataLen);
        pData->dataLen = dataLen;

        THROW_ERROR(PushUniQueue(&pChannel->sendQueue, (void*)pData));
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
        if (pChannel->status == EPS_TCPCHANNEL_STATUS_IDLE)
        {
            usleep(EPS_CHANNEL_IDLE_INTL * 1000);
            continue;
        }
        
        /* ��TCPͨ�� */
        if (! IsChannelConnected(pChannel))
        {
            if (NOTOK(OpenTcpChannel(pChannel)))
            {
                pChannel->listener.disconnectedNotify(pChannel->listener.pListener,
                    ErrGetErrorCode(), ErrGetErrorDscr());

                ErrClearError();

#if defined(__WINDOWS__)
                Sleep(EPS_CHANNEL_RECONNECT_INTL);
#endif

#if defined(__LINUX__) || defined(__HPUX__) 
                usleep(EPS_CHANNEL_RECONNECT_INTL * 1000);
#endif                

                continue;
            }
            else
            {
                pChannel->listener.connectedNotify(pChannel->listener.pListener);
            }
        }
        
        /* ���ȴ������ݷ��� */
        if (NOTOK(SendData(pChannel)))
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
    
    CloseTcpChannel(pChannel);

    pChannel->status = EPS_TCPCHANNEL_STATUS_STOP;

    return 0;
}

/**
 * ��TCPͨ��
 *
 * @param   pChannel            in  - TCPͨ������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT OpenTcpChannel(EpsTcpChannelT* pChannel)
{
    SOCKET fd = INVALID_SOCKET;

    TRY
    {
        int result = SOCKET_ERROR;
      
        fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd == INVALID_SOCKET)
        {
            int lstErrno = NET_ERRNO;
            THROW_ERROR(ERCD_EPS_SOCKET_ERROR, EpsGetSystemError(lstErrno));
        }
   
        BOOL reuseaddr = TRUE;
        result = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseaddr, sizeof(reuseaddr));
        if (result == SOCKET_ERROR)
        {
            int lstErrno = NET_ERRNO;
            THROW_ERROR(ERCD_EPS_SOCKET_ERROR, EpsGetSystemError(lstErrno));
        }

        struct sockaddr_in srvAddr;
        memset(&srvAddr, 0x00, sizeof(srvAddr));
            
        srvAddr.sin_family      = AF_INET;
        srvAddr.sin_addr.s_addr = inet_addr(pChannel->srvAddr);
        srvAddr.sin_port        = htons(pChannel->srvPort);

        result = connect(fd, (struct sockaddr *)&srvAddr, sizeof(struct sockaddr));
        if (result == SOCKET_ERROR)
        {  
            int lstErrno = NET_ERRNO;
            THROW_ERROR(ERCD_EPS_SOCKET_ERROR, EpsGetSystemError(lstErrno));
        }
            
        pChannel->socket = fd;

        ClearSendQueue(pChannel);
    }
    CATCH
    {
    	if (fd != INVALID_SOCKET)
    	{
#if defined(__WINDOWS__)
    		closesocket(fd);
#endif

#if defined(__LINUX__) || defined(__HPUX__) 
            close(fd);
#endif

    		fd = INVALID_SOCKET;
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
ResCodeT CloseTcpChannel(EpsTcpChannelT* pChannel)
{
    TRY
    {
       if (pChannel->socket != INVALID_SOCKET)
        {
        	shutdown(pChannel->socket, SHUT_RDWR);

#if defined(__WINDOWS__)
        	closesocket(pChannel->socket);
#endif

#if defined(__LINUX__) || defined(__HPUX__) 
            close(pChannel->socket);
#endif
   
        	pChannel->socket = INVALID_SOCKET;
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
            THROW_ERROR(PopUniQueue(&pChannel->sendQueue, (void**)(&pData)));
    
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

                    int lstErrno = NET_ERRNO;
                    THROW_ERROR(ERCD_EPS_SOCKET_ERROR, EpsGetSystemError(lstErrno));
                }

                sendLen += result;
            }
            free(pData);
        }
    }
    CATCH
    {
        CloseTcpChannel(pChannel);
        
        if (pChannel->status == EPS_TCPCHANNEL_STATUS_WORK)
        {
            pChannel->listener.disconnectedNotify(pChannel->listener.pListener,
                ErrGetErrorCode(), ErrGetErrorDscr());
        }
        ErrClearError();
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
        if (result == SOCKET_ERROR)
        {
            int lstErrno = NET_ERRNO;
            THROW_ERROR(ERCD_EPS_SOCKET_ERROR, EpsGetSystemError(lstErrno));
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
                THROW_ERROR(ERCD_EPS_SOCKET_ERROR, "Connection closed by remote");
            }
            else
            {
                int lstErrno = NET_ERRNO;
                THROW_ERROR(ERCD_EPS_SOCKET_ERROR, EpsGetSystemError(lstErrno));
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
        CloseTcpChannel(pChannel);
        
        if (pChannel->status == EPS_TCPCHANNEL_STATUS_WORK)
        {
            pChannel->listener.disconnectedNotify(pChannel->listener.pListener,
                ErrGetErrorCode(), ErrGetErrorDscr());
        }
        ErrClearError();
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
            THROW_ERROR(PopUniQueue(&pChannel->sendQueue, (void**)(&pData)));
            
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
    return (IsUniQueueInited(&pChannel->sendQueue));
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
    return (pChannel->socket != INVALID_SOCKET);
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
