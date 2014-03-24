/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    tcpDriver.c
 *
 * TCP����������ʵ���ļ�
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
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#include "step/stepCodec.h"
#include "tcpDriver.h"


/**
 * �ڲ���������
 */

static void OnChannelConnected(void* pListener);
static void OnChannelDisconnected(void* pListener, ResCodeT result, const char* reason);
static void OnChannelReceived(void* pListener, ResCodeT rc, const char* data, uint32 dataLen);
static void OnChannelSended(void* pListener, ResCodeT rc, const char* data, uint32 dataLen);

static void OnEpsConnected(uint32 hid);
static void OnEpsDisconnected(uint32 hid, ResCodeT result, const char* reason);
static void OnEpsLoginRsp(uint32 hid, uint16 heartbeatIntl, ResCodeT result, const char* reason);
static void OnEpsLogoutRsp(uint32 hid, ResCodeT result, const char* reason);
static void OnEpsMktDataSubRsp(uint32 hid, EpsMktTypeT mktType, ResCodeT result, const char* reason);
static void OnEpsMktDataArrived(uint32 hid, const EpsMktDataT* pMktData);
static void OnEpsEventOccurred(uint32 hid, EpsEventTypeT eventType, ResCodeT eventCode, const char* eventText);

static ResCodeT HandleLoginRsp(EpsTcpDriverT* pDriver, const StepMessageT* pMsg);
static ResCodeT HandleLogoutRsp(EpsTcpDriverT* pDriver, const StepMessageT* pMsg);
static ResCodeT HandleMDSubscribeRsp(EpsTcpDriverT* pDriver, const StepMessageT* pMsg);
static ResCodeT HandleMarketData(EpsTcpDriverT* pDriver, const StepMessageT* pMsg);

static ResCodeT BuildLoginRequest(uint64 msgSeqNum, const char* username, const char* password, 
            uint16 heartbeatIntl, char* data, int32* pDataLen);
static ResCodeT BuildLogoutRequest(uint64 msgSeqNum, const char* reason, char* data, int32* pDataLen);
static ResCodeT BuildSubscribeRequest(uint64 msgSeqNum, EpsMktTypeT mktType, char* data, int32* pDataLen);
//static ResCodeT BuildHeartbeatRequest(uint64 msgSeqNum, char* data, int32* pDataLen);
    
static ResCodeT ParseAddress(const char* address, char* srvAddr, uint16* srvPort);
static ResCodeT GetSendingTime(char* szSendingTime);


/**
 * ����ʵ��
 */

