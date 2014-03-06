/*
 * Express Project Studio, Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    stepEncoder.c
 *
 * STEP������ʵ���ļ� 
 *
 * @version $Id
 * @since   2013/10/19
 * @author  Jin Chengxun
 *    
 */

/*
 MODIFICATION HISTORY:
 <pre>
 ================================================================================
 DD-MMM-YYYY INIT.    SIR    Modification Description
 ----------- -------- ------ ----------------------------------------------------
 19-10-2013  CXJIN           ����
 ================================================================================
  </pre>
*/

/*
 * ����ͷ�ļ�
 */

#include <string.h>
#include <stdio.h>

#include "cmn/errcode.h"
#include "cmn/errlib.h"
#include "eps/epsTypes.h"

#include "stepCodec.h"
#include "stepCodecUtil.h"

/*
 * ȫ�ֶ���
 */

/*
 * STEP��Ϣ���Ͷ�Ӧ��ϵ��
 */
static const char* STEP_MSGTYPE_MAP[] =
{
    STEP_MSGTYPE_LOGON_VALUE,
    STEP_MSGTYPE_LOGOUT_VALUE,
    STEP_MSGTYPE_HEARTBEAT_VALUE,
    STEP_MSGTYPE_MD_REQUEST_VALUE,
    STEP_MSGTYPE_MD_SNAPSHOT_VALUE,
    STEP_MSGTYPE_TRADING_STATUS_VALUE,
};


/*
 * �ڲ���������
 */

static ResCodeT EncodeLogonDataRecord(LogonRecordT* pRecord, StepDirectionT direction, 
        char* buf, int32 bufSize, int32* pEncodeSize);
static ResCodeT EncodeLogoutRecord(LogoutRecordT* pRecord, StepDirectionT direction, 
        char* buf, int32 bufSize, int32* pEncodeSize);
static ResCodeT EncodeMDRequestRecord(MDRequestRecordT* pRecord, StepDirectionT direction, 
        char* buf, int32 bufSize, int32* pEncodeSize);
static ResCodeT EncodeMDSnapshotFullRefreshRecord(MDSnapshotFullRefreshRecordT* pRecord, 
        StepDirectionT direction, char* buf, int32 bufSize, int32* pEncodeSize);
static ResCodeT EncodeStepMessageBody(StepMessageT* pMsg, StepDirectionT direction,
        char* buf, int32 bufSize, int32* pEncodeSize);


/*
 * ����ʵ��
 */

