/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    udpDriver.c
 *
 * UDP����������ʵ���ļ�
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
#include <string.h>
#include <errno.h>

#include "step/stepCodec.h"
#include "udpDriver.h"


/**
 * ���Ͷ���
 */

/*
 * UDP�¼�����ö��
 */
typedef enum EpsUdpEventTypeTag
{
    EPS_UDP_EVENTTYPE_LOGIN         = 1,    /* ��½�¼� */
    EPS_UDP_EVENTTYPE_LOGOUT        = 2,    /* �ǳ��¼� */
    EPS_UDP_EVENTTYPE_SUBSCRIBED    = 3,    /* �����¼� */
} EpsUdpEventTypeT;


/**
 * �ڲ���������
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

static ResCodeT HandleReceiveTimeout(EpsUdpDriverT* pDriver);
static ResCodeT ParseAddress(const char* address, char* mcAddr, uint16* mcPort, char* localAddr);


/**
 * ����ʵ��
 */

/**
 *  ��ʼ��UDP������
 *
 * @param   pDriver             in  - UDP������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
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

        InitRecMutex(&pDriver->lock);
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
 * ����ʼ��UDP������
 *
 * @param   pDriver             in  - UDP������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT UninitUdpDriver(EpsUdpDriverT* pDriver)
{
    TRY
    {
        LockRecMutex(&pDriver->lock);
            
        UninitUdpChannel(&pDriver->channel);
        UninitMktDatabase(&pDriver->database);

        UnlockRecMutex(&pDriver->lock);
 
        UninitRecMutex(&pDriver->lock);
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
 *  ע��UDP�������ص�������
 *
 * @param   pDriver             in  - UDP������
 * @param   pSpi                in  - �û��ص��ӿ�
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT RegisterUdpDriverSpi(EpsUdpDriverT* pDriver, const EpsClientSpiT* pSpi)
{
    TRY
    {
        LockRecMutex(&pDriver->lock);

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
        if (pSpi->eventOccurredNotify != NULL)
        {
            pDriver->spi.eventOccurredNotify = pSpi->eventOccurredNotify;
        }
    }
    CATCH
    {
    }
    FINALLY
    {
        UnlockRecMutex(&pDriver->lock);

        RETURN_RESCODE;
    }
}

/**
 * ����UDP����������
 *
 * @param   pDriver             in  - UDP������
 * @param   address             in  - ���ӵ�ַ
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT ConnectUdpDriver(EpsUdpDriverT* pDriver, const char* address)
{
    char        localAddr[EPS_IP_MAX_LEN+1];
    char        mcAddr[EPS_IP_MAX_LEN+1];
    uint16      mcPort;

    TRY
    {
        LockRecMutex(&pDriver->lock);
        
        memcpy(localAddr, pDriver->channel.localAddr, sizeof(localAddr));
        memcpy(mcAddr, pDriver->channel.mcAddr, sizeof(mcAddr));
        mcPort = pDriver->channel.mcPort;
   
        THROW_ERROR(ParseAddress(address, pDriver->channel.mcAddr, 
            &pDriver->channel.mcPort, pDriver->channel.localAddr));
        THROW_ERROR(StartupUdpChannel(&pDriver->channel));
    }
    CATCH
    {
        if(GET_RESCODE() == ERCD_EPS_DUPLICATE_CONNECT)
        {
            memcpy(pDriver->channel.localAddr, localAddr, sizeof(localAddr));
            memcpy(pDriver->channel.mcAddr, mcAddr, sizeof(mcAddr));
            pDriver->channel.mcPort = mcPort;
        }
    }
    FINALLY
    {
        UnlockRecMutex(&pDriver->lock);

        RETURN_RESCODE;
    }
}

/**
 * �Ͽ�UDP����������
 *
 * @param   pDriver             in  - UDP������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT DisconnectUdpDriver(EpsUdpDriverT* pDriver)
{
    TRY
    {
        ResCodeT rc = NO_ERR;
        
        LockRecMutex(&pDriver->lock);
        
        rc = ShutdownUdpChannel(&pDriver->channel);

        UnlockRecMutex(&pDriver->lock);

        THROW_ERROR(rc);
        THROW_ERROR(JoinUdpChannel(&pDriver->channel));
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
 * ��½UDP������ 
 * 
 * @param   pDriver             in  - UDP������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT LoginUdpDriver(EpsUdpDriverT* pDriver, const char* username, 
    const char* password, uint16 heartbeatIntl)
{
    TRY
    {
        LockRecMutex(&pDriver->lock);

        EpsUdpChannelEventT event = 
        {
            EPS_UDP_EVENTTYPE_LOGIN, 0
        };

        snprintf(pDriver->username, sizeof(pDriver->username), username);
        snprintf(pDriver->password, sizeof(pDriver->password), username);
        pDriver->heartbeatIntl = heartbeatIntl;
        THROW_ERROR(TriggerUdpChannelEvent(&pDriver->channel, event));
    }
    CATCH
    {
    }
    FINALLY
    {
        UnlockRecMutex(&pDriver->lock);

        RETURN_RESCODE;
    }
}

/*
 * �ǳ�UDP������
 *
 * @param   pDriver             in  - UDP������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT LogoutUdpDriver(EpsUdpDriverT* pDriver, const char* reason)
{
    TRY
    {
        LockRecMutex(&pDriver->lock);

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
        UnlockRecMutex(&pDriver->lock);

        RETURN_RESCODE;
    }
}

/**
 * ����UDP������
 *
 * @param   pDriver             in  - UDP������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT SubscribeUdpDriver(EpsUdpDriverT* pDriver, EpsMktTypeT mktType)
{
    TRY
    {
        LockRecMutex(&pDriver->lock);

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
        UnlockRecMutex(&pDriver->lock);

        RETURN_RESCODE;
    }
}

/**
 * UDPͨ�����ӳɹ�֪ͨ
 *
 * @param   pListener             in  - UDP������
 */
