/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    epsClientTest.c
 *
 * Express�ӿ�API���Գ���
 *
 * @version $Id
 * @since   2014/03/05
 * @author  Wu Zheng
 */

/**
MODIFICATION HISTORY:
<pre>
================================================================================
DD-MMM-YYYY INIT.    SIR    Modification Description
----------- -------- ------ ----------------------------------------------------
05-MAR-2014 ZHENGWU         ����
================================================================================
</pre>
*/

/**
 * ����ͷ�ļ�
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmn/errlib.h"
#include "eps/epsClient.h"


/**
 * ����ʵ��
 */

static void Usage()
{
    printf("Usage: epsClientTest <mcAddr:mcPort;localAddr>\n\n" \
           "example:\n" \
           "epsClientTest 230.11.1.1:3300;196.123.71.3\n");
}

static void OnEpsConnectedTest(uint32 hid)
{
    printf("==> OnConnected(), hid: %d\n", hid);
}

static void OnEpsMktDataArrivedTest(uint32 hid, const EpsMktDataT* pMktData)
{
    printf("==> OnMktDataArrived(), hid: %d, applID: %d, applSeqNum: %lld\n", hid, pMktData->applID, pMktData->applSeqNum);
}


int main(int argc, char *argv[])
{
    TRY
    {
        ResCodeT rc = NO_ERR;

        setvbuf(stdout, NULL, _IONBF, 0); /* ���ñ�׼���Ϊ���л���ģʽ */

        if (argc < 2)
        {
            Usage();
            exit(0);
        }

        printf("\n>>> epsClientTest starting ... \n");

        printf("==> call EpsInitLib() ... ");
        rc = EpsInitLib();
        if (OK(rc))
        {
            printf("OK.\n");
        }
        else
        {
            printf("failed, Error: %s!!!\n", GetLastError());
            THROW_RESCODE(rc);
        }

        printf("==> call EpsCreateHandle() ... ");
        uint32 hid;
        rc = EpsCreateHandle(&hid, EPS_CONN_MODE_UDP);
        if (OK(rc))
        {
            printf("OK. hid: %d\n", hid);
        }
        else
        {
            printf("failed, Error: %s!!!\n", GetLastError());
            THROW_RESCODE(rc);
        }

        printf("==> call EpsRegisterSpi() ... ");
        EpsClientSpiT spi = 
        {
            OnEpsConnectedTest,
            NULL,
            NULL,
            NULL,
            NULL,
            OnEpsMktDataArrivedTest,
            NULL
        };
 
        rc = EpsRegisterSpi(hid, &spi);
        if (OK(rc))
        {
            printf("OK. hid: %d\n", hid);
        }
        else
        {
            printf("failed, Error: %s!!!\n", GetLastError());
            THROW_RESCODE(rc);
        }

        printf("==> call EpsConnect() ... ");
        rc = EpsConnect(hid, argv[1]);
        if (OK(rc))
        {
            printf("OK. hid :%d\n", hid);
        }
        else
        {
            printf("failed, Error: %s!!!\n", GetLastError());
            THROW_RESCODE(rc);
        }

        while (1)
        {
            int c = getchar();
            if (c == 'q' || c == 'Q')
            {
                printf(">>> receive exit signal ...\n\n");
                break;
            }
        }

        printf("==> call EpsDisconnect() ... ");
        rc = EpsDisconnect(hid);
        if (OK(rc))
        {
            printf("OK. hid :%d\n", hid);
        }
        else
        {
            printf("failed, Error: %s!!!\n", GetLastError());
            THROW_RESCODE(rc);
        }

        printf("==> call EpsDestroyHandle() ... ");
        rc = EpsDestroyHandle(hid);
        if (OK(rc))
        {
            printf("OK. hid :%d\n", hid);
        }
        else
        {
            printf("failed, Error: %s!!!\n", GetLastError());
            THROW_RESCODE(rc);
        }

        printf("==> call EpsUninitLib() ... ");
        rc = EpsUninitLib();
        if (OK(rc))
        {
            printf("OK.\n");
        }
        else
        {
            printf("failed, Error: %s!!!\n", GetLastError());
            THROW_RESCODE(rc);
        }
    }
    CATCH
    {
        
    }
    FINALLY
    {
        printf(">>> epsClientTest stop.\n");
        return (OK(GET_RESCODE()) ? 0 : (0 - GET_RESCODE()));
    }
}
