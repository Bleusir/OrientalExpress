/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    epsClient.h
 *
 * �����쳵�ӿ�API����ͷ�ļ�
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

#ifndef EPS_CLIENT_H
#define EPS_CLIENT_H

/**
 * ����ͷ�ļ�
 */

#include "epsTypes.h"
#include "epsData.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
 * �ӿں�������
 */

/**
 * ��ʼ��Express��
 *
 * @return  �ɹ�����1�����򷵻ش�����
 *
 * memo: �����ǵ�һ�������õ�Express�⺯��
 */
int32 EpsInitLib();

/**
 * ����ʼ��Express��
 *
 * @return  �ɹ�����1�����򷵻ش�����
 *
 * memo: ���������һ�����õ�Express�⺯�������øú�����ر����о�����ͷ���Դ
 */
int32 EpsUninitLib();

/**
 * ����ָ������ģʽ�Ĳ������
 *
 * @param   pHid            out - �����ľ��ID
 * @param   mode            in  - ����ģʽ
 *
 * @return  �ɹ�����1�����򷵻ش�����
 *
 * memo: ���������ִ�к������ӡ���½�����ĵȲ����Ļ�����λ�����ID����Ψһ��ʶ�����
 *       ����������ڷ��������Դ
 */
int32 EpsCreateHandle(uint32* pHid, EpsConnModeT mode);

/**
 * ���پ��
 *
 * @param   hid             in  - �����ٵľ��ID
 *
 * @return  �ɹ�����1�����򷵻ش�����
 *
 * memo: ���پ�������ͷž�������Դ
 */
int32 EpsDestroyHandle(uint32 hid);


/**
 * ע���û��ص��ӿ�
 *
 * @param   hid             in  - ��ִ��ע������ľ��ID
 * @param   pSpi            in  - ��ִ��ע����û��ص��ӿ�
 *
 * @return  �ɹ�����1�����򷵻ش�����
 *
 * memo: ע���û��ص��ӿڣ����ӿ��ڴ�������ɹ����������
 */
int32 EpsRegisterSpi(uint32 hid, const EpsClientSpiT* pSpi);

/**
 * ���ӷ�����
 *
 * @param   hid             in  - ��ִ�����Ӳ����ľ��ID
 * @param   address         in  - ������ַ�ַ���������
 *                                TCP: 196.123.1.1:8000
 *                                UDP: 230.11.1.1:3333;196.123.71.1
 *
 * @return  �ɹ�����1�����򷵻ش�����
 *
 * memo: ������������ָ����ַ�ķ�������ע��ָ���ķ�������ַ����������ģʽ��ƥ�䣬
 *       ���ӳɹ�ͨ���ͻ��˻ص�����connectedNotify֪ͨ�û���
 *       �ڵ���EpsDisconnect()ǰ�������ӶϿ������Զ���������        
 */
int32 EpsConnect(uint32 hid, const char* address);

/**
 * �Ͽ����ӷ�����
 *
 * @param   hid             in  - ��ִ�жϿ����Ӳ����ľ��ID
 *
 * @return  �ɹ�����1�����򷵻ش�����
 *
 * memo: �Ͽ��������ӷ����������óɹ����û��������յ��ص�֪ͨ
 */
int32 EpsDisconnect(uint32 hid);

/**
 * ��½������
 *
 * @param   hid             in  - ��ִ�е�½�����ľ��ID
 * @param   username        in  - ��½�û���
 * @param   password        in  - ��¼����
 * @param   hearbeatIntl    in  - �������
 *
 * @return  �ɹ�����1�����򷵻ش�����
 *
 * memo: �������������½ָ����ӿ������ӳɹ���������ã���½���ͨ���ͻ��˻ص�����loginRspNotify֪ͨ�û���
 *       ���ӿڶ���UDPģʽ�ľ���ǿ�ѡ���ã����Ƿ��سɹ���
 */
int32 EpsLogin(uint32 hid, const char* username, const char* password, uint16 heartbeatIntl);

/**
 * �ǳ�������
 *
 * @param   hid             in  - ��ִ�еǳ������ľ��ID
 *
 * @return  �ɹ�����1�����򷵻ش�����
 *
 * memo: ������������ǳ�ָ����ӿ��ڵ�½�ɹ���������ã��ǳ����ͨ���ͻ��˻ص�����logoutRspNotify֪ͨ�û���
 *       ���ӿڶ���UDPģʽ�ľ���ǿ�ѡ���ã����Ƿ��سɹ���
 */
int32 EpsLogout(uint32 hid, const char* reason);

/**
 * ����ָ���г����͵���������
 *
 * @param   hid             in  - ��ִ�ж��Ĳ����ľ��ID
 * @param   mktType         in  - �����ĵ��г�����
 *
 * @return  �ɹ�����1�����򷵻ش�����
 *
 * memo: ���������������ָ����ӿ��ڵ�½�ɹ�(TCPģʽ)�����ӳɹ�(UDPģʽ)��������ã�
 *       ���Ľ��ͨ���ͻ��˻ص�����mktDataSubRspNotify֪ͨ�û�
 */
int32 EpsSubscribeMarketData(uint32 hid, EpsMktTypeT mktType);

/**
 * ��ȡ���һ�δ�����Ϣ����
 *
 * @return  ���һ�εĴ���������Ϣ
 *
 * memo: �����ýӿں���ʧ�ܻ�ص��ӿ���ʾʧ��ʱ������ñ��ӿڣ�
 *       ���ñ��ӿڻ�ȡ�Ĵ�����Ϣ�������߳���ص�
 */
const char* EpsGetLastError();


#ifdef __cplusplus
}
#endif

#endif /* EPS_CLIENT_H */
