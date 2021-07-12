/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or its licensors.
 * Without the prior written permission of Airoha and/or its licensors,
 * any reproduction, modification, use or disclosure of Airoha Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) Airoha Software
 * if you have agreed to and been bound by the applicable license agreement with
 * Airoha ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of Airoha Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT AIROHA SOFTWARE RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
 */


#include "bt.h"
#include "App_Media.h"
#include "App_ResourceManager.h"
#include "APP_AudioDriver.h"
#include "AudioDSP_StreamManager.h"
#include "App_LedControl.h"
#include "App_EventOpcode.h"
#include "App_State.h"
#include "App_ChargeBattery.h"
#include "App_Battery.h"
#include "FW_Api.h"
#include "App_Nvram_Type.h"
#include "PM_Connection.h"
#include "App_VpRtControl.h"
#include "App_System.h"

static void app_Media_ClearMediaEvtQ(U16 EvtIndex);
static U32 app_Media_MsgHandler(Handler handler, U16 id, void *payload, U32 id_ext);

/****************************************************************************************************
Variables
*****************************************************************************************************/
HandlerData gMediaHandler = { app_Media_MsgHandler };
MEDIA_EVT_CTL_STRU gMediaCtl;

/****************************************************************************************************
Define
*****************************************************************************************************/

#define	MEDIA_EVT_DATA_NUM		gMediaCtl.mediaEvtNum
#define MEDIA_EVT_LOCK_NUM		gMediaCtl.mediaEvtLockNum
#define MEDIA_EVT_FILTER_NUM	gMediaCtl.mediaEvtFilterNum

#define	MEDIA_EVT_DATA(X)		((MEDIA_EVT_DATA_STRU *)gMediaCtl.pMediaData + X)
#define	MEDIA_EVT_LOCK(X)		((MEDIA_EVT_LOCK_STRU *)gMediaCtl.pMediaLockTbl + X)
#define MEDIA_EVT_FILTER(X)		((MEDIA_EVT_FILTER_STRU *)gMediaCtl.pMediaFilter+ X)

/****************************************************************************************************
Static function prototype
*****************************************************************************************************/
//#if !BRINGUP_1562
//#if !FPGA
static BOOL app_Media_IsMediaEventPlayUnderHFPState(U16 eventCode);
//#endif
//#endif

/****************************************************************************************************
Static function
*****************************************************************************************************/
static void app_Media_ReadMediaDataNum(void)
{
	U16 number;

	gMediaCtl.mediaEvtNum = 0;
	if((number = NVKEY_ReadFullKey(NVKEYID_APP_MEDIA_EVENT_DATA, NULL, 0)) > 0)
	{
		if((number % sizeof(MEDIA_EVT_DATA_STRU)) == 0)
		{
			gMediaCtl.mediaEvtNum = (number / sizeof(MEDIA_EVT_DATA_STRU));
		}
	}
}

static void app_Media_ReadMediaLockNum(void)
{
	U16 number;

	gMediaCtl.mediaEvtLockNum = 0;
	if((number = NVKEY_ReadFullKey(NVKEYID_APP_MEDIA_EVENT_LOCK, NULL, 0)) > 0)
	{
		if((number % sizeof(MEDIA_EVT_LOCK_STRU)) == 0)
		{
			gMediaCtl.mediaEvtLockNum = (number / sizeof(MEDIA_EVT_LOCK_STRU));
		}
	}
}

static void app_Media_ReadMediaFilterNum(void)
{
	U16 number;

	gMediaCtl.mediaEvtFilterNum = 0;
	if((number = NVKEY_ReadFullKey(NVKEYID_APP_MEDIA_EVENT_FILTER, NULL, 0)) > 0)
	{
		if((number % sizeof(MEDIA_EVT_FILTER_STRU)) == 0)
		{
			gMediaCtl.mediaEvtFilterNum = (number / sizeof(MEDIA_EVT_FILTER_STRU));
		}
	}
}

