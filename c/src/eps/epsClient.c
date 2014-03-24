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

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "cmn/errlib.h"
#include "udp/udpDriver.h"
#include "tcp/tcpDriver.h"

#include "epsClient.h"


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
 
static volatile gint   g_isLibInited = FALSE;   /* ���ʼ����� */
static volatile uint32 g_maxHid = 0;            /* ��ǰ�����ID */
static GHashTable*     g_handlePool = NULL;     /* ����� */
static GStaticRecMutex g_libLock;               /* ��ͬ������ */


/**
 * �ڲ���������
 */

static BOOL IsLibInited();

static void DisconnectHandle(EpsHandleT* pHandle);
static void DestroyHandle(EpsHandleT* pHandle);

static void _disposeHandle(gpointer data);


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
        if (g_atomic_int_compare_and_exchange(&g_isLibInited, FALSE, TRUE))
        {
        	if (!g_thread_get_initialized())
        	{
        		g_thread_init(NULL);
        	}

            g_handlePool = g_hash_table_new_full(g_int_hash, g_int_equal, NULL, _disposeHandle);
            g_maxHid = 1;
            g_static_rec_mutex_init(&g_libLock);
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
        if (g_atomic_int_compare_and_exchange(&g_isLibInited, TRUE, FALSE))
        {
            g_static_rec_mutex_lock(&g_libLock);

            g_hash_table_destroy(g_handlePool);
            g_handlePool = NULL;
            g_maxHid = 0;

            g_static_rec_mutex_unlock(&g_libLock);

            g_static_rec_mutex_free(&g_libLock);
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
      
        pHandle = (EpsHandleT*)calloc(1, sizeof(EpsHandleT));
        if (pHandle == NULL)
        {
            THROW_ERROR(ERCD_EPS_OPERSYSTEM_ERROR, strerror(errno));
        }
        pHandle->connMode = mode;

        pHandle->hid = g_atomic_int_exchange_and_add((gint*)&g_maxHid, 1);
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

        g_static_rec_mutex_lock(&g_libLock);
        g_hash_table_insert(g_handlePool, (void*)(&pHandle->hid), pHandle);
        g_static_rec_mutex_unlock(&g_libLock);

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

        g_static_rec_mutex_lock(&g_libLock);
        EpsHandleT* pHandle = (EpsHandleT*)g_hash_table_lookup(g_handlePool, (gconstpointer)&hid);
        if (pHandle != NULL)
        {
            g_hash_table_remove(g_handlePool, (gconstpointer)&hid);
        }
        g_static_rec_mutex_unlock(&g_libLock);

        if (pHandle == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_HID);
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

        g_static_rec_mutex_lock(&g_libLock);
        EpsHandleT* pHandle = (EpsHandleT*)g_hash_table_lookup(g_handlePool, (gconstpointer)&hid);
        g_static_rec_mutex_unlock(&g_libLock);

        if (pHandle == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_HID);
        }

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

        g_static_rec_mutex_lock(&g_libLock);
        EpsHandleT* pHandle = (EpsHandleT*)g_hash_table_lookup(g_handlePool, (gconstpointer)&hid);
        g_static_rec_mutex_unlock(&g_libLock);
        
        if (pHandle == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_HID);
        }

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

        g_static_rec_mutex_lock(&g_libLock);
        EpsHandleT* pHandle = (EpsHandleT*)g_hash_table_lookup(g_handlePool, (gconstpointer)&hid);
        g_static_rec_mutex_unlock(&g_libLock);

        if (pHandle == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_HID);
        }

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

        g_static_rec_mutex_lock(&g_libLock);
        EpsHandleT* pHandle = (EpsHandleT*)g_hash_table_lookup(g_handlePool, (gconstpointer)&hid);
        g_static_rec_mutex_unlock(&g_libLock);

        if (pHandle == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_HID);
        }

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

        g_static_rec_mutex_lock(&g_libLock);
        EpsHandleT* pHandle = (EpsHandleT*)g_hash_table_lookup(g_handlePool, (gconstpointer)&hid);
        g_static_rec_mutex_unlock(&g_libLock);
        
        if (pHandle == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_HID);
        }

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

        g_static_rec_mutex_lock(&g_libLock);
        EpsHandleT* pHandle = (EpsHandleT*)g_hash_table_lookup(g_handlePool, (gconstpointer)&hid);
        g_static_rec_mutex_unlock(&g_libLock);

        if (pHandle == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_HID);
        }

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
    return (g_atomic_int_get(&g_isLibInited) == TRUE);
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

    free(pHandle);
}

/**
 * �ͷž��(����GHashTable�Զ�����)
 *
 * @param   data             in  - ���ͷŵľ��ID
 */
static void _disposeHandle(gpointer data)
{
    EpsHandleT* pHandle = (EpsHandleT*)data;

    DisconnectHandle(pHandle);
    DestroyHandle(pHandle);
}


