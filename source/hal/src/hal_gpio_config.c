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

#include "hal_gpio_config.h"
#include "nvkey.h"
#include "nvkey_list.h"
#include <string.h>

#define GPIO_PIN_MASK  (0x0003FFFF)
const uint32_t GPIO_CONFIG_PIN_MASK[1] = {GPIO_PIN_MASK};

hal_gpio_pin_t hal_gpio_get_pin_num_from_gpio_var_name(gpio_var_name_t gpio_var_name)
{
    uint32_t i = 0;
    uint32_t len = 0;
    gpio_nvkey_t gpio_nvkey;

    if (gpio_var_name >= GPIO_VAR_NAME_MAX) {
        //printf("READ GPIO_%d NVKEY, wrong parameter  \r\n", i);
        return HAL_GPIO_MAX;
    }

    len = NVKEY_ReadFullKey(NVKEYID_GPIO_PIN_SETTING, &gpio_nvkey, sizeof(gpio_nvkey));
    if (len == 0) {
        //printf("READ GPIO_%d NVKEY fail \r\n", i);
        return HAL_GPIO_MAX;
    }
    for (i = 0; i < HAL_GPIO_MAX; i++) {
        if (GPIO_CONFIG_PIN_MASK[i >> 5] & (0x01 << (i & 0x1f))) {
            if (gpio_nvkey.gpio_info[i].gpio_var_name == gpio_var_name) {
                return i;
            }
        }
    }

    return HAL_GPIO_MAX;
}

hal_gpio_pin_t hal_gpio_get_pin_num_from_eint_var_name(eint_var_name_t eint_var_name)
{
    uint32_t i = 0;
    uint32_t len = 0;
    gpio_nvkey_t gpio_nvkey;

    if (eint_var_name >= EINT_VAR_NAME_MAX) {
        //printf("READ GPIO_%d NVKEY, wrong parameter  \r\n", i);
        return HAL_GPIO_MAX;
    }

    len = NVKEY_ReadFullKey(NVKEYID_GPIO_PIN_SETTING, &gpio_nvkey, sizeof(gpio_nvkey));
    if (len == 0) {
        //printf("READ GPIO_%d NVKEY fail \r\n", i);
        return HAL_GPIO_MAX;
    }
    for (i = 0; i < HAL_GPIO_MAX; i++) {
        if (GPIO_CONFIG_PIN_MASK[i >> 5] & (0x01 << (i & 0x1f))) {
            if (gpio_nvkey.gpio_info[i].eint_var_name == eint_var_name) {
                return i;
            }
        }
    }

    return HAL_GPIO_MAX;
}

hal_eint_number_t hal_gpio_get_eint_num_from_eint_var_name(eint_var_name_t eint_var_name)
{
    uint32_t i = 0;
    uint32_t len = 0;
    gpio_nvkey_t gpio_nvkey;

    if (eint_var_name >= EINT_VAR_NAME_MAX) {
        //printf("READ GPIO_%d NVKEY, wrong parameter  \r\n", i);
        return HAL_GPIO_MAX;
    }

    len = NVKEY_ReadFullKey(NVKEYID_GPIO_PIN_SETTING, &gpio_nvkey, sizeof(gpio_nvkey));
    if (len == 0) {
        //printf("READ GPIO_%d NVKEY fail \r\n", i);
        return HAL_GPIO_MAX;
    }
    for (i = 0; i < HAL_GPIO_MAX; i++) {
        if (GPIO_CONFIG_PIN_MASK[i >> 5] & (0x01 << (i & 0x1f))) {
            if (gpio_nvkey.gpio_info[i].eint_var_name == eint_var_name) {
                return (hal_eint_number_t)i;
            }
        }
    }

    return HAL_GPIO_MAX;
}

gpio_var_name_t hal_gpio_get_gpio_var_name(hal_gpio_pin_t gpio_num)
{
    uint32_t len = 0;
    gpio_nvkey_t gpio_nvkey;

    if (gpio_num >= HAL_GPIO_MAX) {
        //printf("READ GPIO_%d NVKEY, wrong parameter  \r\n", i);
        return GPIO_VAR_NAME_NOT_DEFINED;
    }

    len = NVKEY_ReadFullKey(NVKEYID_GPIO_PIN_SETTING, &gpio_nvkey, sizeof(gpio_nvkey));
    if (len == 0) {
        //printf("READ GPIO_%d NVKEY fail \r\n", i);
        return GPIO_VAR_NAME_NOT_DEFINED;
    }

    return gpio_nvkey.gpio_info[gpio_num].gpio_var_name;
}


