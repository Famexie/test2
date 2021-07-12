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

#if defined(PROFILE_GFP_ENABLE)
#include "Gfp_Bt.h"
#include "App_Gfp.h"
#include "gfps_api.h"
#include "App_Media.h"

extern U8 usSPPStatus;
extern U8 usBleStatus;

#define F_GFPS_ADV_ENABLE_REQ      0x08


#define F_SPP_GFP_CONNECTED         0x01
#define F_SPP_GFP_START_SER         0x02
#define F_SPP_GFP_DISCONNECTED      0x04

#define F_GFP_LEFT_EAR              0x01
#define F_GFP_RIGHT_EAR             0x02
#define GFPS_TID_RESERVE            11
#define MIN_PACKET_LEN  5

static BOOL bNextPacket = FALSE;
static U8 tempLen = 0;
static U8 storeBuf[10]={0};

#if 1
#define DBG_LOG_PROFILE_GFP(_message, arg_cnt, ...) LOG_MSGID_I(GFP, _message, arg_cnt, ##__VA_ARGS__)
#else
#define DBG_LOG_PROFILE_GFP(_message, arg_cnt, ...) printf( _message, ##__VA_ARGS__)
#endif

/**************************************************************************************************
* Prototype
**************************************************************************************************/
static U32 APP_GfpHandler(Handler handler, U16 id, void *msg, U32 handler_id);


/**************************************************************************************************
* Extern function  
**************************************************************************************************/
extern void LeGFPSv2_Init(void);
extern uint8_t* reg_get_model_id();
extern uint8_t* reg_get_ble_random_bdaddr();
extern void reg_timer_start(uint8_t id, uint32_t ms, void (*cb)(void *parm));
extern void reg_timer_stop(uint8_t id);


extern U8 BatteryLevel[3];


/**************************************************************************************************
* Static Variables
**************************************************************************************************/
typedef struct APP_GFP_CTRL_T
{
    BD_ADDR_T SppAddr;
    BOOL isSppConnected;
}APP_GFP_CTRL_T;

static APP_GFP_CTRL_T gApp_GfpCtrl =
{
    .SppAddr = {{0}},
    .isSppConnected = 0,
};

static const HandlerData gAppGfpHandler = {APP_GfpHandler};


/**************************************************************************************************
* Static Functions
**************************************************************************************************/
static void APP_Gfp_DeviceInfo(GFP_MESSAGE_STREAM_STRU *payload)
{

    switch (payload->header.code)
    {
        case GFP_CODE_CAPABILITIES:
            DBG_LOG_PROFILE_GFP("[APP] GFP_CODE_PLATFORM_TYPE:%x", 1, payload->data.deviceInfo.capabilities );
            break;

        case GFP_CODE_PLATFORM_TYPE:
            DBG_LOG_PROFILE_GFP("[APP] GFP_CODE_CAPABILITIES:%x %x", 2, payload->data.deviceInfo.platformType[0],payload->data.deviceInfo.platformType[0]);
            break;

        case GFP_CODE_ACTIVE_COMPONENTS_REQ:
        case GFP_CODE_ACTIVE_COMPONENTS_RESP:
            break;
    }

}

static void tcb_action_hdl(void *parm)
{
    UNUSED(parm);
    APP_Media_PushMediaEvent( MEDIA_EVT_RINGTONE2_IN_CALL );
    reg_timer_start( GFPS_TID_RESERVE, 2000, tcb_action_hdl );
}

void APP_Gfp_ActionHandle(U8 act )
{
    if ( act & F_GFP_LEFT_EAR )
    {
        APP_Media_PushMediaEvent( MEDIA_EVT_RINGTONE_IN_CALL );
        reg_timer_start( GFPS_TID_RESERVE, 2000, tcb_action_hdl );
    }
    else if ( act & F_GFP_RIGHT_EAR )
    {
        APP_Media_PushMediaEvent( MEDIA_EVT_RINGTONE2_IN_CALL );
        reg_timer_start( GFPS_TID_RESERVE, 2000, tcb_action_hdl );
    }
    else
    {
        reg_timer_stop( GFPS_TID_RESERVE );
    }

    APP_Gfp_SendMessageStream( GFP_GROUP_ACKNOWLEDGEMENT, GFP_CODE_ACK );
}

