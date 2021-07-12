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

#ifdef MTK_SWLA_ENABLE
#include "swla.h"
#include "FreeRTOS.h"
#include "memory_attribute.h"
#include "exception_handler.h"
#include "hal.h"
#include "syslog.h"
#include <stdio.h>
#include <string.h>
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define SLA_STREAM_MODE 0xa0 /* swla data will be output by log service */
#define SLA_DUMP_MODE 0xb0  /* swla data will be dumped when exception occure, only support dump mode on IOT */
#define SLA_CORE_ID 0x0     /* only single core, cm4 core */

#define SLA_NODE_SIZE 0x8   /* SWLA node size, currently is 8byte, include [context, time stamp] */

#define MAIN_VER '1'
#define SUB_VER '0'        /* SWLA implementation on IOT */

/* Private variables ---------------------------------------------------------*/
const SA_IMAGE_HEADER1 gSLA_Header1 = {
    MAIN_VER,
    SUB_VER,
    sizeof(SA_IMAGE_HEADER1) + sizeof(SA_IMAGE_HEADER2),
    0, //? main part desc len
    0, //? addon desc len
    SLA_DUMP_MODE,
    SLA_CORE_ID,
    {0, 0}, //res[2]
    0, //? MDSys US
    {0x41,0x42,0x31,0x35,0x36,0x32,0x00}, //"AB1562"
    {0x49,0x4E,0x56,0x41,0x4C,0x49,0x44}, //"INVALID"
};

ATTR_ZIDATA_IN_DRAM static uint32_t xSLA_EnableFlag = 0;
ATTR_ZIDATA_IN_DRAM static SA_NODE_t *pxSLA_Base;
ATTR_ZIDATA_IN_DRAM static uint32_t xSLA_CurIndex;
ATTR_ZIDATA_IN_DRAM static uint32_t xSLA_MaxIndex;
ATTR_ZIDATA_IN_DRAM static uint32_t xSLA_WrapFlag;

#define SLA_WORKING_BUFFER_SIZE 0xC00  //defaut 4k , please change as your request
__attribute__ ((aligned (16))) uint8_t xSLA_ARR[SLA_WORKING_BUFFER_SIZE] = {0};

/* Private functions ---------------------------------------------------------*/
static void SLA_get_region(uint32_t *pxBase, uint32_t *pxLen)
{
    *pxBase = (uint32_t)&xSLA_ARR[0];
    *pxLen = SLA_WORKING_BUFFER_SIZE;
}

static void SLA_MemoryCallbackInit(void)
{
    /* get swla region location and length according to layout */
    uint32_t xBase, xLen;
    unsigned int *pxBase, *pxEnd;
    memory_region_type region_config;

    /* get swla working buffer */
    SLA_get_region(&xBase, &xLen);
    pxBase = (unsigned int *)xBase;

    exception_printf("####SWLA enabled[0x%08X,0x%08X]####\r\n", (unsigned int)xBase, (unsigned int)xLen);

    /* update SWLA header */
    xBase += sizeof(SA_IMAGE_HEADER1);
    *((uint32_t *)(xBase + 4)) = (xSLA_CurIndex == 0) ? ((uint32_t)(pxSLA_Base + xSLA_CurIndex)) : ((uint32_t)(pxSLA_Base + xSLA_CurIndex - 1)); // Curr Position
    *((uint32_t *)(xBase + 12)) = xSLA_WrapFlag; /* wrap count */

    /* SWLA buffer valid length */
    if (xSLA_WrapFlag) {
        pxEnd = (unsigned int *)(xBase + xLen - sizeof(SA_IMAGE_HEADER1));
    } else {
        pxEnd = (unsigned int *)(pxSLA_Base + xSLA_CurIndex);
        *((uint32_t *)(xBase + 8)) = xSLA_CurIndex * sizeof(SA_NODE_t); /* raw data length */
    }

    /* register exception region to exception flow to dump swla raw data */
    region_config.region_name = "swla";
    region_config.start_address = pxBase;
    region_config.end_address = pxEnd;
    region_config.is_dumped = 1;
    exception_register_regions(&region_config);
}

static void SLA_MemoryDumpInit(void)
{
    uint32_t ret;
    exception_config_type callback_config;

    /* register callback to exception flow */
    callback_config.init_cb = SLA_MemoryCallbackInit;
    callback_config.dump_cb = NULL;

    ret = exception_register_callbacks(&callback_config);
    if (!ret) {
        configASSERT(0);
    }
}