/**
 *  ��ʼ��TCP������
 *
 * @param   pDriver             in  - TCP������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT InitTcpDriver(EpsTcpDriverT* pDriver)
{
    TRY
    {
        THROW_ERROR(InitTcpChannel(&pDriver->channel));
        THROW_ERROR(InitMktDatabase(&pDriver->database));
    
        EpsTcpChannelListenerT listener =
        {
            pDriver,
            OnChannelConnected,
            OnChannelDisconnected,
            OnChannelReceived,
            OnChannelSended
        };
        THROW_ERROR(RegisterTcpChannelListener(&pDriver->channel, &listener));

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
       
        pDriver->status = EPS_TCP_STATUS_DISCONNECTED;
        pDriver->msgSeqNum = 1;
        pDriver->recvBufferLen = 0;

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
 * ����ʼ��TCP������
 *
 * @param   pDriver             in  - TCP������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT UninitTcpDriver(EpsTcpDriverT* pDriver)
{
    TRY
    {
        g_static_rec_mutex_lock(&pDriver->lock);

        UninitTcpChannel(&pDriver->channel);
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
 * ע��TCP�������ص�������
 *
 * @param   pDriver             in  - TCP������
 * @param   pSpi                in  - �û��ص��ӿ�
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT RegisterTcpDriverSpi(EpsTcpDriverT* pDriver, const EpsClientSpiT* pSpi)
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
 * ����TCP����������
 *
 * @param   pDriver             in  - TCP������
 * @param   address             in  - ���ӵ�ַ
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT ConnectTcpDriver(EpsTcpDriverT* pDriver, const char* address)
{
    TRY
    {
        g_static_rec_mutex_lock(&pDriver->lock);

        THROW_ERROR(ParseAddress(address, pDriver->channel.srvAddr, &pDriver->channel.srvPort));
        THROW_ERROR(StartupTcpChannel(&pDriver->channel));
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
 * �Ͽ�TCP����������
 *
 * @param   pDriver             in  - TCP������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT DisconnectTcpDriver(EpsTcpDriverT* pDriver)
{
    TRY
    {
        g_static_rec_mutex_lock(&pDriver->lock);

        THROW_ERROR(ShutdownTcpChannel(&pDriver->channel));
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
 * ��½TCP������ 
 * 
 * @param   pDriver             in  - TCP������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT LoginTcpDriver(EpsTcpDriverT* pDriver, const char* username, 
    const char* password, uint16 heartbeatIntl)
{
    TRY
    {
        g_static_rec_mutex_lock(&pDriver->lock);

        EpsTcpStatusT status = pDriver->status;
        if (status != EPS_TCP_STATUS_CONNECTED)
        {
            THROW_ERROR(ERCD_EPS_INVALID_OPERATION);    
        }

        char data[STEP_MSG_MAX_LEN];
        int32 dataLen = (int32)sizeof(data);
        THROW_ERROR(BuildLoginRequest(pDriver->msgSeqNum++, 
            username, password, heartbeatIntl, data, &dataLen));

        pDriver->status = EPS_TCP_STATUS_LOGGING;
        THROW_ERROR(SendTcpChannel(&pDriver->channel, data, dataLen));
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
 * �ǳ�TCP������
 *
 * @param   pDriver             in  - UDP������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT LogoutTcpDriver(EpsTcpDriverT* pDriver, const char* reason)
{
    TRY
    {
        g_static_rec_mutex_lock(&pDriver->lock);

        EpsTcpStatusT status = pDriver->status;
        if (status != EPS_TCP_STATUS_LOGINED && status != EPS_TCP_STATUS_PUBLISHING)
        {
            THROW_ERROR(ERCD_EPS_INVALID_OPERATION);    
        }

        char data[STEP_MSG_MAX_LEN];
        int32 dataLen = (int32)sizeof(data);
        THROW_ERROR(BuildLogoutRequest(pDriver->msgSeqNum++, reason, data, &dataLen));
        
        pDriver->status = EPS_TCP_STATUS_LOGOUTING;
        THROW_ERROR(SendTcpChannel(&pDriver->channel, data, dataLen));
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
 * ����TCP������
 *
 * @param   pDriver             in  - UDP������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT SubscribeTcpDriver(EpsTcpDriverT* pDriver, EpsMktTypeT mktType)
{
    TRY
    {
        g_static_rec_mutex_lock(&pDriver->lock);
        EpsTcpStatusT status = pDriver->status;
        g_static_rec_mutex_unlock(&pDriver->lock);

        if (status != EPS_TCP_STATUS_LOGINED && status != EPS_TCP_STATUS_PUBLISHING)
        {
            THROW_ERROR(ERCD_EPS_INVALID_OPERATION);    
        }

        THROW_ERROR(SubscribeMktData(&pDriver->database, mktType));

        char data[STEP_MSG_MAX_LEN];
        int32 dataLen = (int32)sizeof(data);
        THROW_ERROR(BuildSubscribeRequest(pDriver->msgSeqNum++, mktType, data, &dataLen));

        THROW_ERROR(SendTcpChannel(&pDriver->channel, data, dataLen));
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
 * TCPͨ�����ӳɹ�֪ͨ
 *
 * @param   pListener             in  - TCP������
 */
