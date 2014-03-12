/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    udpDriver.c
 *
 * UDP�鲥������ʵ���ļ�
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
        if (pDriver == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pDriver");
        }

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
    }
    CATCH
    {
        if (pDriver != NULL)
        {
            UninitUdpDriver(pDriver);
        }
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
        if (pDriver == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pDriver");
        }

        UninitUdpChannel(&pDriver->channel);
        UninitMktDatabase(&pDriver->database);
        memset(pDriver, 0x00, sizeof(EpsUdpDriverT));
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
        if (pDriver == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pDriver");
        }

        if (pSpi == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pSpi");
        }

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
        RETURN_RESCODE;
    }
}

/**
 *  ����UDP����������
 *
 * @param   pDriver             in  - UDP������
 * @param   address             in  - ���ӵ�ַ
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT ConnectUdpDriver(EpsUdpDriverT* pDriver, const char* address)
{
    TRY
    {
        if (pDriver == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pDriver");
        }

        if (address == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "address");
        }

        THROW_ERROR(ParseAddress(address, pDriver->channel.mcAddr, 
            &pDriver->channel.mcPort, pDriver->channel.localAddr));
        THROW_ERROR(StartupUdpChannel(&pDriver->channel));
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
        if (pDriver == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pDriver");
        }

        ShutdownUdpChannel(&pDriver->channel);
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
        if (pDriver == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pDriver");
        }

        EpsUdpChannelEventT* pEvent = calloc(1, sizeof(EpsUdpChannelEventT));
        if (pEvent == NULL)
        {
            THROW_ERROR(ERCD_EPS_OPERSYSTEM_ERROR, strerror(errno));
        }
        pEvent->eventType = EPS_UDP_EVENTTYPE_LOGIN;
        THROW_ERROR(TriggerUdpChannelEvent(&pDriver->channel, pEvent));
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
        if (pDriver == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pDriver");
        }

        EpsUdpChannelEventT* pEvent = calloc(1, sizeof(EpsUdpChannelEventT));
        if (pEvent == NULL)
        {
            THROW_ERROR(ERCD_EPS_OPERSYSTEM_ERROR, strerror(errno));
        }
        pEvent->eventType = EPS_UDP_EVENTTYPE_LOGOUT;
        THROW_ERROR(TriggerUdpChannelEvent(&(pDriver->channel), pEvent));
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
        if (pDriver == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pDriver");
        }

        if (mktType > EPS_MKTTYPE_NUM)
        {
            THROW_ERROR(ERCD_EPS_INVALID_MKTTYPE);
        }
      
        EpsUdpChannelEventT* pEvent = calloc(1, sizeof(EpsUdpChannelEventT));
        if (pEvent == NULL)
        {
            THROW_ERROR(ERCD_EPS_OPERSYSTEM_ERROR, strerror(errno));
        }
        pEvent->eventType = EPS_UDP_EVENTTYPE_SUBSCRIBED;
        pEvent->eventParam = (uint32)mktType;
        THROW_ERROR(TriggerUdpChannelEvent(&pDriver->channel, pEvent));
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
 * UDPͨ�����ӳɹ�֪ͨ
 *
 * @param   pListener             in  - UDP������
 */
static void OnChannelConnected(void* pListener)
{
    if (pListener == NULL) 
    {
        return;
    }

    EpsUdpDriverT* pDriver = (EpsUdpDriverT*)pListener;
    pDriver->spi.connectedNotify(pDriver->hid);
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
    if (pListener == NULL)
    {
        return;
    }
    
    EpsUdpDriverT* pDriver = (EpsUdpDriverT*)pListener;
    pDriver->spi.disconnectedNotify(pDriver->hid, result, reason);
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
    if (pListener == NULL)
    {
        return;
    }
    
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
        return;
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
 * UDPͨ�������¼�֪ͨ
 *
 * @param   pListener               in  - UDP������
 * @param   pEvent                  in  - �¼�����
 */
static void OnChannelEventOccurred(void* pListener, EpsUdpChannelEventT* pEvent)
{
    if (pListener == NULL || pEvent == NULL)
    {
        return;
    }

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
        char* p1, *p2;

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
