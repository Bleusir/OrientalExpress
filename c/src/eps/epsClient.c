/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    epsClient.c
 *
 * Express�ӿ�APIʵ���ļ�
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
17-FEB-2014 ZHENGWU         ����
================================================================================
</pre>
*/

/**
 * ����ͷ�ļ�
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "cmn/errlib.h"
#include "cmn/atomic.h"
#include "cmn/recMutex.h"
#include "udp/udpDriver.h"
#include "tcp/tcpDriver.h"

#include "epsClient.h"


/**
 * �궨��
 */

#define EPS_HANDLE_MAX_COUNT            32  /* ��������� */


/**
 * ���Ͷ���
 */

/*
 * �������
 */
typedef struct EpsHandleTag
{
    uint32          hid;        /* ���ID */
    EpsConnModeT    connMode;   /* ����ģʽ */
    union EpsDriverTag
    {
        EpsUdpDriverT udpDriver;
        EpsTcpDriverT tcpDriver;
    } driver;                   /* ������ */
} EpsHandleT;


/**
 * ȫ�ֶ���
 */
 
static volatile int    g_isLibInited = FALSE;   /* ���ʼ����� */
static EpsHandleT      g_handlePool[EPS_HANDLE_MAX_COUNT];/* ����� */
static EpsRecMutexT    g_libLock;               /* ��ͬ������ */


/**
 * �ڲ���������
 */

static BOOL IsLibInited();


static ResCodeT InitHandlePool();
static ResCodeT UninitHandlePool();
static ResCodeT GetNewHandle(EpsHandleT** ppHandle);
static ResCodeT FindHandle(uint32 hid, EpsHandleT** ppHandle);

static void DisconnectHandle(EpsHandleT* pHandle);
static void DestroyHandle(EpsHandleT* pHandle);


/**
 * �ӿں�������
 */

/**
 * ��ʼ��Express��
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT EpsInitLib()
{
    TRY
    {
        if (EpsAtomicIntCompareAndExchange(&g_isLibInited, FALSE, TRUE))
        {
            InitHandlePool();
            InitRecMutex(&g_libLock);
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
 * ����ʼ��Express��
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT EpsUninitLib()
{
    TRY
    {
        if (EpsAtomicIntCompareAndExchange(&g_isLibInited, TRUE, FALSE))
        {
            LockRecMutex(&g_libLock);
            UninitHandlePool();
            UnlockRecMutex(&g_libLock);

            UninitRecMutex(&g_libLock);
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
 * ����ָ������ģʽ�Ĳ������
 *
 * @param   pHid            out - �����ľ��ID
 * @param   mode            in  - ����ģʽ
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
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

        EpsHandleT* pHandle = NULL;
        LockRecMutex(&g_libLock);
        ResCodeT rc = GetNewHandle(&pHandle);
        UnlockRecMutex(&g_libLock);
        THROW_ERROR(rc);

        pHandle->connMode = mode;
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
 * ���پ��
 *
 * @param   hid             in  - �����ٵľ��ID
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT EpsDestroyHandle(uint32 hid)
{
    TRY
    {
        if (! IsLibInited())
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "library");
        }

        EpsHandleT* pHandle = NULL;
        LockRecMutex(&g_libLock);
        ResCodeT rc = FindHandle(hid, &pHandle);
        if (OK(rc))
        {
            DisconnectHandle(pHandle);
            DestroyHandle(pHandle);
        }
        UnlockRecMutex(&g_libLock);

        THROW_ERROR(rc);
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
 * ע���û��ص��ӿ�
 *
 * @param   hid             in  - ��ִ��ע������ľ��ID
 * @param   pSpi            in  - ��ִ��ע����û��ص��ӿ�
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
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

        EpsHandleT* pHandle = NULL;
        LockRecMutex(&g_libLock);
        ResCodeT rc = FindHandle(hid, &pHandle);
        UnlockRecMutex(&g_libLock);
        THROW_ERROR(rc);
       
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
 * ���ӷ�����
 *
 * @param   hid             in  - ��ִ�����Ӳ����ľ��ID
 * @param   address         in  - ������ַ�ַ���������
 *                                TCP: 196.123.1.1:8000
 *                                UDP: 230.11.1.1:3333;196.123.71.1
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
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

        EpsHandleT* pHandle = NULL;
        LockRecMutex(&g_libLock);
        ResCodeT rc = FindHandle(hid, &pHandle);
        UnlockRecMutex(&g_libLock);
        THROW_ERROR(rc);
        
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
 * �Ͽ����ӷ�����
 *
 * @param   hid             in  - ��ִ�жϿ����Ӳ����ľ��ID
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT EpsDisconnect(uint32 hid)
{
    TRY
    {
        if (! IsLibInited())
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "library");
        }

        EpsHandleT* pHandle = NULL;
        LockRecMutex(&g_libLock);
        ResCodeT rc = FindHandle(hid, &pHandle);
        UnlockRecMutex(&g_libLock);
        THROW_ERROR(rc);

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
 * ��½������
 *
 * @param   hid             in  - ��ִ�е�½�����ľ��ID
 * @param   username        in  - ��½�û���
 * @param   password        in  - ��¼����
 * @param   hearbeatIntl    in  - �������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
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

        EpsHandleT* pHandle = NULL;
        LockRecMutex(&g_libLock);
        ResCodeT rc = FindHandle(hid, &pHandle);
        UnlockRecMutex(&g_libLock);
        THROW_ERROR(rc);

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
 * �ǳ�������
 *
 * @param   hid             in  - ��ִ�еǳ������ľ��ID
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
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

        EpsHandleT* pHandle = NULL;
        LockRecMutex(&g_libLock);
        ResCodeT rc = FindHandle(hid, &pHandle);
        UnlockRecMutex(&g_libLock);
        THROW_ERROR(rc);

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
 * ����ָ���г����͵���������
 *
 * @param   hid             in  - ��ִ�ж��Ĳ����ľ��ID
 * @param   mktType         in  - �����ĵ��г�����
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT EpsSubscribeMarketData(uint32 hid, EpsMktTypeT mktType)
{
    TRY
    {
        if (! IsLibInited())
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "library");
        }

        EpsHandleT* pHandle = NULL;
        LockRecMutex(&g_libLock);
        ResCodeT rc = FindHandle(hid, &pHandle);
        UnlockRecMutex(&g_libLock);
        THROW_ERROR(rc);

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
 * ��ȡ���һ��������Ϣ
 *
 * @return  ���ش�����Ϣ��ַ
 */