static void OnChannelConnected(void* pListener)
{
    EpsTcpDriverT* pDriver = (EpsTcpDriverT*)pListener;

    g_static_rec_mutex_lock(&pDriver->lock);

    pDriver->status = EPS_TCP_STATUS_CONNECTED;

    pDriver->spi.connectedNotify(pDriver->hid);

    g_static_rec_mutex_unlock(&pDriver->lock);
}

/**
 * TCPͨ�����ӶϿ�֪ͨ
 *
 * @param   pListener           in  - TCP������
 * @param   result              in  - �Ͽ�������
 * @param   reason              in  - �Ͽ�ԭ������
 */
static void OnChannelDisconnected(void* pListener, ResCodeT result, const char* reason)
{
    EpsTcpDriverT* pDriver = (EpsTcpDriverT*)pListener;

    g_static_rec_mutex_lock(&pDriver->lock);
    
    pDriver->status = EPS_TCP_STATUS_DISCONNECTED;

    UnSubscribeAllMktData(&pDriver->database);
    
    pDriver->spi.disconnectedNotify(pDriver->hid, result, reason);

    g_static_rec_mutex_unlock(&pDriver->lock);
}

/**
 * TCPͨ�����ݽ���֪ͨ
 *
 * @param   pListener           in  - TCP������
 * @param   result              in  - ���ս����
 * @param   data                in  - ��������
 * @param   dataLen             in  - �������ݳ���
 */
static void OnChannelReceived(void* pListener, ResCodeT result, const char* data, uint32 dataLen)
{
    EpsTcpDriverT* pDriver = (EpsTcpDriverT*)pListener;

    TRY
    {
        g_static_rec_mutex_lock(&pDriver->lock);

        if (OK(result))
        {
            memcpy(pDriver->recvBuffer + pDriver->recvBufferLen, data, dataLen);
            pDriver->recvBufferLen += dataLen;

            ResCodeT rc = NO_ERR;
            StepMessageT msg;
            int32 decodeSize = 0;
            uint32 pickupLen = 0;
            while (TRUE)
            {
                rc = DecodeStepMessage(pDriver->recvBuffer + pickupLen, 
                    pDriver->recvBufferLen - pickupLen, &msg, &decodeSize);
                if (NOTOK(rc))
                {
                    if (rc == ERCD_STEP_STREAM_NOT_ENOUGH)
                    {
                        break;
                    }

                    pDriver->spi.eventOccuredNotify(pDriver->hid, EPS_EVENTTYPE_WARNING, rc, ErrGetErrorDscr());
                    THROW_ERROR(rc);
                }

                pickupLen += decodeSize;

                switch (msg.msgType)
                {
                    case STEP_MSGTYPE_LOGON:
                        HandleLoginRsp(pDriver, &msg);
                        break;
                    case STEP_MSGTYPE_LOGOUT:
                        HandleLogoutRsp(pDriver, &msg);
                        break;
                    case STEP_MSGTYPE_MD_REQUEST:
                        HandleMDSubscribeRsp(pDriver, &msg);
                        break;
                    case STEP_MSGTYPE_MD_SNAPSHOT:
                        HandleMarketData(pDriver, &msg);
                        break;
                    case STEP_MSGTYPE_HEARTBEAT:
                    case STEP_MSGTYPE_TRADING_STATUS:
                        break;
                    default:
                        THROW_ERROR(ERCD_EPS_UNEXPECTED_MSGTYPE);
                        break;
                }
            }

            if (pDriver->recvBufferLen > pickupLen && pickupLen > 0)
    		{
    			memmove(pDriver->recvBuffer, pDriver->recvBuffer+pickupLen, pDriver->recvBufferLen-pickupLen);
    		}

            pDriver->recvBufferLen -= pickupLen;
        }
        else
        {
            THROW_ERROR(result);
        }

    }
    CATCH
    {
    }
    FINALLY
    {
        g_static_rec_mutex_unlock(&pDriver->lock);
    }
}

