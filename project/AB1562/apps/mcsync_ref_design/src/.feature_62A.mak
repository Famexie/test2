################################################################################
# NVKEY control
################################################################################
NVKEY_XML       =  nvkey.xml
NVKEY_FOTA_XML  =  nvkey_fota.xml
SYSKEY_FOTA_XML =  AB1530B_syskey_fota.xml
SYSKEY_XML      =  AB1530B_syskey.xml

################################################################################
# Feature control
################################################################################

################################################################################
# Logging config
################################################################################
MTK_SYSLOG_VERSION_2                                    ?= y
MTK_DEBUG_PLAIN_LOG_ENABLE                              ?= n
MTK_SYSLOG_SUB_FEATURE_STRING_LOG_SUPPORT                = y
MTK_SYSLOG_SUB_FEATURE_BINARY_LOG_SUPPORT                = y
MTK_SYSLOG_SUB_FEATURE_OFFLINE_DUMP_ACTIVE_MODE          = n
MTK_SYSLOG_SUB_FEATURE_MSGID_TO_STRING_LOG_SUPPORT      ?= n

MTK_MUX_ENABLE                                          ?= y
MTK_PORT_SERVICE_ENABLE                                 ?= y
MTK_CPU_NUMBER_0                                        ?= y
FPGA_ENV                                                ?= n

################################################################################
# Exception config
################################################################################
MTK_FULLDUMP_ENABLE                                     ?= y
MTK_MINIDUMP_ENABLE                                     ?= y

################################################################################
# Bootreason config
################################################################################
MTK_BOOTREASON_CHECK_ENABLE                             ?= y

################################################################################
# System hang tracer
################################################################################
MTK_SYSTEM_HANG_TRACER_ENABLE                           ?= y

################################################################################
# Audio Feature config
################################################################################
MTK_INEAR_ENHANCEMENT                                   ?= n
MTK_DUALMIC_INEAR                                       ?= n
MTK_3RD_PARTY_NR                                        ?= n
MTK_OPUS_ENCODER_ENABLE                                 ?= n
MTK_DSP_HWVAD_ENABLE                                    ?= y
MTK_LEAKAGE_DETECTION_ENABLE                            ?= y
MTK_DSP_MULTI_MIC_ENABLE                                ?= y
MTK_LOW_LATENCY_VP_WITH_48K_FS                          ?= n
MP3_VOICE_PROMPT_AUDIO_CTRL                             ?= y
AIROHA_WWE_ENABLE                                       ?= y
AMAZON_AMA_ENABLE                                       ?= n
GOOGLE_GVA_ENABLE                                       ?= n
XIAOAI_DEVELOPMENT                                      ?= n
XIAOAI_TWO_STAGE_OTA                                    ?= n
MP3_VOICE_PROMPT_AUDIO_CTRL                             ?= n
################################################################################
# Profile Feature config
################################################################################
PROFILE_AMA_ENABLE                                      ?= n
PROFILE_GFP_ENABLE                                      ?= y
PROFILE_SWIFT_ENABLE                                    ?= n
PROFILE_HEADSET_ENABLE                                  ?= y
AMA_IAP2_SUPPORT_ENABLE                                 ?= n
################################################################################
# Project Feature config
################################################################################
APP_GAME_MODE                                           ?= y
A2DP_LOW_LATENCY                                        ?= n
MODULE_LOG_FEATURE                                      ?= y
A2DP_PACKET_ASSEMBLE                                    ?= y
LOG_PRINT_156X                                          ?= n
AVRCP_Profile                                           ?= y
A2DP_Profile                                            ?= y
SMART_CHG_CSE_DEVELOPMENT                               ?= y
SMART_CHG_CSE_LIDON_RECONNECT                           ?= y
SMART_CHG_CSE_AUTO_AIR_PAIRING                          ?= n
#SMART_CHARGING_RECOVERY_MODE                           ?= y
MP3_LOCAL_PLAYBACK                                      ?= n
MP3_LOCAL_PLAYBACK_MMI_CTRL                             ?= n
CHANNEL_SELECTION_ENABLE                                ?= y
CAPTOUCH_FUNCTION_ENABLE                                ?= y
ANC_FF_MMI_CTRL                                         ?= y
APP_OPUS_ENCODER                                        ?= n
AIROHA_BT_SPP_ENABLE                                    ?= n
AIROHA_BT_HID_ENABLE                                    ?= n
AIROHA_BT_LE_ENABLE                                     ?= n
DO_RHO_IMMEDIATELY                                      ?= n
CODEC_SWITCH_WORKAROUND_MI9                             ?= n
SPECIAL_WHITE_LIST_HANDLE                               ?= y
CODEC_SWITCH_WORKAROUND_SONY_XPERIA                     ?= y
MCSYNC_SHARE_MODE                                       ?= n
TAKE_OVER_LINK                                          ?= n
APP_TILE_ENABLE                                         ?= n
PRESSURE_SENSOR                                         ?= n
DISCONNECT_AGENT_RECOVERY                               ?= n
