#ifndef _KS103_H
#define _KS103_H

#include "common.h"

#define PI 3.1416
#define     KS103_SOFT_DEVICE          I2C3        //����ks103 ���õĽӿ� Ϊ I2C3

#define     KS103_SOFT_ADRESS          (0xE8)      /*KS103Device Address*/

extern float ks103_distance,ks103_delta_distance;//KS103��õľ��룬��λΪmm

// ks103�Ĵ�����ַ
//****************************************


//��������
extern void  KS103_init(void);                        //��ʼ��MPU6050_SOFT
extern void  KS103_soft_write_reg(uint8 reg, uint8 Data);  //дMPU6050_SOFT�Ĵ���
extern uint8 KS103_soft_read_reg(uint8 reg);               //��MPU6050_SOFT�Ĵ���
extern int16 KS103_soft_getdata(uint8 REG_Address);//�����ֽ�����
extern void KS103_get_distance(void);//��ȡ�߶�




#endif