/**
 * TCPͨ�����ݷ���֪ͨ
 *
 * @param   pListener           in  - TCP������
 * @param   result              in  - ���ͽ����
 * @param   data                in  - ��������
 * @param   dataLen             in  - �������ݳ���
 */
static void OnChannelSended(void* pListener, ResCodeT rc, const char* data, uint32 dataLen)
{
}

/**
 * ������½Ӧ��
 *
 * @param   pDriver             in  - TCP������
 * @param   pMsg                in  - ��½Ӧ����Ϣ
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
static ResCodeT HandleLoginRsp(EpsTcpDriverT* pDriver, const StepMessageT* pMsg)
{
    TRY
    {
        g_static_rec_mutex_lock(&pDriver->lock);

        pDriver->status = EPS_TCP_STATUS_LOGINED;

        LogonRecordT* pRecord = (LogonRecordT*)pMsg->body;
        pDriver->spi.loginRspNotify(pDriver->hid, pRecord->heartBtInt,
            NO_ERR, "login succeed");
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
 * �����ǳ�Ӧ��
 *
 * @param   pDriver             in  - TCP������
 * @param   pMsg                in  - �ǳ�Ӧ����Ϣ
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
static ResCodeT HandleLogoutRsp(EpsTcpDriverT* pDriver, const StepMessageT* pMsg)
{
    TRY
    {
        g_static_rec_mutex_lock(&pDriver->lock);

        pDriver->status = EPS_TCP_STATUS_LOGOUT;

        UnSubscribeAllMktData(&pDriver->database);
        
        LogoutRecordT* pRecord = (LogoutRecordT*)pMsg->body;

        EpsTcpStatusT status = pDriver->status;
        switch (status)
        {
            case EPS_TCP_STATUS_LOGGING:
                pDriver->spi.loginRspNotify(pDriver->hid, pDriver->heartbeatIntl,
                    ERCD_EPS_LOGIN_FAILED, pRecord->text);
                break;
            case EPS_TCP_STATUS_LOGOUTING:
                pDriver->spi.logoutRspNotify(pDriver->hid, NO_ERR, pRecord->text);
                break;
            case EPS_TCP_STATUS_PUBLISHING:
                pDriver->spi.mktDataSubRspNotify(pDriver->hid, EPS_MKTTYPE_ALL,
                    ERCD_EPS_SUBMARKETDATA_FAILED, pRecord->text);
                break;
            default:
                pDriver->spi.logoutRspNotify(pDriver->hid, NO_ERR, pRecord->text);
 
                break;
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
 * �������鶩��Ӧ��
 *
 * @param   pDriver             in  - TCP������
 * @param   pMsg                in  - ���鶩��Ӧ����Ϣ
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
static ResCodeT HandleMDSubscribeRsp(EpsTcpDriverT* pDriver, const StepMessageT* pMsg)
{
    TRY
    {
        g_static_rec_mutex_lock(&pDriver->lock);

        MDRequestRecordT* pRecord = (MDRequestRecordT*)pMsg->body;
     
        EpsMktTypeT mktType = (EpsMktTypeT)atoi(pRecord->securityType);

        pDriver->status = EPS_TCP_STATUS_PUBLISHING;

        pDriver->spi.mktDataSubRspNotify(pDriver->hid, mktType, NO_ERR, "subscribe succeed");
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
 * ������������
 *
 * @param   pDriver             in  - TCP������
 * @param   pMsg                in  - ����������Ϣ
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
static ResCodeT HandleMarketData(EpsTcpDriverT* pDriver, const StepMessageT* pMsg)
{
    TRY
    {
        g_static_rec_mutex_lock(&pDriver->lock);

        ResCodeT rc = AcceptMktData(&pDriver->database, pMsg);
        if (NOTOK(rc))
        {
            if (rc == ERCD_EPS_DATASOURCE_CHANGED)
            {
                pDriver->spi.eventOccuredNotify(pDriver->hid, EPS_EVENTTYPE_WARNING, rc, ErrGetErrorDscr());
            }
            else
            {
                THROW_RESCODE(rc);
            }
        }

        EpsMktDataT mktData;
        rc = ConvertMktData(pMsg, &mktData);
        if (NOTOK(rc))
        {
            pDriver->spi.eventOccuredNotify(pDriver->hid, EPS_EVENTTYPE_WARNING, rc, ErrGetErrorDscr());
        }

        pDriver->spi.mktDataArrivedNotify(pDriver->hid, &mktData);
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
 * ������½������Ϣ
 *
 * @param   msgSeqNum           in  - ��Ϣ���
 * @param   username            in  - ��½�û���
 * @param   password            in  - ��½����
 * @param   heartbeatIntl       in  - �������
 * @param   data                out - ��������Ϣ������
 * @param   pDataLen            in  - ��Ϣ����������
 *                              out - ��������Ϣ����
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
static ResCodeT BuildLoginRequest(uint64 msgSeqNum, const char* username, const char* password, 
        uint16 heartbeatIntl, char* data, int32* pDataLen)
{
    TRY
    {
        StepMessageT msg;
        memset(&msg, 0x00, sizeof(msg));
            
        msg.msgType   = STEP_MSGTYPE_LOGON;
        msg.msgSeqNum = msgSeqNum;
        GetSendingTime(msg.sendingTime);
        snprintf(msg.senderCompID, sizeof(msg.senderCompID), STEP_TARGET_COMPID_VALUE);
        snprintf(msg.targetCompID, sizeof(msg.targetCompID), STEP_TARGET_COMPID_VALUE);
        snprintf(msg.msgEncoding, sizeof(msg.msgEncoding), STEP_MSG_ENCODING_VALUE);

        LogonRecordT* pRecord = (LogonRecordT*)msg.body;
        pRecord->encryptMethod = STEP_ENCRYPT_METHOD_NONE;
        pRecord->heartBtInt = heartbeatIntl;
        snprintf(pRecord->username, sizeof(pRecord->username), username);
        snprintf(pRecord->password, sizeof(pRecord->password), password);

        THROW_ERROR(EncodeStepMessage(&msg, STEP_DIRECTION_REQ, data, *pDataLen, pDataLen));
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
 * �����ǳ�������Ϣ
 *
 * @param   msgSeqNum           in  - ��Ϣ���
 * @param   reason              in  - �ǳ�ԭ��
 * @param   data                out - �����ĵ�½��Ϣ
 * @param   pDataLen            in  - ��Ϣ����������
 *                              out - ��������Ϣ����
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
static ResCodeT BuildLogoutRequest(uint64 msgSeqNum, const char* reason, char* data, int32* pDataLen)
{
    TRY
    {
        StepMessageT msg;
        memset(&msg, 0x00, sizeof(msg));
            
        msg.msgType   = STEP_MSGTYPE_LOGOUT;
        msg.msgSeqNum = msgSeqNum;
        GetSendingTime(msg.sendingTime);
        snprintf(msg.senderCompID, sizeof(msg.senderCompID), STEP_TARGET_COMPID_VALUE);
        snprintf(msg.targetCompID, sizeof(msg.targetCompID), STEP_TARGET_COMPID_VALUE);
        snprintf(msg.msgEncoding, sizeof(msg.msgEncoding), STEP_MSG_ENCODING_VALUE);
            
        LogoutRecordT* pRecord = (LogoutRecordT*)msg.body;
        snprintf(pRecord->text, sizeof(pRecord->text), reason);
    
        THROW_ERROR(EncodeStepMessage(&msg, STEP_DIRECTION_REQ, data, *pDataLen, pDataLen));
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
 * ��������������Ϣ
 *
 * @param   msgSeqNum           in  - ��Ϣ���
 * @param   mktType             in  - �����г�����
 * @param   data                out - �����ĵ�½��Ϣ
 * @param   pDataLen            in  - ��Ϣ����������
 *                              out - ��������Ϣ����
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
static ResCodeT BuildSubscribeRequest(uint64 msgSeqNum, EpsMktTypeT mktType, char* data, int32* pDataLen)
{
    TRY
    {
        StepMessageT msg;
        memset(&msg, 0x00, sizeof(msg));
            
        msg.msgType   = STEP_MSGTYPE_MD_REQUEST;
        msg.msgSeqNum = msgSeqNum;
        GetSendingTime(msg.sendingTime);
        snprintf(msg.senderCompID, sizeof(msg.senderCompID), STEP_TARGET_COMPID_VALUE);
        snprintf(msg.targetCompID, sizeof(msg.targetCompID), STEP_TARGET_COMPID_VALUE);
        snprintf(msg.msgEncoding, sizeof(msg.msgEncoding), STEP_MSG_ENCODING_VALUE);
            
        MDRequestRecordT* pRecord = (MDRequestRecordT*)msg.body;
        snprintf(pRecord->securityType, sizeof(pRecord->securityType), "%02d", mktType);
     
        THROW_ERROR(EncodeStepMessage(&msg, STEP_DIRECTION_REQ, data, *pDataLen, pDataLen));
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
 * ��������������Ϣ
 *
 * @param   msgSeqNum           in  - ��Ϣ���
 * @param   data                out - �����ĵ�½��Ϣ
 * @param   pDataLen            in  - ��Ϣ����������
 *                              out - ��������Ϣ����
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
/*
static ResCodeT BuildHeartbeatRequest(uint64 msgSeqNum, char* data, int32* pDataLen)
{
    TRY
    {
        StepMessageT msg;
        memset(&msg, 0x00, sizeof(msg));
            
        msg.msgType   = STEP_MSGTYPE_HEARTBEAT;
        msg.msgSeqNum = msgSeqNum;
        GetSendingTime(msg.sendingTime);
        snprintf(msg.senderCompID, sizeof(msg.senderCompID), STEP_TARGET_COMPID_VALUE);
        snprintf(msg.targetCompID, sizeof(msg.targetCompID), STEP_TARGET_COMPID_VALUE);
        snprintf(msg.msgEncoding, sizeof(msg.msgEncoding), STEP_MSG_ENCODING_VALUE);
            
        THROW_ERROR(EncodeStepMessage(&msg, STEP_DIRECTION_REQ, data, *pDataLen, pDataLen));
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}
*/

