/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    stepCodecUtil.c
 *
 * STEPЭ�����빤�߿�ʵ���ļ�
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
#include <stdlib.h>

#include "cmn/errlib.h"
#include "eps/epsTypes.h"

#include "stepCodecUtil.h"


/*
 * ȫ������
 */
const char STEP_DELIMITER  = 0x01;      /* �ָ��� <SOH> */


/*
 * �ڲ���������
 */

/*
 * ���ҷָ���
 */
static int32 FindDelimiter(const char* buf, int32 bufSize, char delimiter);

/*
 * ����ʵ��
 */

/*
 * ����int8����
 *
 * @param   tag             in  -  ���
 * @param   value           in  -  ֵ
 * @param   buf             out -  ���뻺����
 * @param   bufSize         in  -  ���뻺��������
 * @param   pOffset         in  -  ����ǰ������ƫ��
 *                          out -  ����󻺳���ƫ��
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT AddInt8Field(int32 tag, int8 value, char* buf, int32 bufSize, 
        int32* pOffset)
{
    TRY
    {
        char* bufBegin    = buf + *pOffset;
        int32 bufLeftSize = bufSize - *pOffset;
        int len = snprintf(bufBegin, bufLeftSize, "%d=%c%c", tag, value, STEP_DELIMITER);
        if (bufLeftSize < len + 1)
        {
            THROW_ERROR(ERCD_STEP_BUFFER_OVERFLOW);
        }

        *pOffset += len; 
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
 * ����int16����
 *
 * @param   tag             in  -  ���
 * @param   value           in  -  ֵ
 * @param   buf             out -  ���뻺����
 * @param   bufSize         in  -  ���뻺��������
 * @param   pOffset         in  -  ����ǰ������ƫ��
 *                          out -  ����󻺳���ƫ��
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT AddInt16Field(int32 tag, int16 value, char* buf, int32 bufSize, 
        int32* pOffset)
{
    TRY
    {
        char* bufBegin    = buf + *pOffset;
        int32 bufLeftSize = bufSize - *pOffset;
        int len = snprintf(bufBegin, bufLeftSize, "%d=%d%c", tag, value, STEP_DELIMITER);
        if (bufLeftSize < len + 1)
        {
            THROW_ERROR(ERCD_STEP_BUFFER_OVERFLOW);
        }

        *pOffset += len; 
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
 * ����int32����
 *
 * @param   tag             in  -  ���
 * @param   value           in  -  ֵ
 * @param   buf             out -  ���뻺����
 * @param   bufSize         in  -  ���뻺��������
 * @param   pOffset         in  -  ����ǰ������ƫ��
 *                          out -  ����󻺳���ƫ��
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT AddInt32Field(int32 tag, int32 value, char* buf, int32 bufSize, 
        int32* pOffset)
{
    TRY
    {
        char* bufBegin    = buf + *pOffset;
        int32 bufLeftSize = bufSize - *pOffset;
        int len = snprintf(bufBegin, bufLeftSize, "%d=%d%c", tag, value, STEP_DELIMITER);
        if (bufLeftSize < len + 1)
        {
            THROW_ERROR(ERCD_STEP_BUFFER_OVERFLOW);
        }

        *pOffset += len; 
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
 * ����int64����
 *
 * @param   tag             in  -  ���
 * @param   value           in  -  ֵ
 * @param   buf             out -  ���뻺����
 * @param   bufSize         in  -  ���뻺��������
 * @param   pOffset         in  -  ����ǰ������ƫ��
 *                          out -  ����󻺳���ƫ��
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT AddInt64Field(int32 tag, int64 value, char* buf, int32 bufSize, 
        int32* pOffset)
{
    TRY
    {
        char* bufBegin    = buf + *pOffset;
        int32 bufLeftSize = bufSize - *pOffset;
        int len = snprintf(bufBegin, bufLeftSize, "%d=%lld%c", tag, value, STEP_DELIMITER);
        if (bufLeftSize < len + 1)
        {
            THROW_ERROR(ERCD_STEP_BUFFER_OVERFLOW);
        }

        *pOffset += len; 

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
 * ����uint8����
 *
 * @param   tag             in  -  ���
 * @param   value           in  -  ֵ
 * @param   buf             out -  ���뻺����
 * @param   bufSize         in  -  ���뻺��������
 * @param   pOffset         in  -  ����ǰ������ƫ��
 *                          out -  ����󻺳���ƫ��
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT AddUint8Field(int32 tag, uint8 value, char* buf, int32 bufSize, 
        int32* pOffset)
{
    TRY
    {
        char* bufBegin    = buf + *pOffset;
        int32 bufLeftSize = bufSize - *pOffset;
        int len = snprintf(bufBegin, bufLeftSize, "%d=%u%c", tag, value, STEP_DELIMITER);
        if (bufLeftSize < len + 1)
        {
            THROW_ERROR(ERCD_STEP_BUFFER_OVERFLOW);
        }

        *pOffset += len; 
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
 * ����uint16����
 *
 * @param   tag             in  -  ���
 * @param   value           in  -  ֵ
 * @param   buf             out -  ���뻺����
 * @param   bufSize         in  -  ���뻺��������
 * @param   pOffset         in  -  ����ǰ������ƫ��
 *                          out -  ����󻺳���ƫ��
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT AddUint16Field(int32 tag, uint16 value, char* buf, int32 bufSize, 
        int32* pOffset)
{
    TRY
    {
        char* bufBegin    = buf + *pOffset;
        int32 bufLeftSize = bufSize - *pOffset;
        int len = snprintf(bufBegin, bufLeftSize, "%d=%u%c", tag, value, STEP_DELIMITER);
        if (bufLeftSize < len + 1)
        {
            THROW_ERROR(ERCD_STEP_BUFFER_OVERFLOW);
        }

        *pOffset += len;
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
 * ����uint32����
 *
 * @param   tag             in  -  ���
 * @param   value           in  -  ֵ
 * @param   buf             out -  ���뻺����
 * @param   bufSize         in  -  ���뻺��������
 * @param   pOffset         out -  ���볤�� 
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT AddUint32Field(int32 tag, uint32 value, char* buf, int32 bufSize, 
        int32* pOffset)
{
    TRY
    {
        char* bufBegin    = buf + *pOffset;
        int32 bufLeftSize = bufSize - *pOffset;
        int len = snprintf(bufBegin, bufLeftSize, "%d=%u%c", tag, value, STEP_DELIMITER);
        if (bufLeftSize < len + 1)
        {
            THROW_ERROR(ERCD_STEP_BUFFER_OVERFLOW);
        }

        *pOffset += len; 
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
 * ����uint64����
 *
 * @param   tag             in  -  ���
 * @param   value           in  -  ֵ
 * @param   buf             out -  ���뻺����
 * @param   bufSize         in  -  ���뻺��������
 * @param   pOffset         in  -  ����ǰ������ƫ��
 *                          out -  ����󻺳���ƫ��
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT AddUint64Field(int32 tag, uint64 value, char* buf, int32 bufSize, 
        int32* pOffset)
{
    TRY
    {
        char* bufBegin    = buf + *pOffset;
        int32 bufLeftSize = bufSize - *pOffset;
        int len = snprintf(bufBegin, bufLeftSize, "%d=%llu%c", tag, value, STEP_DELIMITER);
        if (bufLeftSize < len + 1)
        {
            THROW_ERROR(ERCD_STEP_BUFFER_OVERFLOW);
        }

        *pOffset += len;
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
 * ����string����
 *
 * @param   tag             in  -  ���
 * @param   value           in  -  ֵ
 * @param   buf             out -  ���뻺����
 * @param   bufSize         in  -  ���뻺��������
 * @param   pOffset         in  -  ����ǰ������ƫ��
 *                          out -  ����󻺳���ƫ��
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT AddStringField(int32 tag, const char* value, char* buf, int32 bufSize, 
        int32* pOffset)
{
    TRY
    {
        char* bufBegin    = buf + *pOffset;
        int32 bufLeftSize = bufSize - *pOffset;
        int len = snprintf(bufBegin, bufLeftSize, "%d=%s%c", tag, value, STEP_DELIMITER);
        if (bufLeftSize < len + 1)
        {
            THROW_ERROR(ERCD_STEP_BUFFER_OVERFLOW);
        }

        *pOffset += len;
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
 * ����binary����
 *
 * @param   tag             in  -  ���
 * @param   value           in  -  ֵ
 * @param   valueLen        in  -  ֵ����
 * @param   buf             out -  ���뻺����
 * @param   bufSize         in  -  ���뻺��������
 * @param   pOffset         in  -  ����ǰ������ƫ��
 *                          out -  ����󻺳���ƫ��
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT AddBinaryField(int32 tag, const char* value, int32 valueLen, char* buf, 
        int32 bufSize, int32* pOffset)
{
    TRY
    {
        char* bufBegin    = buf + *pOffset;
        int32 bufLeftSize = bufSize - *pOffset;
        int len = snprintf(bufBegin, bufLeftSize, "%d=", tag);
        int32 fieldSize = len + valueLen + 1;
        if (bufLeftSize < fieldSize)
        {
            THROW_ERROR(ERCD_STEP_BUFFER_OVERFLOW);
        }

        memcpy(bufBegin+len, value, valueLen);
        bufBegin[fieldSize - 1] = STEP_DELIMITER;
        *pOffset += fieldSize;
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
 * ��ȡ�ַ���������
 *
 * @param   buf         in  - ���뻺����
 * @param   bufSize     in  - ���뻺��������
 * @param   pField      out - �����ֶ�
 * @param   pOffset     in  - ����ǰ������ƫ��
 *                      out - ����󻺳���ƫ��
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT GetTextField(const char* buf, int32 bufSize, StepFieldT* pField, int32* pOffset)
{
    TRY
    {
        memset(pField, 0x00, sizeof(StepFieldT));
        
        const char* bufBegin    = buf + *pOffset;
        const int32 bufLeftSize = bufSize - *pOffset;

        int32 offset1, offset2;

        offset1 = FindDelimiter(bufBegin, bufLeftSize, '='); 
        if (offset1 < 0)
        {
            THROW_ERROR(ERCD_STEP_INVALID_FLDFORMAT, "Sign (=) not found");
        }
        else if (offset1 == 0)
        {
            THROW_ERROR(ERCD_STEP_INVALID_FLDFORMAT, "Tag not found");
        }

        /* У��TAG��Ч��: �ַ���������Ч10�������������ַ�����Ϊ'0' */
        int32 i = 0;
        const char* p = bufBegin;
        if (*p > '9' || *p < '1')
        {
            THROW_ERROR(ERCD_STEP_INVALID_TAG, offset1, bufBegin);
        }
        i++; p++;
        for (; i < offset1; i++, p++)
        {
            if (*p > '9' || *p < '0')
            {
                THROW_ERROR(ERCD_STEP_INVALID_TAG, offset1, bufBegin);
            }
        }
    
        offset2 = FindDelimiter(bufBegin+offset1+1, bufLeftSize-offset1-1, STEP_DELIMITER); 
        if (offset2 < 0)
        {
            THROW_ERROR(ERCD_STEP_INVALID_FLDFORMAT, "Sign (<SOH>) not found");
        }
        else if (offset2 == 0)
        {
            THROW_ERROR(ERCD_STEP_INVALID_FLDFORMAT, "Value not found");
        }
         
        pField->tag = atoi(bufBegin);
        pField->value = (char*)(bufBegin + offset1 + 1);
        pField->valueSize = offset2;
        *pOffset += offset1 + offset2 + 2;
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
 * ��ȡ������������
 *
 * @param   buf         in  - ���뻺����
 * @param   bufSize     in  - ���뻺��������
 * @param   valueSize   in  - ֵ�򳤶�
 * @param   pField      out - �����ֶ�
 * @param   pOffset     in  - ����ǰ������ƫ��
 *                      out - ����󻺳���ƫ��
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT GetBinaryField(const char* buf, int32 bufSize, int32 valueSize, StepFieldT* pField, 
        int32* pOffset)
{
    TRY
    {
        memset(pField, 0x00, sizeof(StepFieldT));
        
        const char* bufBegin    = buf + *pOffset;
        const int32 bufLeftSize = bufSize - *pOffset;

        int32 offset1, offset2;

        offset1 = FindDelimiter(bufBegin, bufLeftSize, '='); 
        if (offset1 < 0)
        {
            THROW_ERROR(ERCD_STEP_INVALID_FLDFORMAT, "Sign (=) not found");
        }
        else if (offset1 == 0)
        {
            THROW_ERROR(ERCD_STEP_INVALID_FLDFORMAT, "Tag not found");
        }

        /* У��TAG��Ч��: �ַ���������Ч10�������������ַ�����Ϊ'0' */
        int32 i = 0;
        const char* p = bufBegin;
        if (*p > '9' || *p < '1')
        {
            THROW_ERROR(ERCD_STEP_INVALID_TAG, offset1, bufBegin);
        }
        i++; p++;
        for (; i < offset1; i++, p++)
        {
            if (*p > '9' || *p < '0')
            {
                THROW_ERROR(ERCD_STEP_INVALID_TAG, offset1, bufBegin);
            }
        }

        offset2 = offset1 + valueSize + 1;
        if (offset2+1 > bufLeftSize)    
        {
            THROW_ERROR(ERCD_STEP_INVALID_FLDFORMAT, "value size overflow");
        }
        else if (bufBegin[offset2] != STEP_DELIMITER)
        {
            THROW_ERROR(ERCD_STEP_INVALID_FLDFORMAT, "Sign (<SOH>) not found");
        }
         
        pField->tag = atoi(bufBegin);
        pField->value = (char*)(bufBegin + offset1 + 1);
        pField->valueSize = valueSize;

        *pOffset += offset2 + 1; 
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
 * ����У���
 *
 * @param   buf         in  -  ������
 * @param   bufSize     in  -  ����������
 * @param   checksum    out -  У���
 *
 * @return  �ɹ�����NO_ERR�����򷵻ش�����
 */
ResCodeT CalcChecksum(const char* buf, int32 bufSize, char* checksum)
{
    TRY
    {
        uint8 iChecksum = 0;
        int32 idx;
        
        for (idx = 0; idx < bufSize; )
        {
            iChecksum += buf[idx++];
        }
        sprintf(checksum, "%03d", iChecksum);
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
 * �����ָ���
 *
 * @param   buf         in  -  ����������
 * @param   bufSize     in  -  ��������������
 * @param   delimiter   in  -  �ָ���
 *
 * @return  �ɹ����طָ����ڻ����е�λ�ƣ����򷵻�-1
 */
static int32 FindDelimiter(const char* buf, int32 bufSize, char delimiter)
{
    int32 idx = 0, offset = -1;
    for (idx = 0; idx < bufSize; idx++)
    {
        if(delimiter == buf[idx])
        {
            offset = idx;
            break;
        }
    }

    return offset;
}