static BOOL app_Meida_ReadMediaData(void)
{
	U8 *pNvkeyPtr;

	app_Media_ReadMediaDataNum();
	if(MEDIA_EVT_DATA_NUM == 0)
	{
		return FALSE;
	}

	if(NVKEY_ReadFullKey(NVKEYID_APP_MEDIA_EVENT_DATA, NULL, 0) != (sizeof(MEDIA_EVT_DATA_STRU) * MEDIA_EVT_DATA_NUM))
	{
		return FALSE;
	}

	pNvkeyPtr = (U8 *)NVKEY_GetPayloadFlashAddress(NVKEYID_APP_MEDIA_EVENT_DATA);

	if(pNvkeyPtr == NULL)
	{
		return FALSE;
	}

	gMediaCtl.pMediaData = pNvkeyPtr;

	return TRUE;
}

static BOOL app_Meida_ReadMediaLockTbl(void)
{
	U8 *pNvkeyPtr;

	app_Media_ReadMediaLockNum();

	if(MEDIA_EVT_LOCK_NUM == 0)
	{
		return TRUE;
	}

	if(NVKEY_ReadFullKey(NVKEYID_APP_MEDIA_EVENT_LOCK, NULL, 0) != (sizeof(MEDIA_EVT_LOCK_STRU) * MEDIA_EVT_LOCK_NUM))
	{
		return FALSE;
	}
	pNvkeyPtr = (U8 *)NVKEY_GetPayloadFlashAddress(NVKEYID_APP_MEDIA_EVENT_LOCK);

	if(pNvkeyPtr == NULL)
	{
		return FALSE;
	}

	gMediaCtl.pMediaLockTbl = pNvkeyPtr;

	return TRUE;
}

static BOOL app_Media_ReadMediaFilterData(void)
{
	U8 *pNvkeyPtr;

	app_Media_ReadMediaFilterNum();

	if(MEDIA_EVT_FILTER_NUM == 0)
	{
		return TRUE;
	}

	if(NVKEY_ReadFullKey(NVKEYID_APP_MEDIA_EVENT_FILTER, NULL, 0) != (sizeof(MEDIA_EVT_FILTER_STRU) * MEDIA_EVT_FILTER_NUM))
	{
		return FALSE;
	}
	pNvkeyPtr = (U8 *)NVKEY_GetPayloadFlashAddress(NVKEYID_APP_MEDIA_EVENT_FILTER);

	if(pNvkeyPtr == NULL)
	{
		return FALSE;
	}

	gMediaCtl.pMediaFilter = pNvkeyPtr;
	return TRUE;
}

static void app_Media_RemoveQBuf(U16 msgOpCode)
{
	U8 evtCount;

	if(gMediaCtl.mediaQNum > 0 && gMediaCtl.mediaQ[0] == msgOpCode)
	{
		gMediaCtl.mediaQNum--;
		for(evtCount = 0 ; evtCount < gMediaCtl.mediaQNum ; evtCount++)
		{
			gMediaCtl.mediaQ[evtCount] = gMediaCtl.mediaQ[evtCount + 1];
		}
		gMediaCtl.mediaQ[evtCount] = 0;
	}
}

//#if !BRINGUP_1562
//#if !FPGA
static BOOL app_Media_CheckValidPowerOffMediaEvt(U16 evtOpCode)
{
	switch(evtOpCode)
	{
		case MEDIA_EVT_KEY_POWER_OFF:
		case MEDIA_EVT_POWER_OFF:
		case MEDIA_EVT_POWER_OFF_IN_SHUTDOWN_LOW_BAT:
		case MEDIA_EVT_OFF_STATE:
		case MEDIA_EVT_POWER_OFF_UNDER_CHECKPOINT1:
		case MEDIA_EVT_POWER_OFF_UNDER_CHECKPOINT2:
		case MEDIA_EVT_POWER_OFF_UNDER_CHECKPOINT3:
		case MEDIA_EVT_KEY_RESET_PAIRED_DEVICES:
		case MEDIA_EVT_KEY_RESET_TO_FACTORY:
			return TRUE;
			break;
		default:
			return FALSE;
			break;
	}
}
//#endif
//#endif

static U16 app_Media_FindMediaEvtCount(U16 evtOpCode)
{
	U16 evtCount;

	for(evtCount = 0 ; evtCount < MEDIA_EVT_DATA_NUM ; evtCount++)
	{
		if(MEDIA_EVT_DATA(evtCount)->evtOpCode != NO_MEDIA && evtOpCode == MEDIA_EVT_DATA(evtCount)->evtOpCode)
		{
			return evtCount;
		}
	}
	return MEDIA_EVT_DATA_NUM;
}