static void APP_Gfp_MessageStreamHandler(GFP_MESSAGE_STREAM_STRU *payload)
{
    DBG_LOG_PROFILE_GFP("[APP] Rx MessageStream Group:%x Code:%x Length:%x", 3, payload->header.group, payload->header.code, ENDIAN_TRANSFORM_U16(payload->header.length));
    switch(payload->header.group)
    {
        case GFP_GROUP_DEVICE_INFORMATION_EVENT:
            DBG_LOG_PROFILE_GFP("[APP] Rx DeviceInformation Data:%x", 1, payload->data.deviceInfo.platformType[0]);
            APP_Gfp_DeviceInfo( payload );
            break;

        case GFP_GROUP_DEVICE_ACTION_EVENT:
            DBG_LOG_PROFILE_GFP("[APP] Rx DeviceAction 0x%X", 1, payload->data.deviceInfo.activeComponentsResp );
            APP_Gfp_ActionHandle( payload->data.deviceInfo.activeComponentsResp );
            break;

        case GFP_GROUP_ACKNOWLEDGEMENT:
            DBG_LOG_PROFILE_GFP("[APP] Rx Acknowledgement Data:%x %x", 2, payload->data.acknowledgement[0], payload->data.acknowledgement[1]);
            break;
    }
}

static void APP_Gfp_SetMessageStream(U8 group, U8 code, GFP_MESSAGE_STREAM_STRU *gfpMessage)
{
    DBG_LOG_PROFILE_GFP("[APP] APP_Gfp_SetMessageStream Group:%x Code:%x", 2, group, code);
    gfpMessage->header.group = group;
    gfpMessage->header.code = code;
    switch(group)
    {
        case GFP_GROUP_BLUETOOTH_EVENT:
            gfpMessage->header.length = ENDIAN_TRANSFORM_U16(0);
            if ( code == 0x01 ) //
            {
                DBG_LOG_PROFILE_GFP("[APP] SetBluetoothEvent ,Enable Silent", 0);
            }
            if ( code == 0x02 ) //
            {
                DBG_LOG_PROFILE_GFP("[APP] SetBluetoothEvent ,Disable Silent", 0);
            }
            
            break;
        case GFP_GROUP_COMPANION_APP_EVENT:
            gfpMessage->header.length = ENDIAN_TRANSFORM_U16(0);
            DBG_LOG_PROFILE_GFP("[APP] SetCompanionAppEvent", 0);
            break;
        case GFP_GROUP_DEVICE_INFORMATION_EVENT:
            DBG_LOG_PROFILE_GFP("[APP] SetDeviceInfo", 0);
            if(code == GFP_CODE_MODEL_ID)
            {
                gfpMessage->header.length = ENDIAN_TRANSFORM_U16(3);

                memcpy( &gfpMessage->data.deviceInfo.modelID[0], reg_get_model_id() , 3);

                DBG_LOG_PROFILE_GFP("[APP] ModelID:%x %x %x", 3, gfpMessage->data.deviceInfo.modelID[0], gfpMessage->data.deviceInfo.modelID[1], gfpMessage->data.deviceInfo.modelID[2]);
            }
            if(code == GFP_CODE_BLE_ADDRESS_UPDATED)
            {
                gfpMessage->header.length = ENDIAN_TRANSFORM_U16(6);
                memcpy( gfpMessage->data.deviceInfo.bleAddr, reg_get_ble_random_bdaddr() , 6 );
            }
            if(code == GFP_CODE_BATTERY_UPDATED)
            {
                gfpMessage->header.length = ENDIAN_TRANSFORM_U16(3);

                gfpMessage->data.deviceInfo.battery[0] = BatteryLevel[0];
                gfpMessage->data.deviceInfo.battery[1] = BatteryLevel[1];
                gfpMessage->data.deviceInfo.battery[2] = BatteryLevel[2];
                
                DBG_LOG_PROFILE_GFP("[APP] Battery:%x %x %x", 3, gfpMessage->data.deviceInfo.battery[0], gfpMessage->data.deviceInfo.battery[1], gfpMessage->data.deviceInfo.battery[2]);
            }
            if(code == GFP_CODE_REMAINING_BATTERY_TIME)
            {
                gfpMessage->header.length = ENDIAN_TRANSFORM_U16(1);
                gfpMessage->data.deviceInfo.remainingTime = 0x1E;
                DBG_LOG_PROFILE_GFP("[APP] Remaining Time", 1, gfpMessage->data.deviceInfo.remainingTime);
            }
            if(code == GFP_CODE_ACTIVE_COMPONENTS_RESP)
            {
                gfpMessage->header.length = ENDIAN_TRANSFORM_U16(1);
                gfpMessage->data.deviceInfo.activeComponentsResp = 0x03;
                DBG_LOG_PROFILE_GFP("[APP] Active Components Resp:%x", 1, gfpMessage->data.deviceInfo.activeComponentsResp);
            }
            if(code == GFP_CODE_CAPABILITIES)
            {
                gfpMessage->header.length = ENDIAN_TRANSFORM_U16(1);
                DBG_LOG_PROFILE_GFP("[APP] Capabilities:%x", 1, gfpMessage->data.deviceInfo.capabilities);
            }
            if(code == GFP_CODE_PLATFORM_TYPE)
            {
                gfpMessage->header.length = ENDIAN_TRANSFORM_U16(2);
                DBG_LOG_PROFILE_GFP("[APP] PlayformType:%x", 3, gfpMessage->data.deviceInfo.platformType[0],gfpMessage->data.deviceInfo.platformType[1]);
            }
            break;
            
        case GFP_GROUP_DEVICE_ACTION_EVENT:
            gfpMessage->header.length = ENDIAN_TRANSFORM_U16(2);
            gfpMessage->data.deviceRing.ringing[0] = 0x03;
            gfpMessage->data.deviceRing.ringing[1] = 0x3C;
            if ( code == GFP_CODE_RING )
            {
                DBG_LOG_PROFILE_GFP("[APP] DeviceAction , Code =%x", 1 , gfpMessage->header.code );
            }
            break;
            
        case GFP_GROUP_ACKNOWLEDGEMENT:
        {
            GFP_ACK_T ack;

            gfpMessage->header.length = ENDIAN_TRANSFORM_U16(2);
            DBG_LOG_PROFILE_GFP("[APP] DeviceAction , Code =%x", 1 , gfpMessage->header.code );

            ack.action = GFP_GROUP_DEVICE_ACTION_EVENT;
            ack.groupID = group;
            ack.codeID = GFP_CODE_ACK;
            
            if ( gfpMessage->header.code == GFP_CODE_ACK )
            {
                gfpMessage->data.acknowledgement[0] = ack.action;
                gfpMessage->data.acknowledgement[1] = ack.codeID;
            }
            else if ( gfpMessage->header.code == GFP_CODE_NAK )
            {
                gfpMessage->header.length = ENDIAN_TRANSFORM_U16(3);
                ack.reason = GFP_ACKNOLEGEMENT_NAK_REASON_NOT_SUPPORTED;
                gfpMessage->data.acknowledgement[0] = ack.reason;
                gfpMessage->data.acknowledgement[1] = ack.groupID;
                gfpMessage->data.acknowledgement[2] = ack.codeID;
            }

            DBG_LOG_PROFILE_GFP("[APP] Acknowledgement:%x %x %x", 3, gfpMessage->data.acknowledgement[0], gfpMessage->data.acknowledgement[1],gfpMessage->data.acknowledgement[2]);
        }
        break;
    }
}

