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

#ifndef __AIROHA_GFPS_V2_API_H__
#define __AIROHA_GFPS_V2_API_H__

#include "bt.h"
#include "Gfp_Bt.h"

#define GFPS_BLE_ADV_DATA_LEN   0x1F
#define GSPS_BLE_IDLE           0x00
#define GFPS_BLE_ADV_STOP       0x01
#define GFPS_BLE_ADVERTISING    0x02
#define GFPS_BLE_ADV_CHANGING   0x08
#define GFPS_BLE_DISCONNECTED   0x10
#define GFPS_BLE_CONNECTED      0x12

typedef struct {
  U8 len;
  U8 adv_data[GFPS_BLE_ADV_DATA_LEN];
} gfps_ble_adv_data_t;

typedef struct _cb_app_func_module_ {
  U8 (*is_bt_discoverable)();
  void (*set_bt_discoverable)(U8 action);
  U8 (*get_bt_connected_cnt)();
  U8 (*get_bt_maximum_link)();
} app_cbfunc_module_t;

U8 GFPSv2_update_adv_data(gfps_ble_adv_data_t *p_adv);
U8 GFPSv2_update_adv_para(BLE_ADV_PARAMS_T *advParams);
U8 GFPSv2_update_scan_response(U8 *rsp);
U8 GFPSv2_set_random_addr(U8 *raddr);
U8 GFPSv2_Enable(U8 enable, app_cbfunc_module_t *module);
U8 GFPSv2_IsEnable();
U8 GFPSv2_is_processing();
void GFPSv2_store_local_passkey(U8 linkIdx, U8 *key);
void App_GfpsAdv_Disable();
void App_GfpsAdv_Update( uint8_t advType );
void App_GfpsAdv_DataParmUpdate();
void App_GfpsAdv_DataParmUpdate_2();
void App_Gfps_Spp_Disconnect();
void App_GfpsAdv_Update_BatteryLevel();
void App_gfp_set_batt_adv();
uint8_t App_gfp_parnter_batt( uint8_t *pVal );
void App_gfp_set_agent_case_LidOn();
void App_gfp_set_agent_case_LidOff();
void App_gfp_set_agent_case_Out();
uint8_t get_random_num();


U8 Gfp_GetLinkState(GFP_LINK_STRU * pLinkInfo);
GFP_LINK_STRU * Gfp_GetLinkByBdAddr(BD_ADDR_T *pBdAddr);
GFP_LINK_STRU * Gfp_GetEmptyLink(void);
U8 Gfp_GetServiceState(void);
U8 App_GfpEnableStat();


#endif //__AIROHA_GFPS_V2_API_H__