static void app_Media_MediaFilterEvtHandle(U16 evtOpCode)
{
	U16 evtCount;

	for(evtCount = 0 ; evtCount < MEDIA_EVT_FILTER_NUM ; evtCount++)
	{
		if(MEDIA_EVT_FILTER(evtCount)->evtOpCode != NO_MEDIA && evtOpCode == MEDIA_EVT_FILTER(evtCount)->evtOpCode)
		{
			if(MEDIA_EVT_FILTER(evtCount)->isCancleFilter)
			{
				APP_LED_CancelFilter(MEDIA_EVT_FILTER(evtCount)->filterIndex);
			}
			else
			{
				APP_LED_SetFilterPattern(MEDIA_EVT_FILTER(evtCount)->filterIndex);
			}
		}
	}
}

static BOOL app_Media_VpRtSyncStopHandler(U16 eventIndex)
{
	DBG_LOG_APP_SYSTEM( "[Media] VpRtSyncStopHandler, eventIndex:0x%x", 1, eventIndex);
	return APP_VpRt_SyncStopHandler(eventIndex);
}

static void app_Media_SendMediaCmd(U16 evtCount)
{
	//#if !BRINGUP_1562
	//#if !FPGA
	BD_ADDR_T* pBdServiceAddr = APP_GetServiceBdAddr();
	U8 mmiState = APP_State_GetTopState(pBdServiceAddr);
#ifdef MCSYNC_SHARE_MODE
	U8 shareMode = BtMCSync_GetShareMode();
#endif

	DBG_LOG_APP_SYSTEM( "[Media]: mediaQ0:%d, LED: %d, VP: %s, RT: %s", 4, gMediaCtl.mediaQ[0], (MEDIA_EVT_DATA(evtCount)->ledFGIndex & EVENT_LED_FG_TIMEOUT_MASK), APP_VPLogString[MEDIA_EVT_DATA(evtCount)->vpIndex], APP_RTLogString[MEDIA_EVT_DATA(evtCount)->rtIndex]);

	if((mmiState != APP_OFF && mmiState != APP_DETACHING_LINK) || app_Media_CheckValidPowerOffMediaEvt(MEDIA_EVT_DATA(evtCount)->evtOpCode))
	{
		if(MEDIA_EVT_DATA(evtCount)->ledFGIndex != LED_INVALID)
			APP_LED_SetFgParameter(gMediaCtl.mediaQ[0], (MEDIA_EVT_DATA(evtCount)->ledFGIndex & EVENT_LED_FG_TIMEOUT_MASK), MEDIA_EVT_DATA(evtCount)->overwriteTime & EVENT_LED_FG_TIMEOUT_MASK, (MEDIA_EVT_DATA(evtCount)->ledFGIndex & VOICE_PROMPT_BEFORE_RINGTONE_BIT) ? TRUE : FALSE);

		APP_VpRt_StopSystemVpUnderHFPState(MEDIA_EVT_DATA(evtCount)->evtOpCode);

		APP_VpRt_SetMediaEventInfo(MEDIA_EVT_DATA(evtCount)->evtOpCode, MEDIA_EVT_DATA(evtCount)->vpIndex, MEDIA_EVT_DATA(evtCount)->rtIndex, MEDIA_EVT_DATA(evtCount)->overwriteTime);

		if(PM_IsProfileConnected(pBdServiceAddr, PROFILE_MCSYNC) && APP_Media_IsMediaEventAllowSync(MEDIA_EVT_DATA(evtCount)->evtOpCode)
		#ifdef MCSYNC_SHARE_MODE
			&& shareMode != MCSYNC_SHARE_MODE_FOLLOWER_ENABLE
		#endif
		)
		{
			//DBG_LOG_APP_SYSTEM( "[Media] Media Need to Sync, event:0x%x", 1, MEDIA_EVT_DATA(evtCount)->evtOpCode);
			APP_VpRt_UnSniffSendIFPacket();
			return;
		}
		else
		{
			//DBG_LOG_APP_SYSTEM( "[Media] Media Not Need to Sync, event:%d", 1, MEDIA_EVT_DATA(evtCount)->evtOpCode);
			if((MEDIA_EVT_DATA(evtCount)->vpIndex != 0xFF) || (MEDIA_EVT_DATA(evtCount)->rtIndex != 0xFF))
			{
				if(!app_Media_IsMediaEventPlayUnderHFPState(MEDIA_EVT_DATA(evtCount)->evtOpCode)
				#ifdef MCSYNC_SHARE_MODE
					&& shareMode != MCSYNC_SHARE_MODE_FOLLOWER_ENABLE
				#endif
				)
				{
					U8 state = (BtAwsMce_GetDefaultRole() == ROLE_PARTNER) ? APP_GetAgentState() : mmiState;

					if(state == APP_HFP_INCOMMING)
					{
						if(BtAwsMce_IsDefaultRoleAgent())
							DBG_LOG_APP_SYSTEM( "[Media] Agent media event in HFP state, event:%x", 1, MEDIA_EVT_DATA(evtCount)->evtOpCode);
						else if(BtAwsMce_GetDefaultRole() == ROLE_PARTNER)
							DBG_LOG_APP_SYSTEM( "[Media] Partner media event in HFP state, event:%x", 1, MEDIA_EVT_DATA(evtCount)->evtOpCode);

						APP_Media_SendFakeMediaCmd(MEDIA_EVT_DATA(evtCount)->evtOpCode);
						return;
					}
				}

				APP_VpRt_OSDPCTimer((MEDIA_EVT_DATA(evtCount)->evtOpCode == MEDIA_EVT_PLAYING_BEEP_SYNC) ? AWSMCE_VPRT_BEEP_TIMER : AWSMCE_VPRT_PLAYING_TIMER, APP_VP_RT_DIRECTLY_PLAY_TIME);

				if((MEDIA_EVT_DATA(evtCount)->overwriteTime & VOICE_PROMPT_BEFORE_RINGTONE_BIT))
				{
					APP_AudioDriver_SendSubSinkCmd(AUDIO_VP, MEDIA_EVT_DATA(evtCount)->vpIndex, MEDIA_EVT_DATA(evtCount)->evtOpCode);
					APP_AudioDriver_SendSubSinkCmd(AUDIO_RT, MEDIA_EVT_DATA(evtCount)->rtIndex, MEDIA_EVT_DATA(evtCount)->evtOpCode);
				}
				else
				{
					APP_AudioDriver_SendSubSinkCmd(AUDIO_RT, MEDIA_EVT_DATA(evtCount)->rtIndex, MEDIA_EVT_DATA(evtCount)->evtOpCode);
					APP_AudioDriver_SendSubSinkCmd(AUDIO_VP, MEDIA_EVT_DATA(evtCount)->vpIndex, MEDIA_EVT_DATA(evtCount)->evtOpCode);
				}
			}
		}
	}
	//#endif
	//#endif

	APP_Media_SendFakeMediaCmd(MEDIA_EVT_DATA(evtCount)->evtOpCode);
}