static void APP_Gfp_RxDataHandler(GFP_RX_DATA_IND_T * ind)
{
    if ( bNextPacket )
    {
        memcpy( &storeBuf[tempLen], &ind->data[0], ind->length );
        APP_Gfp_MessageStreamHandler((GFP_MESSAGE_STREAM_STRU *)&storeBuf[0] );
        tempLen = 0;
        bNextPacket = FALSE;
        return;
    
    }
    if ( ind->length < MIN_PACKET_LEN )
    {
        bNextPacket = TRUE;
        tempLen = ind->length;
        memcpy( &storeBuf[0], &ind->data[0], tempLen);
        return;
    }

    
    if(ind->length >= sizeof(GFP_MESSAGE_STREAM_HEADER_T))
    {
        tempLen = 0;
        bNextPacket = 0;
        DBG_LOG_PROFILE_GFP("[APP] Rx Data:%x %x %x %x", 4, ind->data[0], ind->data[1], ind->data[2], ind->data[3]);
        APP_Gfp_MessageStreamHandler((GFP_MESSAGE_STREAM_STRU *)&ind->data);
    }
}

static void APP_Gfp_TxAvailableIndHandler(GFP_TX_AVAILABLE_IND_T * ind)
{
    UNUSED(ind);
//	DBG_LOG_PROFILE_GFP("[APP] Tx Available Ind: BdAddr:0x%x%x", 2, FW_bdaddr_to_2U32(&ind->bdAddr, TRUE), FW_bdaddr_to_2U32(&ind->bdAddr, FALSE));
}