static void OnChannelConnected(void* pListener)
{
    EpsUdpDriverT* pDriver = (EpsUdpDriverT*)pListener;
    LockRecMutex(&pDriver->lock);
    pDriver->spi.connectedNotify(pDriver->hid);
    UnlockRecMutex(&pDriver->lock);
}

/**
 * UDPͨ�����ӶϿ�֪ͨ
 *
 * @param   pListener           in  - UDP������
 * @param   result              in  - �Ͽ�������
 * @param   reason              in  - �Ͽ�ԭ������
 */
static void OnChannelDisconnected(void* pListener, ResCodeT result, const char* reason)
{
    EpsUdpDriverT* pDriver = (EpsUdpDriverT*)pListener;
    LockRecMutex(&pDriver->lock);
    UnsubscribeAllMktData(&pDriver->database);
    pDriver->spi.disconnectedNotify(pDriver->hid, result, reason);
    UnlockRecMutex(&pDriver->lock);
}

/**
 * UDPͨ�����ݽ���֪ͨ
 *
 * @param   pListener           in  - UDP������
 * @param   result              in  - ���մ�����
 * @param   data                in  - ��������
 * @param   dataLen             in  - �������ݳ���
 */
static void OnChannelReceived(void* pListener, ResCodeT result, const char* data, uint32 dataLen)
{
    EpsUdpDriverT* pDriver = (EpsUdpDriverT*)pListener;
    TRY
    {
        LockRecMutex(&pDriver->lock);

        if (OK(result))
        {
            ResCodeT rc = NO_ERR;
            
            StepMessageT msg;
            int32 decodeSize = 0;
            THROW_ERROR(DecodeStepMessage(data, dataLen, &msg, &decodeSize));

            if (msg.msgType != STEP_MSGTYPE_MD_SNAPSHOT)
            {
                THROW_RESCODE(NO_ERR);
            }

            rc = AcceptMktData(&pDriver->database, &msg);
            if (NOTOK(rc))
            {
                if (rc == ERCD_EPS_DATASOURCE_CHANGED)
                {
                    pDriver->spi.eventOccurredNotify(pDriver->hid, EPS_EVENTTYPE_WARNING, rc, ErrGetErrorDscr());
                }
                else if (rc == ERCD_EPS_MKTTYPE_UNSUBSCRIBED)
                {
                    THROW_RESCODE(NO_ERR);
                }
                else 
                {
                    THROW_RESCODE(rc);
                }
            }

            EpsMktDataT mktData;
            THROW_ERROR(ConvertMktData(&msg, &mktData));

            pDriver->spi.mktDataArrivedNotify(pDriver->hid, &mktData);

            pDriver->recvIdleTimes = 0;
        }
        else
        {
            if (result == ERCD_EPS_SOCKET_TIMEOUT)
            {
                THROW_ERROR(HandleReceiveTimeout(pDriver));
            }
            else 
            {
                THROW_ERROR(result);
            }
        }
    }
    CATCH
    {
        pDriver->spi.eventOccurredNotify(pDriver->hid, EPS_EVENTTYPE_ERROR, ErrGetErrorCode(), ErrGetErrorDscr());

        CloseUdpChannel(&pDriver->channel);
        OnChannelDisconnected(pListener, ErrGetErrorCode(), ErrGetErrorDscr());

        ErrClearError();
    }
    FINALLY
    {
        UnlockRecMutex(&pDriver->lock);
    }
}

/**
 * UDPͨ�������¼�֪ͨ
 *
 * @param   pListener               in  - UDP������
 * @param   pEvent                  in  - �¼�����
 */
static void OnChannelEventOccurred(void* pListener, EpsUdpChannelEventT* pEvent)
{
    EpsUdpDriverT* pDriver = (EpsUdpDriverT*)pListener;

    LockRecMutex(&pDriver->lock);

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
            UnsubscribeAllMktData(&pDriver->database);
            
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

    UnlockRecMutex(&pDriver->lock);
}

/**
 * UDPͨ�����ݽ��ճ�ʱ֪ͨ
 *
 * @param   pDriver             in  - UDP������
 */
static ResCodeT HandleReceiveTimeout(EpsUdpDriverT* pDriver)
{
    TRY
    {
        pDriver->recvIdleTimes++;
        
        if ((pDriver->recvIdleTimes * EPS_SOCKET_RECV_TIMEOUT) >= EPS_DRIVER_KEEPALIVE_TIME)
        {
            ErrSetError(ERCD_EPS_CHECK_KEEPALIVE_TIMEOUT);
                    
            pDriver->spi.eventOccurredNotify(pDriver->hid, EPS_EVENTTYPE_WARNING, 
                        GET_RESCODE(), ErrGetErrorDscr());

            pDriver->recvIdleTimes = 0;
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
 * ������ַ�ַ���
 *
 * @param   address                 in  - ��ַ�ַ���
 * @param   mcAddr                  out - �鲥��ַ
 * @param   mcPort                  out - �鲥�˿�
 * @param   localAddr               out - ���ص�ַ
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
static ResCodeT ParseAddress(const char* address, char* mcAddr, uint16* mcPort, char* localAddr)
{
    TRY
    {
        /* �ַ�����ʽΪ230.11.1.1:3333;196.123.71.1 */
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
 * Express SPIռλ����
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

