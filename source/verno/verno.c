    
#include "verno.h" 
char build_date_time_str[] = "2021/07/09 11:05:37"; 
char sw_verno_str[] = MTK_FW_VERSION; 
char hw_verno_str[] = "ab1562_evb"; 
    
#ifdef MTK_SYSLOG_VERSION_2 
#include "memory_attribute.h" 
ATTR_LOG_TIMESTAMP log_bin_build_date_time_str[] = "2021/07/09 11:05:37"; 
ATTR_LOG_VERSION log_bin_sw_verno_str[] = MTK_FW_VERSION; 
#endif 