static void APP_Gfp_StartServiceCfmHandler(GFP_START_SERVICE_CFM_T * cfm)
{
    DBG_LOG_PROFILE_GFP("[APP] Start Service Cfm: %d", 1, cfm->status);
}

static void APP_Gfp_StopServiceCfmHandler(GFP_STOP_SERVICE_CFM_T * cfm)
{
    DBG_LOG_PROFILE_GFP("[APP] Stop Service Cfm: %d", 1, cfm->status);
}


void APP_Gfp_InformationUpdate(uint8_t gfp_code )
{
    if ( !gApp_GfpCtrl.isSppConnected )  return;

    APP_Gfp_SendMessageStream(GFP_GROUP_DEVICE_INFORMATION_EVENT, gfp_code);
}

static void APP_Gfp_ConnectCfmHandler(GFP_CONNECT_CFM_T * cfm)
{
    DBG_LOG_PROFILE_GFP("[APP] Connect Cfm: BdAddr:0x%x%x, status: %d", 3, FW_bdaddr_to_2U32(&cfm->bdAddr, TRUE), FW_bdaddr_to_2U32(&cfm->bdAddr, FALSE), cfm->status);

    gApp_GfpCtrl.isSppConnected = TRUE;
    memcpy(&gApp_GfpCtrl.SppAddr, &cfm->bdAddr, sizeof(BD_ADDR_T));
 
    // For Retroactively Writing Account Key
    APP_Gfp_SendMessageStream(GFP_GROUP_DEVICE_INFORMATION_EVENT, GFP_CODE_MODEL_ID);
    APP_Gfp_SendMessageStream(GFP_GROUP_DEVICE_INFORMATION_EVENT, GFP_CODE_BLE_ADDRESS_UPDATED);
//    APP_Gfp_SendMessageStream(GFP_GROUP_DEVICE_INFORMATION_EVENT, GFP_CODE_CAPABILITIES);

    APP_Gfp_SendMessageStream(GFP_GROUP_DEVICE_INFORMATION_EVENT, GFP_CODE_BATTERY_UPDATED);

    APP_Gfp_SendMessageStream( GFP_GROUP_DEVICE_ACTION_EVENT, GFP_CODE_RING );

    App_GfpsAdv_DataParmUpdate();
}