/**
 * ��ȡ���鷢��ʱ��
 *
 * @param   szSendingTime   in  - ����ʱ�仺����
 *
 * @return ���ظ�ʽΪHHMMSSss�ĵ�ǰʱ��
 */
static ResCodeT GetSendingTime(char* szSendingTime)
{
    TRY
    {
        struct timeval tv_time;
        struct tm nowTime;
        
        gettimeofday(&tv_time, NULL);
        localtime_r((time_t *)&(tv_time.tv_sec), &nowTime);
        sprintf(szSendingTime, "%02d%02d%02d%02ld",
            nowTime.tm_hour, nowTime.tm_min, nowTime.tm_sec, tv_time.tv_usec / 10000);
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
 * @param   srvAddr                 out - �鲥��ַ
 * @param   srvPort                 out - �鲥�˿�
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
static ResCodeT ParseAddress(const char* address, char* srvAddr, uint16* srvPort)
{
    TRY
    {
        /* �ַ�����ʽΪ196.123.71.3:3333 */
        const char* p = strstr(address, ":");
        if (p == NULL || p == address)
        {
            THROW_ERROR(ERCD_EPS_INVALID_ADDRESS);
        }

        memcpy(srvAddr, address, (p-address));
        *srvPort = atoi(p + 1);
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