hal_gpio_status_t hal_gpio_get_all_pin_config_from_eint_var_name(gpio_nvkey_t *all_gpio_nvkey)
{
    uint32_t len = 0;
    gpio_nvkey_t gpio_nvkey;

    if (all_gpio_nvkey == NULL) {
        return HAL_GPIO_STATUS_INVALID_PARAMETER;
    }

    len = NVKEY_ReadFullKey(NVKEYID_GPIO_PIN_SETTING, &gpio_nvkey, sizeof(gpio_nvkey));
    if (len == 0) {
        //printf("READ GPIO_%d NVKEY fail \r\n", i);
        return HAL_GPIO_STATUS_ERROR;
    }
    *all_gpio_nvkey = gpio_nvkey;

    return HAL_GPIO_STATUS_OK;
}


hal_gpio_status_t hal_gpio_set_output_with_gpio_var_name(gpio_var_name_t gpio_var_name, hal_gpio_data_t gpio_data)
{
    hal_gpio_pin_t pin;

	pin = hal_gpio_get_pin_num_from_gpio_var_name(gpio_var_name);
	if (pin == HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR;
	}

	return hal_gpio_set_output(pin, gpio_data);
}



void gpio_config_all(void)
{
    uint32_t i = 0;
    uint32_t len = 0;
    gpio_nvkey_t gpio_nvkey;

    len = NVKEY_ReadFullKey(NVKEYID_GPIO_PIN_SETTING, &gpio_nvkey, sizeof(gpio_nvkey));
    if (len == 0) {
        //printf("READ GPIO_%d NVKEY fail \r\n", i);
        return;
    }

    for (i = 0; i < HAL_GPIO_MAX; i++) {
        if (GPIO_CONFIG_PIN_MASK[i >> 5] & (0x01 << (i & 0x1f))) {
            hal_pinmux_set_function(i, gpio_nvkey.gpio_info[i].gpio_pinmux);
            hal_gpio_set_direction(i, gpio_nvkey.gpio_info[i].gpio_direction);
            if (gpio_nvkey.gpio_info[i].gpio_direction == HAL_GPIO_DIRECTION_INPUT) {
                if (gpio_nvkey.gpio_info[i].gpio_pupd == GPIO_PULL_UP) {
                    hal_gpio_pull_up(i);
                } else if (gpio_nvkey.gpio_info[i].gpio_pupd == GPIO_PULL_DOWN) {
                    hal_gpio_pull_down(i);
                } else if (gpio_nvkey.gpio_info[i].gpio_pupd == GPIO_PULL_DISABLE) {
                    hal_gpio_disable_pull(i);
                }
            } else {
                hal_gpio_set_output(i, gpio_nvkey.gpio_info[i].gpio_output);
            }
        }
    }
}


#ifdef HAL_RTC_FEATURE_GPIO
extern hal_rtc_status_t hal_rtc_gpio_init(hal_rtc_gpio_config_t *gpio_config);
#endif
void    rtc_gpio_config_all(void)
{
#ifdef HAL_RTC_FEATURE_GPIO
    uint32_t i      = 0;
    uint32_t len    = 0;
    uint8_t  gpio_x = 0;
    gpio_nvkey_t gpio_nvkey;

    len = NVKEY_ReadFullKey(NVKEYID_GPIO_PIN_SETTING, &gpio_nvkey, sizeof(gpio_nvkey));
    if (len == 0) {
        //printf("READ GPIO_%d NVKEY fail \r\n", i);
        return;
    }
    for (i = HAL_GPIO_MAX; i < (HAL_GPIO_MAX + RTC_GPIO_NUM); i++) {
        hal_rtc_gpio_config_t   gpio_cfg;

        memset(&gpio_cfg, 0, sizeof(gpio_cfg));
        gpio_cfg.rtc_gpio = gpio_x;
        if(gpio_nvkey.gpio_info[i].gpio_pinmux == 2){
            gpio_cfg.is_analog = true;
        } else {
            gpio_cfg.is_analog = false;
        }
        if(gpio_nvkey.gpio_info[i].gpio_direction == HAL_GPIO_DIRECTION_INPUT){
            gpio_cfg.is_input = true;
        }else {
            gpio_cfg.is_input = false;
        }
        if(gpio_nvkey.gpio_info[i].gpio_pupd == GPIO_PULL_UP){
            gpio_cfg.is_pull_up = true;
        }else{
            gpio_cfg.is_pull_up = false;
        }
        if(gpio_nvkey.gpio_info[i].gpio_pupd == GPIO_PULL_DOWN){
            gpio_cfg.is_pull_down = true;
        }else{
            gpio_cfg.is_pull_down = false;
        }
        hal_rtc_gpio_init(&gpio_cfg);
        gpio_x++;
    }
#endif
}
   