static void APP_Gfp_DisconnectCfmHandler(GFP_DISCONNECT_CFM_T * cfm)
{
    DBG_LOG_PROFILE_GFP("[APP] Disconnect Cfm: BdAddr:0x%x%x, status: %d", 3, FW_bdaddr_to_2U32(&cfm->bdAddr, TRUE), FW_bdaddr_to_2U32(&cfm->bdAddr, FALSE), cfm->status);

    gApp_GfpCtrl.isSppConnected = FALSE;
    memset(&gApp_GfpCtrl.SppAddr, 0, sizeof(BD_ADDR_T));
    App_Gfps_Spp_Disconnect();
}

static U32 APP_GfpHandler(Handler handler, U16 id, void *msg, U32 handler_id)
{
    UNUSED(handler);
    UNUSED(handler_id);

    switch(id)
    {
        case GFP_RX_DATA_IND:
            APP_Gfp_RxDataHandler((GFP_RX_DATA_IND_T *)msg);
            break;
        case GFP_TX_AVAILABLE_IND:
            APP_Gfp_TxAvailableIndHandler((GFP_TX_AVAILABLE_IND_T *)msg);
            break;
        case GFP_START_SERVICE_CFM:
            APP_Gfp_StartServiceCfmHandler((GFP_START_SERVICE_CFM_T *)msg);
            break;
        case GFP_STOP_SERVICE_CFM:
            APP_Gfp_StopServiceCfmHandler((GFP_STOP_SERVICE_CFM_T *)msg);
            break;
        case GFP_CONNECT_CFM:
            usSPPStatus |= F_SPP_GFP_CONNECTED;
            usSPPStatus &= ~F_SPP_GFP_DISCONNECTED;
            APP_Gfp_ConnectCfmHandler((GFP_CONNECT_CFM_T *)msg);
            break;
        case GFP_DISCONNECT_CFM:
            usSPPStatus &= ~F_SPP_GFP_CONNECTED ;
            usSPPStatus |= F_SPP_GFP_DISCONNECTED;
            usBleStatus &= ~F_GFPS_ADV_ENABLE_REQ;
            APP_Gfp_DisconnectCfmHandler((GFP_DISCONNECT_CFM_T *)msg);
            break;
        default:
            break;
    }
    return 0;
}


/**************************************************************************************************
* Public Functions
**************************************************************************************************/
void APP_Gfp_RegisterProfile(void)
{
    if ( GFPSv2_IsEnable() == 0 ) 
    {
        DBG_LOG_PROFILE_GFP("[APP] RegisterService return", 0);
        return;
    }

    Gfp_StartService((Handler)&gAppGfpHandler, GFP_CHANNEL);
    DBG_LOG_PROFILE_GFP("[APP] RegisterService", 0);
    //BLE  service Init
    LeGFPSv2_Init();
}

void APP_Gfp_SendMessageStream(U8 group, U8 code)
{
    U16 length;
    GFP_MESSAGE_STREAM_STRU *gfpMessage = (GFP_MESSAGE_STREAM_STRU *)FW_GetMemory(sizeof(GFP_MESSAGE_STREAM_STRU));
    
    APP_Gfp_SetMessageStream(group, code, gfpMessage);
    length  = sizeof(GFP_MESSAGE_STREAM_HEADER_T) + ENDIAN_TRANSFORM_U16(gfpMessage->header.length);
    if(Gfp_TxData(&gApp_GfpCtrl.SppAddr, length, (U8 *)gfpMessage))
        DBG_LOG_PROFILE_GFP("[APP] Send MessageStream Group:%x Code:%x Length:%x SUCCESS", 3, group, code, length);
    else
        DBG_LOG_PROFILE_GFP("[APP] Send MessageStream Group:%x Code:%x Length:%x FAIL", 3, group, code, length);

    FW_FreeMemory( gfpMessage );

}
#endif
