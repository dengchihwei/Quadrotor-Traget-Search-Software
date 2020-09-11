#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H
#include "common.h"
#include "data_common.h"
#include "param_common.h"
//void AttitudeInitialize();
void  power_monitor_init();
void power_monitor(uint8 chn,_Bool armed,float rate);
#endif // MOTORCONTROL_H
