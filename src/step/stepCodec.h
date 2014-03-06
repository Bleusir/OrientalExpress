/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/*
 * @file    stepCodec.h

 * STEPЭ������������ͷ�ļ� 

 * @version $Id
 * @since   2013/10/19
 * @author  Jin Chengxun 
*/

/*
 MODIFICATION HISTORY:
 <pre>
 ================================================================================
 DD-MMM-YYYY INIT.    SIR    Modification Description
 ----------- -------- ------ ----------------------------------------------------
 19-10-2013  CXJIN    ����
 ================================================================================
  </pre>
*/

#ifndef EPS_STEP_CODEC_H
#define EPS_STEP_CODEC_H

/*
 * ����ͷ�ļ�
 */

#include "stepMessage.h"


#ifdef __cplusplus
extern "C" {
#endif

/*
 * ��������
 */

/*
 * ����STEP��Ϣ
 */
ResCodeT EncodeStepMessage(StepMessageT* pMsg, StepDirectionT direction,
        char* buf, int32 bufSize, int32* pEncodeSize);

/*
 * ����STEP��Ϣ
 */
ResCodeT DecodeStepMessage(const char* buf, int32 bufSize, StepMessageT* pMsg, 
        int32* pDecodeSize);

/*
 * У��STEP��Ϣ
 */
ResCodeT ValidateStepMessage(const StepMessageT* pMsg, StepDirectionT direction);

#ifdef __cplusplus
}
#endif

#endif /* EPS_STEP_CODEC_H */