/**
 * ����STEP��Ϣ
 *
 * @param   pMsg            in  - STEP��Ϣ
 * @param   direction       in  - ��Ϣ���䷽��
 * @param   buf             out - ���뻺����
 * @param   bufSize         in  - ���뻺��������
 * @param   pEncodeSize     out - ���볤��
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT EncodeStepMessage(StepMessageT* pMsg, StepDirectionT direction,
        char* buf, int32 bufSize, int32* pEncodeSize)
{
    ResCodeT rc = NO_ERR;
    TRY
    {
        char body[STEP_MSG_MAX_LEN * 2];
        int32 bodySize = 0, encodeSize = 0;

        /* ������Ϣ�� */
        rc = EncodeStepMessageBody(pMsg, direction, body, sizeof(body), &bodySize);\
        if(NOTOK(rc))
        {
            THROW_RESCODE(rc);
        }

        /* ������Ϣͷ */
        rc = AddStringField(STEP_BEGIN_STRING_TAG, STEP_BEGIN_STRING_VALUE, buf, 
                bufSize, &encodeSize);
        if(NOTOK(rc))
        {
            THROW_RESCODE(rc);
        }

        rc = AddUint32Field(STEP_BODY_LENGTH_TAG, bodySize, buf, bufSize, 
                &encodeSize);
        if(NOTOK(rc))
        {
            THROW_RESCODE(rc);
        }

        /* Ԥ����BODY��CHECKSUM�ֶε�λ�� */
        if (encodeSize + bodySize + STEP_CHECKSUM_FIELD_LEN > bufSize)
        {   
            THROW_ERROR(ERCD_STEP_BUFFER_OVERFLOW);
        }

        memcpy(buf+encodeSize, body, bodySize);
        
        encodeSize += bodySize;

        /* ������Ϣβ */
        char checksum[STEP_CHECKSUM_LEN+1];
        rc = CalcChecksum(buf, encodeSize, checksum);
        if(NOTOK(rc))
        {
            THROW_RESCODE(rc);
        }
        rc = AddStringField(STEP_CHECKSUM_TAG, checksum, buf, bufSize, 
                &encodeSize);
        if(NOTOK(rc))
        {
            THROW_RESCODE(rc);
        }
       
        *pEncodeSize = encodeSize;
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
 * �����½��Ϣ
 *
 * @param   pRecord         in  - ��½��Ϣ
 * @param   direction       in  - ��Ϣ����
 * @param   buf             in  - ���뻺����
 *                          out - �����Ļ�����
 * @param   bufSize         in  - ���뻺��������
 * @param   pEncodeSize     in  - ����ǰ�������ѱ��볤��
 *                          out - ����󻺳����ѱ��볤��
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT EncodeLogonDataRecord(LogonRecordT* pRecord, StepDirectionT direction, 
        char* buf, int32 bufSize, int32* pEncodeSize)
{
    ResCodeT rc = NO_ERR;
    TRY
    {
        int32 recordSize = 0;

        char* bufBegin    = buf + *pEncodeSize;
        int32 bufLeftSize = bufSize - *pEncodeSize;

        rc = AddInt8Field(STEP_ENCRYPT_METHOD_TAG, pRecord->encryptMethod, 
                bufBegin, bufLeftSize, &recordSize);
        if(NOTOK(rc))
        {
            THROW_RESCODE(rc);
        }

        rc = AddUint16Field(STEP_HEARTBT_INT_TAG, pRecord->heartBtInt, 
                bufBegin, bufLeftSize, &recordSize);
        if(NOTOK(rc))
        {
            THROW_RESCODE(rc);
        }

        rc = AddStringField(STEP_USERNAME_TAG, pRecord->username, 
                bufBegin, bufLeftSize, &recordSize);
        if(NOTOK(rc))
        {
            THROW_RESCODE(rc);
        }

        if(STEP_DIRECTION_REQ == direction)
        {
            rc = AddStringField(STEP_PASSWORD_TAG, pRecord->password, 
                    bufBegin, bufLeftSize, &recordSize);
            if(NOTOK(rc))
            {
                THROW_RESCODE(rc);
            }
        }

        *pEncodeSize += recordSize;
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
 * ����ǳ���Ϣ
 *
 * @param   pRecord         in  - �ǳ���Ϣ
 * @param   direction       in  - ��Ϣ����
 * @param   buf             in  - ���뻺����
 *                          out - �����Ļ�����
 * @param   bufSize         in  - ���뻺��������
 * @param   pEncodeSize     in  - ����ǰ�������ѱ��볤��
 *                          out - ����󻺳����ѱ��볤��
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT EncodeLogoutRecord(LogoutRecordT* pRecord, StepDirectionT direction, 
        char* buf, int32 bufSize, int32* pEncodeSize)
{
    ResCodeT rc = NO_ERR;
    TRY
    {
        int32 recordSize = 0;

        char* bufBegin    = buf + *pEncodeSize;
        int32 bufLeftSize = bufSize - *pEncodeSize;

        rc = AddStringField(STEP_TEXT_TAG, pRecord->text, 
                bufBegin, bufLeftSize, &recordSize);
        if(NOTOK(rc))
        {
            THROW_RESCODE(rc);
        }

        *pEncodeSize += recordSize;
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
 * �������鶩����Ϣ
 *
 * @param   pRecord         in  - ���鶩����Ϣ
 * @param   direction       in  - ��Ϣ����
 * @param   buf             in  - ���뻺����
 *                          out - �����Ļ�����
 * @param   bufSize         in  - ���뻺��������
 * @param   pEncodeSize     in  - ����ǰ�������ѱ��볤��
 *                          out - ����󻺳����ѱ��볤��
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT EncodeMDRequestRecord(MDRequestRecordT* pRecord, StepDirectionT direction, 
        char* buf, int32 bufSize, int32* pEncodeSize)
{
    ResCodeT rc = NO_ERR;
    TRY
    {
        int32 recordSize = 0;

        char* bufBegin    = buf + *pEncodeSize;
        int32 bufLeftSize = bufSize - *pEncodeSize;

        rc = AddStringField(STEP_SECURITY_TYPE_TAG, pRecord->securityType, 
                bufBegin, bufLeftSize, &recordSize);
        if(NOTOK(rc))
        {
            THROW_RESCODE(rc);
        }

        *pEncodeSize += recordSize;
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
 * �����������
 *
 * @param   pRecord         in     -  ���������Ϣ
 * @param   direction       in  - ��Ϣ����
 * @param   buf             in  - ���뻺����
 *                          out - �����Ļ�����
 * @param   bufSize         in  - ���뻺��������
 * @param   pEncodeSize     in  - ����ǰ�������ѱ��볤��
 *                          out - ����󻺳����ѱ��볤��
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
static ResCodeT EncodeMDSnapshotFullRefreshRecord(MDSnapshotFullRefreshRecordT* pRecord, 
        StepDirectionT direction, char* buf, int32 bufSize, int32* pEncodeSize)
{
    ResCodeT rc = NO_ERR;
    TRY
    {
        int32 recordSize = 0;

        char* bufBegin    = buf + *pEncodeSize;
        int32 bufLeftSize = bufSize - *pEncodeSize;
        
        rc = AddStringField(STEP_SECURITY_TYPE_TAG, pRecord->securityType, 
                bufBegin, bufLeftSize, &recordSize);
        if(NOTOK(rc))
        {
            THROW_RESCODE(rc);
        }

        rc = AddInt16Field(STEP_TRADE_SES_MODE_TAG, pRecord->tradSesMode, 
                bufBegin, bufLeftSize, &recordSize);
        if(NOTOK(rc))
        {
            THROW_RESCODE(rc);
        }

        rc = AddUint32Field(STEP_APPL_ID_TAG, pRecord->applID, 
                 bufBegin, bufLeftSize, &recordSize);
        if(NOTOK(rc))
        {
            THROW_RESCODE(rc);
        }

        rc = AddUint64Field(STEP_APPL_SEQ_NUM_TAG, pRecord->applSeqNum, 
                 bufBegin, bufLeftSize, &recordSize);
        if(NOTOK(rc))
        {
            THROW_RESCODE(rc);
        }

        rc = AddStringField(STEP_TRADE_DATE_TAG, pRecord->tradeDate, 
                 bufBegin, bufLeftSize, &recordSize);
        if(NOTOK(rc))
        {
            THROW_RESCODE(rc);
        }

        if (pRecord->lastUpdateTime[0] != 0x00)
        {
            rc = AddStringField(STEP_LAST_UPDATETIME_TAG, pRecord->lastUpdateTime, 
                     bufBegin, bufLeftSize, &recordSize);
            if(NOTOK(rc))
            {
                THROW_RESCODE(rc);
            }
        }
        rc = AddStringField(STEP_MD_UPDATETYPE_TAG, pRecord->mdUpdateType, 
                 bufBegin, bufLeftSize, &recordSize);
        if(NOTOK(rc))
        {
            THROW_RESCODE(rc);
        }

        rc = AddUint32Field(STEP_MD_COUNT_TAG, pRecord->mdCount, 
                 bufBegin, bufLeftSize, &recordSize);
        if(NOTOK(rc))
        {
            THROW_RESCODE(rc);
        }

        rc = AddUint32Field(STEP_RAWDATA_LENGTH_TAG, pRecord->mdDataLen, 
                 bufBegin, bufLeftSize, &recordSize);
        if(NOTOK(rc))
        {
            THROW_RESCODE(rc);
        }

        rc = AddBinaryField(STEP_RAWDATA_TAG, pRecord->mdData, pRecord->mdDataLen, 
                 bufBegin, bufLeftSize, &recordSize);
        if(NOTOK(rc))
        {
            THROW_RESCODE(rc);
        }

        *pEncodeSize += recordSize;
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
 * ����STEP��Ϣ��
 *
 * @param   pMsg            in  -  STEP��Ϣ
 * @param   direction       in  - ��Ϣ����
 * @param   buf             in  - ���뻺����
 *                          out - �����Ļ�����
 * @param   bufSize         in  - ���뻺��������
 * @param   pEncodeSize     in  - ����ǰ�������ѱ��볤��
 *                          out - ����󻺳����ѱ��볤��
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
static ResCodeT EncodeStepMessageBody(StepMessageT* pMsg, StepDirectionT direction,
        char* buf, int32 bufSize, int32* pEncodeSize)
{
    ResCodeT rc = NO_ERR;
    TRY
    {
        if (pMsg->msgType <= STEP_MSGTYPE_INVALID ||
            pMsg->msgType >= STEP_MSGTYPE_COUNT)
        {
            THROW_ERROR(ERCD_STEP_INVALID_MSGTYPE, pMsg->msgType);
        }
     
        int encodeSize = 0;
        
        rc = AddStringField(STEP_MSG_TYPE_TAG, STEP_MSGTYPE_MAP[pMsg->msgType], 
                (char*)buf, bufSize, &encodeSize);
        if(NOTOK(rc))
        {
            THROW_RESCODE(rc);
        }

        rc = AddStringField(STEP_SENDER_COMP_ID_TAG, pMsg->senderCompID, 
                (char*)buf, bufSize, &encodeSize);
        if(NOTOK(rc))
        {
            THROW_RESCODE(rc);
        }

        rc = AddStringField(STEP_TARGET_COMP_ID_TAG, pMsg->targetCompID, 
                (char*)buf, bufSize, &encodeSize);
        if(NOTOK(rc))
        {
            THROW_RESCODE(rc);
        }

        rc = AddUint64Field(STEP_MSG_SEQ_NUM_TAG, pMsg->msgSeqNum, 
                (char*)buf, bufSize, &encodeSize);
        if(NOTOK(rc))
        {
            THROW_RESCODE(rc);
        }

        rc = AddStringField(STEP_SENDING_TIME_TAG, pMsg->sendingTime, 
                (char*)buf, bufSize, &encodeSize);
        if(NOTOK(rc))
        {
            THROW_RESCODE(rc);
        }

        rc = AddStringField(STEP_MSG_ENCODING_TAG, pMsg->msgEncoding, 
                (char*)buf, bufSize, &encodeSize);
        if(NOTOK(rc))
        {
            THROW_RESCODE(rc);
        }

        switch(pMsg->msgType)
        {
            case STEP_MSGTYPE_LOGON:
            {
                rc = EncodeLogonDataRecord((LogonRecordT*)pMsg->body,
                        direction, (char*)buf, bufSize, &encodeSize);
                if(NOTOK(rc))
                {
                    THROW_RESCODE(rc);
                }
                break;
            }
            case STEP_MSGTYPE_LOGOUT:
            {
                rc = EncodeLogoutRecord((LogoutRecordT*)pMsg->body,
                        direction, (char*)buf, bufSize, &encodeSize);
                if(NOTOK(rc))
                {
                    THROW_RESCODE(rc);
                }
                break;
            }
            case STEP_MSGTYPE_HEARTBEAT:
            {
                break;
            }
            case STEP_MSGTYPE_MD_REQUEST:
            {
                rc = EncodeMDRequestRecord((MDRequestRecordT*)pMsg->body,
                        direction, (char*)buf, bufSize, &encodeSize);
                if(NOTOK(rc))
                {
                    THROW_RESCODE(rc);
                }
                break;
            }
            case STEP_MSGTYPE_MD_SNAPSHOT:
            {
                rc = EncodeMDSnapshotFullRefreshRecord((MDSnapshotFullRefreshRecordT*)pMsg->body, 
                        direction, (char*)buf, bufSize, &encodeSize);
                if(NOTOK(rc))
                {
                    THROW_RESCODE(rc);
                }
                break;
            }
            case STEP_MSGTYPE_TRADING_STATUS:
            {
                break;
            }
            
            default:
                THROW_ERROR(ERCD_STEP_INVALID_MSGTYPE, pMsg->msgType);
                break;
        }

        *pEncodeSize = encodeSize;
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}