static void app_Media_SendEvtToMsgHdl(U16 msgOpCode)
{
	APP_State_SendMsgToHandler(NULL, msgOpCode, APP_FAKE_MEDIA_EVT, TRUE);
}

static void app_Media_ClearMediaEvtQ(U16 EvtIndex)
{
	U8 qCount, tarCount;

	for(qCount = 1 ; qCount < gMediaCtl.mediaQNum ; qCount++)
	{
		if(gMediaCtl.mediaQ[qCount] == EvtIndex)
		{
			gMediaCtl.mediaQNum--;
			for(tarCount = qCount ; tarCount < gMediaCtl.mediaQNum - 1; tarCount++)
			{
				gMediaCtl.mediaQ[tarCount] = gMediaCtl.mediaQ[tarCount + 1];
			}
			gMediaCtl.mediaQ[tarCount] = 0;
		}
	}
}

static void app_Media_CantFindNvKeyLogPrint(U16 id)
{
	if(id <= MEDIA_EVT_KEY_INTERNAL_TOTAL_NO)//Media evtKey
	{
		DBG_LOG_APP_SYSTEM( "[Media] Cant Find NVKey:%s", 1, APP_MediaEvent1LogString[id]);
	}
	else if((MEDIA_EVT_BAT_LOW <= id) && (id <= MEDIA_EVT_CALLER_ID))
	{
		DBG_LOG_APP_SYSTEM( "[Media] Cant Find NVKey:%s", 1, APP_MediaEvent2LogString[id-MEDIA_EVT_BAT_LOW]);
	}
	else if((MEDIA_EVT_SLC_CONNECTED <= id) && (id < MEDIA_EVT_SHARE_MODE_BASE))
	{
		DBG_LOG_APP_SYSTEM( "[Media] Cant Find NVKey:%s", 1, APP_MediaEvent3LogString[id-MEDIA_EVT_SLC_CONNECTED]);
	}
	else if((MEDIA_EVT_SHARE_MODE_BASE <= id) && (id < MEDIA_EVT_UART_CMD_RESERVE))
	{
		DBG_LOG_APP_SYSTEM( "[Media] Cant Find NVKey:%s", 1, APP_MediaEvent4LogString[id-MEDIA_EVT_SHARE_MODE_BASE]);
	}
	else
	{
		DBG_LOG_APP_SYSTEM( "[Media] Cant Find NVKey else :%x", 1, id);
	}
}