/* Public functions ---------------------------------------------------------*/

/**
 * @brief  swla enable
 * @param[in]  none.
 * @return none
 */
void SLA_Enable(void)
{
    /* get swla region location and length according to layout */
    uint32_t xBase, xLen;
    uint32_t xTimerStamp;

    /* get swla working buffer */
    SLA_get_region(&xBase, &xLen);


    /* copy SA_IMAGE_HEADER1 to the begin of the SWLA buffer */
    //memset((void *)xBase, 0x0, xLen);
    memcpy((void *)xBase, &gSLA_Header1, sizeof(SA_IMAGE_HEADER1));

    /* to config the SWLA global variable through the SWLA area in layout */
    xBase += sizeof(SA_IMAGE_HEADER1);
    *((uint32_t *)xBase) = xBase + sizeof(SA_IMAGE_HEADER2); // Start Position
    *((uint32_t *)(xBase + 4)) = xBase + sizeof(SA_IMAGE_HEADER2); // Curr Position
    *((uint32_t *)(xBase + 8)) = xLen - gSLA_Header1.xImageHeaderLen; //Raw data length
    *((uint32_t *)(xBase + 12)) = 0; //wrap count

    /* point to the raw data area */
    xBase += sizeof(SA_IMAGE_HEADER2);
    pxSLA_Base = (SA_NODE_t *)xBase;
    xSLA_MaxIndex = ((xLen - gSLA_Header1.xImageHeaderLen) / sizeof(SA_NODE_t)) - 1;
    xSLA_CurIndex = 0;
    xSLA_WrapFlag = 0;

    /* dummy read to make gpt is enabled */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &xTimerStamp);

    /* register callback in exception handling flow to dump SWLA region */
    SLA_MemoryDumpInit();

    /* SWLA enable done */
    xSLA_EnableFlag = 1;
}

/**
 * @brief  swla logging
 * @param[in]       *Context points to the input buffer, include swla label and action
 * @return none
 */
ATTR_TEXT_IN_IRAM void SLA_RamLogging(uint32_t xContext)
{
    uint32_t xTimerStamp, xSavedMask;
    hal_nvic_save_and_set_interrupt_mask(&xSavedMask);

    /* if SWLA is not initialized, do nothing */
    if(!xSLA_EnableFlag) {
        hal_nvic_restore_interrupt_mask(xSavedMask);	
		return;
    }

    /* get time stamp */
    //hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &xTimerStamp);
    xTimerStamp = GPT(1)->GPT_COUNT;

    pxSLA_Base[xSLA_CurIndex].xContext = xContext;
    pxSLA_Base[xSLA_CurIndex].xTimeStamp = xTimerStamp;

    if (xSLA_CurIndex == xSLA_MaxIndex) {
        xSLA_WrapFlag ++;
        xSLA_CurIndex = 0;
    } else {
        xSLA_CurIndex++;
    }

    hal_nvic_restore_interrupt_mask(xSavedMask);
}

/**
 * @brief customer swla logging
 * @param[in]       *customLabel points to the input buffer
 * @param[in]       saAction swla operation, include start, stop and one-shot mode
 * @return none
 */
ATTR_TEXT_IN_IRAM void SLA_CustomLogging(const char *pxCustomLabel, SA_ACTION_t xAction)
{
    uint32_t xSavedMask, xContext;
    uint8_t *pxCustomContext;

    hal_nvic_save_and_set_interrupt_mask(&xSavedMask);
	//portENTER_CRITICAL();

    pxCustomContext = (uint8_t *)&xContext;

    /* check action */
    if ((xAction != SA_START) && (xAction != SA_STOP) && (xAction != SA_LABEL)) {
        LOG_MSGID_I(common, "[parameter error]invalid xAction:%d.\r\n",1 ,(unsigned int)xAction);
        configASSERT(0);
    }

    /* update action */
    pxCustomContext[0] = (uint8_t)xAction;

    /* only support 3-characters for customer label */
    pxCustomContext[1] = (uint8_t)pxCustomLabel[0];
    pxCustomContext[2] = (uint8_t)pxCustomLabel[1];
    pxCustomContext[3] = (uint8_t)pxCustomLabel[2];

    /* write one record to SWLA buffer */
    SLA_RamLogging((uint32_t)xContext);

    //extern void restore_interrupt_mask(uint32_t);
    hal_nvic_restore_interrupt_mask(xSavedMask);
    //portEXIT_CRITICAL();

}

#endif /* MTK_SWLA_ENABLE */
