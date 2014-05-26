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

#include "common.h"
#include "errlib.h"
#include "epsTypes.h"
#include "stepCodecUtil.h"

#include "stepCodec.h"

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
    TRY
    {
        char body[STEP_MSG_MAX_LEN * 2];
        int32 bodySize = 0, encodeSize = 0;

        /* ������Ϣ�� */
        THROW_ERROR(EncodeStepMessageBody(pMsg, direction, body, sizeof(body), &bodySize));

        /* ������Ϣͷ */
        THROW_ERROR(AddStringField(STEP_BEGIN_STRING_TAG, STEP_BEGIN_STRING_VALUE, buf, 
                bufSize, &encodeSize));

        THROW_ERROR(AddUint32Field(STEP_BODY_LENGTH_TAG, bodySize, buf, bufSize, 
                &encodeSize));

        /* Ԥ����BODY��CHECKSUM�ֶε�λ�� */
        if (encodeSize + bodySize + STEP_CHECKSUM_FIELD_LEN > bufSize)
        {   
            THROW_ERROR(ERCD_STEP_BUFFER_OVERFLOW);
        }

        memcpy(buf+encodeSize, body, bodySize);
        
        encodeSize += bodySize;

        /* ������Ϣβ */
        char checksum[STEP_CHECKSUM_LEN+1];
        THROW_ERROR(CalcChecksum(buf, encodeSize, checksum));
        THROW_ERROR(AddStringField(STEP_CHECKSUM_TAG, checksum, buf, bufSize, 
                &encodeSize));
       
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
    TRY
    {
        int32 recordSize = 0;

        char* bufBegin    = buf + *pEncodeSize;
        int32 bufLeftSize = bufSize - *pEncodeSize;

        THROW_ERROR(AddInt8Field(STEP_ENCRYPT_METHOD_TAG, pRecord->encryptMethod, 
                bufBegin, bufLeftSize, &recordSize));

        THROW_ERROR(AddUint16Field(STEP_HEARTBT_INT_TAG, pRecord->heartBtInt, 
                bufBegin, bufLeftSize, &recordSize));

        THROW_ERROR(AddStringField(STEP_USERNAME_TAG, pRecord->username, 
                bufBegin, bufLeftSize, &recordSize));

        if(STEP_DIRECTION_REQ == direction)
        {
            THROW_ERROR(AddStringField(STEP_PASSWORD_TAG, pRecord->password, 
                    bufBegin, bufLeftSize, &recordSize));
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
    TRY
    {
        int32 recordSize = 0;

        char* bufBegin    = buf + *pEncodeSize;
        int32 bufLeftSize = bufSize - *pEncodeSize;

        THROW_ERROR(AddStringField(STEP_TEXT_TAG, pRecord->text, 
                bufBegin, bufLeftSize, &recordSize));

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
    TRY
    {
        int32 recordSize = 0;

        char* bufBegin    = buf + *pEncodeSize;
        int32 bufLeftSize = bufSize - *pEncodeSize;

        THROW_ERROR(AddStringField(STEP_SECURITY_TYPE_TAG, pRecord->securityType, 
                bufBegin, bufLeftSize, &recordSize));

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
    TRY
    {
        int32 recordSize = 0;

        char* bufBegin    = buf + *pEncodeSize;
        int32 bufLeftSize = bufSize - *pEncodeSize;
        
        THROW_ERROR(AddStringField(STEP_SECURITY_TYPE_TAG, pRecord->securityType, 
                bufBegin, bufLeftSize, &recordSize));

        THROW_ERROR(AddInt16Field(STEP_TRADE_SES_MODE_TAG, pRecord->tradSesMode, 
                bufBegin, bufLeftSize, &recordSize));

        THROW_ERROR(AddUint32Field(STEP_APPL_ID_TAG, pRecord->applID, 
                 bufBegin, bufLeftSize, &recordSize));

        THROW_ERROR(AddUint64Field(STEP_APPL_SEQ_NUM_TAG, pRecord->applSeqNum, 
                 bufBegin, bufLeftSize, &recordSize));

        THROW_ERROR(AddStringField(STEP_TRADE_DATE_TAG, pRecord->tradeDate, 
                 bufBegin, bufLeftSize, &recordSize));

        if (pRecord->lastUpdateTime[0] != 0x00)
        {
            THROW_ERROR(AddStringField(STEP_LAST_UPDATETIME_TAG, pRecord->lastUpdateTime, 
                     bufBegin, bufLeftSize, &recordSize));
        }
        THROW_ERROR(AddStringField(STEP_MD_UPDATETYPE_TAG, pRecord->mdUpdateType, 
                 bufBegin, bufLeftSize, &recordSize));

        THROW_ERROR(AddUint32Field(STEP_MD_COUNT_TAG, pRecord->mdCount, 
                 bufBegin, bufLeftSize, &recordSize));

        THROW_ERROR(AddUint32Field(STEP_RAWDATA_LENGTH_TAG, pRecord->mdDataLen, 
                 bufBegin, bufLeftSize, &recordSize));

        THROW_ERROR(AddBinaryField(STEP_RAWDATA_TAG, pRecord->mdData, pRecord->mdDataLen, 
                 bufBegin, bufLeftSize, &recordSize));
 
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
    TRY
    {
        if (pMsg->msgType <= STEP_MSGTYPE_INVALID ||
            pMsg->msgType >= STEP_MSGTYPE_COUNT)
        {
            THROW_ERROR(ERCD_STEP_INVALID_MSGTYPE, pMsg->msgType);
        }
     
        int encodeSize = 0;
        
        THROW_ERROR(AddStringField(STEP_MSG_TYPE_TAG, STEP_MSGTYPE_MAP[pMsg->msgType], 
                (char*)buf, bufSize, &encodeSize));

        THROW_ERROR(AddStringField(STEP_SENDER_COMP_ID_TAG, pMsg->senderCompID, 
                (char*)buf, bufSize, &encodeSize));

        THROW_ERROR(AddStringField(STEP_TARGET_COMP_ID_TAG, pMsg->targetCompID, 
                (char*)buf, bufSize, &encodeSize));

        THROW_ERROR(AddUint64Field(STEP_MSG_SEQ_NUM_TAG, pMsg->msgSeqNum, 
                (char*)buf, bufSize, &encodeSize));

        THROW_ERROR(AddStringField(STEP_SENDING_TIME_TAG, pMsg->sendingTime, 
                (char*)buf, bufSize, &encodeSize));

        THROW_ERROR(AddStringField(STEP_MSG_ENCODING_TAG, pMsg->msgEncoding, 
                (char*)buf, bufSize, &encodeSize));

        switch(pMsg->msgType)
        {
            case STEP_MSGTYPE_LOGON:
            {
                THROW_ERROR(EncodeLogonDataRecord((LogonRecordT*)pMsg->body,
                        direction, (char*)buf, bufSize, &encodeSize));
                break;
            }
            case STEP_MSGTYPE_LOGOUT:
            {
                THROW_ERROR(EncodeLogoutRecord((LogoutRecordT*)pMsg->body,
                        direction, (char*)buf, bufSize, &encodeSize));
                break;
            }
            case STEP_MSGTYPE_HEARTBEAT:
            {
                break;
            }
            case STEP_MSGTYPE_MD_REQUEST:
            {
                THROW_ERROR(EncodeMDRequestRecord((MDRequestRecordT*)pMsg->body,
                        direction, (char*)buf, bufSize, &encodeSize));
                break;
            }
            case STEP_MSGTYPE_MD_SNAPSHOT:
            {
                THROW_ERROR(EncodeMDSnapshotFullRefreshRecord((MDSnapshotFullRefreshRecordT*)pMsg->body, 
                        direction, (char*)buf, bufSize, &encodeSize));
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