static U32 app_Media_MsgHandler(Handler handler, U16 id, void *payload, U32 id_ext)
{
	U16 evtCount;
	UNUSED(handler);
	UNUSED(payload);
	UNUSED(id_ext);
    //logPrint(LOG_OS, PRINT_LEVEL_INFO, "[HALT DBG:7 %d]", 1, id);

	switch(id)
	{
		case MEDIA_STATE_END:
			app_Media_SendEvtToMsgHdl(id_ext);
			if(gMediaCtl.mediaState == MEDIA_STATE_SYNC_PLAY)
			{
				gMediaCtl.mediaState = MEDIA_STATE_END;
			}
			else
			{
				if(gMediaCtl.mediaQ[0] != id_ext)
					break;

				app_Media_RemoveQBuf(id_ext);
				gMediaCtl.mediaState = MEDIA_STATE_END;
			}
		case MEDIA_STATE_START:
			if((gMediaCtl.mediaState == MEDIA_STATE_END || gMediaCtl.mediaState == MEDIA_STATE_SYNC_PLAY) && gMediaCtl.mediaQNum > 0)
			{
				app_Media_MediaFilterEvtHandle(gMediaCtl.mediaQ[0]);

				if(((evtCount = app_Media_FindMediaEvtCount(gMediaCtl.mediaQ[0])) >=  MEDIA_EVT_DATA_NUM)
					#ifdef LINEIN_ENABLE
					|| (APP_LINE_IN_CYCLIC_VOLUME_ENABLED_FEAT && APP_AudioDspIsStreaming(NULL,AUDIO_LINEIN))
					|| (APP_NO_CONNECTED_MEDIA_EVT_IN_LINE_IN_FEAT && (gMediaCtl.mediaQ[0] >= MEDIA_EVT_SLC_CONNECTED || gMediaCtl.mediaQ[0] <= MEDIA_EVT_4_SLC_CONNECTED))
					#endif
					)
				{
					app_Media_CantFindNvKeyLogPrint(gMediaCtl.mediaQ[0]);
					MSG_MessageSendEx(&gMediaHandler, MEDIA_STATE_END, NULL, gMediaCtl.mediaQ[0]);
                    //logPrint(LOG_OS, PRINT_LEVEL_INFO, "[HALT DBG:9]", 0);
					break;
				}

				if(APP_AudioDriver_GetSubSinkQNum() < (DRIVER_SUBSINK_Q_MAX ) && (gMediaCtl.mediaState != MEDIA_STATE_SYNC_PLAY))
				{
					gMediaCtl.mediaState = MEDIA_STATE_START;
					app_Media_SendMediaCmd(evtCount);
				}
				else
				{
					FW_SetTimer(&gMediaHandler, MEDIA_STATE_START, NULL, 0, MEDIA_EVT_DELAY_SEND_MS);
				}
			}
			break;
	}

	return 0;
}


