#ifndef		__LOG_H__
#define		__LOG_H__

#include <stdio.h>


#define ESC_START     "\033["
#define ESC_END       "\033[0m\n"
#define COLOR_FATAL   "31;40;5m"
#define COLOR_ALERT   "31;40;1m"
#define COLOR_CRIT    "31;40;1m"
#define COLOR_ERROR   "31;40;1m"
#define COLOR_WARN    "33;40;1m"	//yellow
#define COLOR_NOTICE  "34;40;1m"	//blue
#define COLOR_INFO    "32;40;1m"	//green
#define COLOR_DEBUG   "36;40;1m"
#define COLOR_TRACE   "37;40;1m"

#define dev_info(format, args...) (printf( ESC_START "01;" COLOR_INFO "[INFO]" ESC_END format, ##args))
#define dev_err(format, args...) (printf( ESC_START "01;" COLOR_FATAL "[INFO]" ESC_END format, ##args))


#endif
