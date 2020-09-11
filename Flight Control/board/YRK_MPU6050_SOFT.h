#ifndef __YRK_MPU6050_SOFT_H__
#define __YRK_MPU6050_SOFT_H__

#define     MPU6050_SOFT_DEVICE          I2C0        //����MPU6050_SOFT ���õĽӿ� Ϊ I2C0

#define     MPU6050_SOFT_ADRESS          (0xd0)      /*MPU6050_SOFT_Device Address*/

// MPU6050_SOFT�Ĵ�����ַ
//****************************************
#define	MPU6050_SOFT_SMPLRT_DIV		0x19	//�����ǲ����ʣ�����ֵ��0x07(125Hz)
#define	MPU6050_SOFT_CONFIG		   	0x1A	//��ͨ�˲�Ƶ�ʣ�����ֵ��0x06(5Hz)
#define	MPU6050_SOFT_GYRO_CONFIG		0x1B	//�������Լ켰������Χ������ֵ��0x18(���Լ죬2000deg/s)
#define	MPU6050_SOFT_ACCEL_CONFIG            0x1C	//���ټ��Լ졢������Χ����ͨ�˲�Ƶ�ʣ�����ֵ��0x01(���Լ죬2G��5Hz)
#define MPU6050_SOFT_I2C_MST_CTRL            0x24
#define MPU6050_SOFT_I2C_SLV0_ADDR           0x25
#define MPU6050_SOFT_I2C_SLV0_REG            0x26
#define MPU6050_SOFT_I2C_SLV0_CTRL           0x27
#define MPU6050_SOFT_INT_PIN_CFG             0x37
#define	MPU6050_SOFT_ACCEL_XOUT_H	        0x3B
#define	MPU6050_SOFT_ACCEL_XOUT_L	        0x3C
#define	MPU6050_SOFT_ACCEL_YOUT_H	        0x3D
#define	MPU6050_SOFT_ACCEL_YOUT_L	        0x3E
#define	MPU6050_SOFT_ACCEL_ZOUT_H	        0x3F
#define	MPU6050_SOFT_ACCEL_ZOUT_L	        0x40
#define	MPU6050_SOFT_TEMP_OUT_H		0x41
#define	MPU6050_SOFT_TEMP_OUT_L		0x42
#define	MPU6050_SOFT_GYRO_XOUT_H		0x43
#define	MPU6050_SOFT_GYRO_XOUT_L		0x44	
#define	MPU6050_SOFT_GYRO_YOUT_H		0x45
#define	MPU6050_SOFT_GYRO_YOUT_L		0x46
#define	MPU6050_SOFT_GYRO_ZOUT_H		0x47
#define	MPU6050_SOFT_GYRO_ZOUT_L		0x48
#define MPU6050_SOFT_USER_CTRL 		0x6A
#define	MPU6050_SOFT_PWR_MGMT_1		0x6B	//��Դ��������ֵ��0x00(��������)
#define	MPU6050_SOFT_WHO_AM_I		0x75	//IIC��ַ�Ĵ���(Ĭ����ֵ0x68��ֻ��)
#define	MPU6050_SOFT_MYSELF		0x68	//IIC��ַ�Ĵ���(Ĭ����ֵ0x68��ֻ��)
#define	MPU6050_SOFT_A_X	0x3B
#define	MPU6050_SOFT_A_Y	0x3D
#define	MPU6050_SOFT_A_Z	0x3F
#define	MPU6050_SOFT_T	0x41
#define	MPU6050_SOFT_G_X	0x43	
#define	MPU6050_SOFT_G_Y	0x45
#define	MPU6050_SOFT_G_Z	0x47

//��������
extern void  mpu6050_soft_init(void);                        //��ʼ��MPU6050_SOFT
extern void  mpu6050_soft_write_reg(uint8 reg, uint8 Data);  //дMPU6050_SOFT�Ĵ���
extern uint8 mpu6050_soft_read_reg(uint8 reg);               //��MPU6050_SOFT�Ĵ���
extern int16 mpu6050_soft_getdata(uint8 REG_Address);
#endif  //__FIRE_MPU6050_SOFT_H__