static void app_Media_PushMediaEventLogPrint(U16 id)
{
	if(id <= MEDIA_EVT_KEY_INTERNAL_TOTAL_NO)//Media evtKey
	{
		DBG_LOG_APP_SYSTEM( "[Media]: PushMediaEvent(0x%x) %s, Qnum: %d", 3, id, APP_MediaEvent1LogString[id], gMediaCtl.mediaQNum);
	}
	else if((MEDIA_EVT_BAT_LOW <= id) && (id <= MEDIA_EVT_CALLER_ID))
	{
		DBG_LOG_APP_SYSTEM( "[Media]: PushMediaEvent(0x%x) %s, Qnum: %d", 3, id, APP_MediaEvent2LogString[id-MEDIA_EVT_BAT_LOW], gMediaCtl.mediaQNum);
	}
	else if((MEDIA_EVT_SLC_CONNECTED <= id) && (id < MEDIA_EVT_SHARE_MODE_BASE))
	{
		DBG_LOG_APP_SYSTEM( "[Media]: PushMediaEvent(0x%x) %s, Qnum: %d", 3, id, APP_MediaEvent3LogString[id-MEDIA_EVT_SLC_CONNECTED], gMediaCtl.mediaQNum);
	}
	else if((MEDIA_EVT_SHARE_MODE_BASE <= id) && (id < MEDIA_EVT_UART_CMD_RESERVE))
	{
		DBG_LOG_APP_SYSTEM( "[Media]: PushMediaEvent(0x%x) %s, Qnum: %d", 3, id, APP_MediaEvent4LogString[id-MEDIA_EVT_SHARE_MODE_BASE], gMediaCtl.mediaQNum);
	}
	else
	{
		DBG_LOG_APP_SYSTEM( "[Media]: PushMediaEvent else %x, Qnum: %d", 2, id, gMediaCtl.mediaQNum);
	}
}

static void app_Media_StopMediaEventLogPrint(U16 id)
{
	if(id <= MEDIA_EVT_KEY_INTERNAL_TOTAL_NO)//Media evtKey
	{
		DBG_LOG_APP_SYSTEM( "[Media]: Stop MediaEvent %s, Qnum: %d", 2, APP_MediaEvent1LogString[id], gMediaCtl.mediaQNum);
	}
	else if((MEDIA_EVT_BAT_LOW <= id) && (id <= MEDIA_EVT_CALLER_ID))
	{
		DBG_LOG_APP_SYSTEM( "[Media]: Stop MediaEvent %s, Qnum: %d", 2, APP_MediaEvent2LogString[id-MEDIA_EVT_BAT_LOW], gMediaCtl.mediaQNum);
	}
	else if((MEDIA_EVT_SLC_CONNECTED <= id) && (id < MEDIA_EVT_SHARE_MODE_BASE))
	{
		DBG_LOG_APP_SYSTEM( "[Media]: Stop MediaEvent %s, Qnum: %d", 2, APP_MediaEvent3LogString[id-MEDIA_EVT_SLC_CONNECTED], gMediaCtl.mediaQNum);
	}
	else if((MEDIA_EVT_SHARE_MODE_BASE <= id) && (id < MEDIA_EVT_UART_CMD_RESERVE))
	{
		DBG_LOG_APP_SYSTEM( "[Media]: Stop MediaEvent %s, Qnum: %d", 2, APP_MediaEvent4LogString[id-MEDIA_EVT_SHARE_MODE_BASE], gMediaCtl.mediaQNum);
	}
	else
	{
		DBG_LOG_APP_SYSTEM( "[Media]: Stop MediaEvent else %x, Qnum: %d", 2, id, gMediaCtl.mediaQNum);
	}
}


//#if !BRINGUP_1562
//#if !FPGA
static BOOL app_Media_IsMediaEventPlayUnderHFPState(U16 eventCode)
{
	BOOL result = FALSE;

	switch(eventCode)
	{
		case MEDIA_EVT_BAT_LOW:
		case MEDIA_EVT_BAT_FULL:
		case MEDIA_EVT_BAT_CHGCPL:
		case MEDIA_EVT_BAT_RECHG:
		case MEDIA_EVT_BAT_CHGROUT:
		case MEDIA_EVT_BAT_CHGRIN:
		case MEDIA_EVT_BAT_CHGTO:
		case MEDIA_EVT_BAT_HW_CHGTO:
		case MEDIA_EVT_BATTERY_INTERVAL_0:
		case MEDIA_EVT_BATTERY_INTERVAL_1:
		case MEDIA_EVT_BATTERY_INTERVAL_2:
		case MEDIA_EVT_BATTERY_INTERVAL_3:
		case MEDIA_EVT_BAT_LOW_LED:
		case MEDIA_EVT_BAT_LOW_RING:
		case MEDIA_EVT_BAT_SECURE_TEMP:
		case MEDIA_EVT_BAT_RISKY_TEMP:
		case MEDIA_EVT_BAT_OK:
		case MEDIA_EVT_BAT_DISCOUNT_TEMP:
			result = FALSE;
			break;

		default:
			result = TRUE;
			break;
	}

	if(!result)
	{
		DBG_LOG_APP_SYSTEM( "[Media] Media Event :%d, is not to be played under HFP state", 1, eventCode);
	}

	return result;
}
//#endif
//#endif

