/*
 * Express Project Studio, Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    stepValidator.c
 *
 * STEPУ����ʵ���ļ� 
 *
 * @version $Id
 * @since   2013/11/18
 * @author  Jin Chengxun
 *    
 */

/*
 MODIFICATION HISTORY:
 <pre>
 ================================================================================
 DD-MMM-YYYY INIT.    SIR    Modification Description
 ----------- -------- ------ ----------------------------------------------------
 18-11-2013  CXJIN           ����
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
 * �ڲ���������
 */

static ResCodeT ValidateStepMsgHeader(const StepMessageT* pMsg, 
        StepDirectionT direction);
static ResCodeT ValidateLogonRecord(const LogonRecordT* pRecord, 
        StepDirectionT direction);
static ResCodeT ValidateLogoutRecord(const LogoutRecordT* pRecord, 
        StepDirectionT direction);
static ResCodeT ValidateMDRequestRecord(const MDRequestRecordT* pRecord, 
        StepDirectionT direction);
static ResCodeT ValidateMDSnapshortRecord(const MDSnapshotFullRefreshRecordT* pRecord, 
        StepDirectionT direction);

/*
 * ����ʵ��
 */

/**
 * У��STEP��Ϣ
 *
 * @param   pMsg            in  - STEP��Ϣ
 * @param   direction       in  - ��Ϣ���䷽��
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT ValidateStepMessage(const StepMessageT* pMsg, StepDirectionT direction)
{
    ResCodeT rc = NO_ERR;
    TRY
    {
        rc = ValidateStepMsgHeader(pMsg, direction);
        if(NOTOK(rc))
        {
            THROW_RESCODE(rc);
        }
        
        switch (pMsg->msgType)
        {
            case STEP_MSGTYPE_LOGON:
            {
                rc = ValidateLogonRecord((const LogonRecordT*)pMsg->body, direction);
                if(NOTOK(rc))
                {
                    THROW_RESCODE(rc);
                }
                break;
            }
            case STEP_MSGTYPE_LOGOUT:
            {
                rc = ValidateLogoutRecord((const LogoutRecordT*)pMsg->body, direction);
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
                rc = ValidateMDRequestRecord((const MDRequestRecordT*)pMsg->body, direction);
                if(NOTOK(rc))
                {
                    THROW_RESCODE(rc);
                }
                break;
            }
            case STEP_MSGTYPE_MD_SNAPSHOT:
            {
                rc = ValidateMDSnapshortRecord((const MDSnapshotFullRefreshRecordT*)pMsg->body, 
                        direction);
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
 * У��STEP��Ϣͷ
 *
 * @param   pMsg            in  - STEP��Ϣ
 * @param   direction       in  - ��Ϣ���䷽��
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
static ResCodeT ValidateStepMsgHeader(const StepMessageT* pMsg, StepDirectionT direction)
{
    TRY
    {
        if (pMsg->msgType <= STEP_MSGTYPE_INVALID || 
            pMsg->msgType >= STEP_MSGTYPE_COUNT)
        {
            THROW_ERROR(ERCD_STEP_INVALID_MSGTYPE, pMsg->msgType);
        }

        if (strncmp(pMsg->senderCompID, STEP_INVALID_STRING_VALUE, sizeof(pMsg->senderCompID)) == 0)
        {
            THROW_ERROR(ERCD_STEP_FLD_NOTFOUND, "49, SenderCompID");
        }

        if (strncmp(pMsg->targetCompID, STEP_INVALID_STRING_VALUE, sizeof(pMsg->targetCompID)) == 0)
        {
            THROW_ERROR(ERCD_STEP_FLD_NOTFOUND, "56, TargetCompID");
        }

        if (pMsg->msgSeqNum == (uint64)STEP_INVALID_UINT_VALUE)
        {
            THROW_ERROR(ERCD_STEP_FLD_NOTFOUND, "34, MsgSeqNum");
        }

        if (strncmp(pMsg->sendingTime, STEP_INVALID_STRING_VALUE, sizeof(pMsg->sendingTime)) == 0)
        {
            THROW_ERROR(ERCD_STEP_FLD_NOTFOUND, "52, SendingTime");
        }

        if (strncmp(pMsg->msgEncoding, STEP_MSG_ENCODING_VALUE, sizeof(pMsg->msgEncoding)) != 0)
        {
            THROW_ERROR(ERCD_STEP_INVALID_FLDVALUE, STEP_MSG_ENCODING_TAG, 
                strlen(pMsg->msgEncoding), pMsg->msgEncoding, "Fixed value GBK");
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
 * У���½��Ϣ
 *
 * @param   pRecord         in  - ��½��Ϣ
 * @param   direction       in  - ��Ϣ���䷽��
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT ValidateLogonRecord(const LogonRecordT* pRecord, StepDirectionT direction)
{
    TRY
    {
        if (pRecord->encryptMethod == STEP_INVALID_INT_VALUE)
        {
            THROW_ERROR(ERCD_STEP_FLD_NOTFOUND, "98, encryptMethod");
        }

        if (pRecord->heartBtInt == (uint16)STEP_INVALID_UINT_VALUE)
        {
            THROW_ERROR(ERCD_STEP_FLD_NOTFOUND, "108, heartBtInt");
        }

        if (strncmp(pRecord->username, STEP_INVALID_STRING_VALUE, 
                sizeof(pRecord->username)) == 0)
        {
            THROW_ERROR(ERCD_STEP_FLD_NOTFOUND, "553, username");
        }
        
        if(direction == STEP_DIRECTION_REQ)
        {
            if (strncmp(pRecord->password, STEP_INVALID_STRING_VALUE, 
                    sizeof(pRecord->password)) == 0)
            {
                THROW_ERROR(ERCD_STEP_FLD_NOTFOUND, "554, password");
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
 * У��ǳ���Ϣ
 *
 * @param   pRecord         in  - �ǳ���Ϣ
 * @param   direction       in  - ��Ϣ���䷽��
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT ValidateLogoutRecord(const LogoutRecordT* pRecord, StepDirectionT direction)
{
    TRY
    {
        if(strncmp(pRecord->text, STEP_INVALID_STRING_VALUE, sizeof(pRecord->text)) == 0)
        {
            THROW_ERROR(ERCD_STEP_FLD_NOTFOUND, "58, Text");
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
 * У�����鶩����Ϣ
 *
 * @param   pRecord         in  - ���鶩����Ϣ
 * @param   direction       in  - ��Ϣ���䷽��
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT ValidateMDRequestRecord(const MDRequestRecordT* pRecord, StepDirectionT direction)
{
    TRY
    {
        if(strncmp(pRecord->securityType, STEP_INVALID_STRING_VALUE, 
                sizeof(pRecord->securityType)) == 0)
        {
            THROW_ERROR(ERCD_STEP_FLD_NOTFOUND, "167, securityType");
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
 * У�����������Ϣ
 *
 * @param   pRecord         in  - ���������Ϣ
 * @param   direction       in  - ��Ϣ���䷽��
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT ValidateMDSnapshortRecord(const MDSnapshotFullRefreshRecordT* pRecord, 
        StepDirectionT direction)
{
    TRY
    {
        if (strncmp(pRecord->securityType, STEP_INVALID_STRING_VALUE, 
                sizeof(pRecord->securityType)) == 0)
        {
            THROW_ERROR(ERCD_STEP_FLD_NOTFOUND, "167, securityType");
        }

        if (pRecord->tradSesMode == STEP_INVALID_INT_VALUE)
        {
            THROW_ERROR(ERCD_STEP_FLD_NOTFOUND, "339, tradSesMode");
        }

        if (pRecord->applID == (uint32)STEP_INVALID_UINT_VALUE)
        {
            THROW_ERROR(ERCD_STEP_FLD_NOTFOUND, "1180, applID");
        }

        if (pRecord->applSeqNum == (uint64)STEP_INVALID_UINT_VALUE)
        {
            THROW_ERROR(ERCD_STEP_FLD_NOTFOUND, "1181, applSeqNum");
        }

        if (strncmp(pRecord->tradeDate, STEP_INVALID_STRING_VALUE, 
                sizeof(pRecord->tradeDate)) == 0)
        {
            THROW_ERROR(ERCD_STEP_FLD_NOTFOUND, "75, tradeDate");
        }
        
        if (strncmp(pRecord->mdUpdateType, STEP_INVALID_STRING_VALUE, 
                sizeof(pRecord->mdUpdateType)) == 0)
        {
            THROW_ERROR(ERCD_STEP_FLD_NOTFOUND, "265, mdUpdateType");
        }

        if(pRecord->mdCount == (uint32)STEP_INVALID_UINT_VALUE)
        {
            THROW_ERROR(ERCD_STEP_FLD_NOTFOUND, "5468, mdCount");
        }

        if(pRecord->mdDataLen == (uint32)STEP_INVALID_UINT_VALUE)
        {
            THROW_ERROR(ERCD_STEP_FLD_NOTFOUND, "95, mdDataLen");
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
