#ifndef _AUTO_RREPORT_APP_H_
#define _AUTO_RREPORT_APP_H_
#include "auto_report.h"

/* g_frame_buf using for organize report frame,the len please modify according your system */
extern uint8_t g_frame_buf[1024];

#if MAX_ALARM_NUM
#define LOW_PROT_TEMP_ALARM              0x00 /* alarm data type,please modify according your system */                     
#define TEMP_HUMI_ALARM                  0x02 /* alarm data type,please modify according your system */      
#endif

#if MAX_SENSOR_NUM
#define TEMP_WINDOW_BIN                         5  /* sensor1 window val,please modify according your system */
#define HUMI_WINDOW_BIN                        10  /* sensor2 window val,please modify according your system */
#define TEMP_LEN                                2  /* sensor1 data len,please modify according your system */
#define HUMI_LEN                                2  /* sensor2 data len,please modify according your system */
#endif

extern int uart_write_report(const void *in,int len);
extern void init_relay_data(void);
extern void init_alarm_data(void);
extern void init_sensor_data(void);
extern void get_report_data_callback(report_type_t r_type);
extern void report_finish_refresh_infor(uint8_t *data, uint8_t len);
extern void reload_sensor_paramter(void);
extern void reload_sensor_infor(void);
#endif