BOOL APP_Media_PushMediaEvent(U16 mediaEventId)
{
	if(!gMediaCtl.isDataOk)
	{
		return TRUE;
	}
	if(gMediaCtl.mediaQNum < MEDIA_EVT_Q_NUM)
	{
		app_Media_PushMediaEventLogPrint(mediaEventId);
		gMediaCtl.mediaQ[gMediaCtl.mediaQNum++] = mediaEventId;

		if(gMediaCtl.mediaQNum == 1)
		{
			FW_MessageSend(&gMediaHandler, MEDIA_STATE_START, NULL);
		}
		return TRUE;
	}
	return FALSE;
}


BOOL APP_Media_SendFakeMediaCmd(U16 mediaEventId)
{
	APP_AudioDriver_SendSubSinkCmd(AUDIO_FAKE_EVT, 0, mediaEventId);
	return FALSE;
}

BOOL APP_Media_IsSubSinkQBufFull(void)
{
	if(APP_AudioDriver_GetSubSinkQNum() >= DRIVER_SUBSINK_Q_MAX || gMediaCtl.mediaQNum >= MEDIA_EVT_Q_NUM)
		return TRUE;

	return FALSE;
}

BOOL APP_Media_IsMediaEvtLock(U16 keyEvt)
{
	U16 evtCount;

	for(evtCount = 0 ; evtCount < MEDIA_EVT_LOCK_NUM ; evtCount++)
	{
		if(MEDIA_EVT_LOCK(evtCount)->evtOpcode == gMediaCtl.mediaQ[0] && (MEDIA_EVT_LOCK(evtCount)->lockKeyEvt == keyEvt))
			return TRUE;
	}

	return FALSE;
}

void APP_Media_StopMediaEvtInternal(U16 EvtIndex)
{
	APP_AudioDriver_ClearSubSinkCmd(EvtIndex);
	app_Media_ClearMediaEvtQ(EvtIndex);
}

void APP_Media_StopMediaEvt(U16 EvtIndex)
{
	BD_ADDR_T* pBdServiceAddr = APP_GetServiceBdAddr();

	app_Media_StopMediaEventLogPrint(EvtIndex);

	if(BtAwsMce_IsDefaultRolePartner())
	{
		if(!PM_IsProfileConnected(pBdServiceAddr, PROFILE_MCSYNC) || !APP_Media_IsMediaEventAllowSync(EvtIndex))
		{
			goto VP_RT_SUBSINK_STOP_HANDLER;
		}
	}

	if(BtAwsMce_IsDefaultRoleAgent())
	{
		if(!BtAwsMce_IsWithPartner(pBdServiceAddr) || !APP_Media_IsMediaEventAllowSync(EvtIndex))
		{
			goto VP_RT_SUBSINK_STOP_HANDLER;
		}
	}

	if(app_Media_VpRtSyncStopHandler(EvtIndex))
	{
		app_Media_ClearMediaEvtQ(EvtIndex);
		return;
	}

	VP_RT_SUBSINK_STOP_HANDLER:
	APP_Media_StopMediaEvtInternal(EvtIndex);
}

void APP_Media_Init(void)
{
	if(app_Meida_ReadMediaData() && app_Meida_ReadMediaLockTbl() && app_Media_ReadMediaFilterData())
	{
		DBG_LOG_APP_SYSTEM( "[Media] Media Init Sucess", 0);
		gMediaCtl.isDataOk = TRUE;
	}

	if(gMediaCtl.isDataOk)
	{
		APP_AudioDriver_Init(&gMediaHandler, MEDIA_STATE_END);

		gMediaCtl.mediaQNum = 0;
		gMediaCtl.mediaState = MEDIA_STATE_END;
		FW_Memset(&gMediaCtl.mediaQ, 0, sizeof(U16) * MEDIA_EVT_Q_NUM);
	}
}