const char* GetLastError()
{
    return ErrGetErrorDscr();
}

/**
 * �жϿ��Ƿ񱻳�ʼ��
 *
 * @return �Ѿ���ʼ������TRUE�����򷵻�FALSE
 */
static BOOL IsLibInited()
{
    return (g_isLibInited == TRUE);
}

/**
 * ��ʼ�������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
static ResCodeT InitHandlePool()
{
    TRY
    {
        memset(g_handlePool, 0x00, sizeof(g_handlePool));
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
 * ����ʼ�������
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
static ResCodeT UninitHandlePool()
{
    TRY
    {
        uint32 i = 0;
        for (i = 0; i < EPS_HANDLE_MAX_COUNT; i++)
        {
            if (g_handlePool[i].hid != 0)
            {
                DisconnectHandle(&g_handlePool[i]);
                DestroyHandle(&g_handlePool[i]);
            }
        }

        memset(g_handlePool, 0x00, sizeof(g_handlePool));
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
 * ��ȡ�¾��
 *
 * @param   ppHandle             out  - ��ȡ�ĵ��¾��
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
static ResCodeT GetNewHandle(EpsHandleT** ppHandle)
{
    TRY
    {
        uint32 i = 0;
        for (i = 0; i < EPS_HANDLE_MAX_COUNT; i++)
        {
            if (g_handlePool[i].hid == 0)
            {
                g_handlePool[i].hid = i + 1;
                *ppHandle = &g_handlePool[i];
                break;
            }
        }

        if (i >= EPS_HANDLE_MAX_COUNT)
        {
            THROW_ERROR(ERCD_EPS_HID_COUNT_BEYOND_LIMIT, EPS_HANDLE_MAX_COUNT);
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


static ResCodeT FindHandle(uint32 hid, EpsHandleT** ppHandle)
{
    TRY
    {
        if (hid == 0 || hid > EPS_HANDLE_MAX_COUNT)
        {
            THROW_ERROR(ERCD_EPS_INVALID_HID);
        }

        if (g_handlePool[hid - 1].hid == 0)
        {
            THROW_ERROR(ERCD_EPS_INVALID_HID);
        }

        *ppHandle = &g_handlePool[hid - 1];
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
 * �Ͽ����
 *
 * @param   pHandle             in  - ��ִ�жϿ������ľ��ID
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
 * ���پ��
 *
 * @param   pHandle             in  - ��ִ�����ٲ����ľ��ID
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

    memset(pHandle, 0x00, sizeof(EpsHandleT));
}