void APP_Media_PowerOffHandle(BOOL isReset)
{
	APP_AudioDriver_SendVoicepromptStopCmd();
	APP_AudioDriver_SendRingtoneStopCmd();
    //logPrint(LOG_OS, PRINT_LEVEL_INFO, "[HALT DBG:8]", 0);

	if(!isReset)
	{
		if(APP_Battery_IsBelowShutDownLevel())
		{
			APP_Media_PushMediaEvent(MEDIA_EVT_POWER_OFF_IN_SHUTDOWN_LOW_BAT);
		}
		else
		{
			switch(APP_ChgBat_GetLevelInQuarter())
			{
				case IND_BATTERY_LOW:
				case IND_BATTERY_INTERVAL_0:
					APP_Media_PushMediaEvent(MEDIA_EVT_POWER_OFF_UNDER_CHECKPOINT1);
					break;
				case IND_BATTERY_INTERVAL_1:
					APP_Media_PushMediaEvent(MEDIA_EVT_POWER_OFF_UNDER_CHECKPOINT2);
					break;
				case IND_BATTERY_INTERVAL_2:
					APP_Media_PushMediaEvent(MEDIA_EVT_POWER_OFF_UNDER_CHECKPOINT3);
					break;
				case IND_BATTERY_INTERVAL_3:
					APP_Media_PushMediaEvent(MEDIA_EVT_POWER_OFF);
					break;
				default:
					FW_Assert(FALSE);
					break;
			}
		}
	}
}

BOOL APP_Media_IsMediaEventAllowSync(U16 eventCode)
{
	BOOL result = FALSE;

	#if SINGLE_HEADSET_MODE_ENABLE
	if(APP_System_IsSingleHSMode())
		return FALSE;
	#endif

	switch(eventCode)
	{
		case MEDIA_EVT_POWER_ON:
		case MEDIA_EVT_POWER_ON_UNDER_CHECKPOINT1:
		case MEDIA_EVT_POWER_ON_UNDER_CHECKPOINT2:
		case MEDIA_EVT_POWER_ON_UNDER_CHECKPOINT3:
		case MEDIA_EVT_KEY_POWER_ON:
		case MEDIA_EVT_KEY_POWER_OFF:
		case MEDIA_EVT_POWER_OFF_UNDER_CHECKPOINT2:
		case MEDIA_EVT_POWER_OFF_UNDER_CHECKPOINT3:
		case MEDIA_EVT_POWER_OFF:
		case MEDIA_EVT_POWER_OFF_UNDER_CHECKPOINT1:
		case MEDIA_EVT_POWER_OFF_IN_SHUTDOWN_LOW_BAT:
		case MEDIA_EVT_SPECIAL_CONNECTED:
		case MEDIA_EVT_SPECIAL_DISCONNECTED:
		case MEDIA_EVT_SLC_CONNECTED:
		case MEDIA_EVT_2_SLC_CONNECTED:
		case MEDIA_EVT_3_SLC_CONNECTED:
		case MEDIA_EVT_4_SLC_CONNECTED:
		case MEDIA_EVT_SLC_DISCONNECTED:
		case MEDIA_EVT_BAT_LOW:
		case MEDIA_EVT_BAT_FULL:
		case MEDIA_EVT_BAT_CHGCPL:
		case MEDIA_EVT_BAT_RECHG:
		case MEDIA_EVT_BAT_CHGROUT:
		case MEDIA_EVT_BAT_CHGRIN:
		case MEDIA_EVT_BAT_CHGTO:
		case MEDIA_EVT_BAT_HW_CHGTO:
		case MEDIA_EVT_BATTERY_INTERVAL_0:
		case MEDIA_EVT_BATTERY_INTERVAL_1:
		case MEDIA_EVT_BATTERY_INTERVAL_2:
		case MEDIA_EVT_BATTERY_INTERVAL_3:
		case MEDIA_EVT_BAT_LOW_LED:
		case MEDIA_EVT_BAT_LOW_RING:
		case MEDIA_EVT_BAT_SECURE_TEMP:
		case MEDIA_EVT_BAT_RISKY_TEMP:
		case MEDIA_EVT_KEY_MEDIA_TRIGGER_1:
		case MEDIA_EVT_LEAKAGE_DETECTION:
        case MEDIA_EVT_KEY_FIND_MY_SINGLE_ACCESSORY:
        case MEDIA_EVT_ANC_CAL_SZ:
			result = FALSE;
			break;
		default:
			result = TRUE;
			break;
	}

	return result;
}

