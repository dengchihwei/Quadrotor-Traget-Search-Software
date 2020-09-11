#include "common.h"

#include "YRK_MPU6050_SOFT.h"


#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"


#define    MPU6050_SDA                  GPIO_PIN_7
#define    MPU6050_SCL                  GPIO_PIN_6
#define    MPU6050_SDA_PORT            GPIO_PORTA_BASE
#define    MPU6050_SCL_PORT            GPIO_PORTA_BASE


#define    MPU6050_SDA_SET()            HWREG(MPU6050_SDA_PORT + (GPIO_O_DATA + (MPU6050_SDA << 2))) = MPU6050_SDA			//IIC�������Ŷ���  
#define    MPU6050_SDA_RST()            HWREG(MPU6050_SDA_PORT + (GPIO_O_DATA + (MPU6050_SDA << 2))) = 0x00			//IIC�������Ŷ���  
#define    MPU6050_SDA_IN()             HWREG(MPU6050_SDA_PORT + (GPIO_O_DATA + (MPU6050_SDA << 2)))			//IIC�������Ŷ��� 
#define    MPU6050_SCL_SET()            HWREG(MPU6050_SCL_PORT + (GPIO_O_DATA + (MPU6050_SCL << 2))) = MPU6050_SCL			//IICʱ�����Ŷ���
#define    MPU6050_SCL_RST()            HWREG(MPU6050_SCL_PORT + (GPIO_O_DATA + (MPU6050_SCL << 2))) = 0x00			//IICʱ�����Ŷ���
#define    MPU6050_Data_O()             HWREG(MPU6050_SDA_PORT + GPIO_O_DIR) = (HWREG(MPU6050_SDA_PORT + GPIO_O_DIR) |(MPU6050_SDA));
#define    MPU6050_Data_I()             HWREG(MPU6050_SDA_PORT + GPIO_O_DIR) = (HWREG(MPU6050_SDA_PORT + GPIO_O_DIR) & ~(MPU6050_SDA));

#pragma optimize=none 
void mpu6050_soft_delay()
{
 // DELAY_US(1);
  //asm("NOP");asm("NOP");//asm("NOP");asm("NOP");

//mpu6050_soft_delay();
}
//**************************************
//mpu6050_I2C��ʼ�ź�
//**************************************
#pragma optimize=none 
void mpu6050_soft_OPEN()
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    // Enable pin PA6 for GPIOOutput

    //
    ROM_GPIOPinTypeGPIOOutputOD(GPIO_PORTA_BASE, GPIO_PIN_6|GPIO_PIN_7);
    
    GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_6|GPIO_PIN_7, GPIO_STRENGTH_4MA,GPIO_PIN_TYPE_STD_WPU);//GPIO��������Ϊ������ģʽ
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6|GPIO_PIN_7,GPIO_PIN_6|GPIO_PIN_7);
}
#pragma optimize=none 
void mpu6050_I2C_Start() 
{ 
    MPU6050_SDA_SET() ;                    //����������     
    MPU6050_SCL_SET() ;                    //����ʱ����     
    mpu6050_soft_delay();              //��ʱ
     
    MPU6050_SDA_RST();                    //�����½���     
    mpu6050_soft_delay();                 //��ʱ 
      
    MPU6050_SCL_RST();                    //����ʱ���� 
} 
//**************************************
//mpu6050_I2Cֹͣ�ź�
//**************************************
#pragma optimize=none 
void mpu6050_I2C_Stop()
{
    MPU6050_SDA_RST();                    //����������
    MPU6050_SCL_SET() ;                    //����ʱ����
   mpu6050_soft_delay();                 //��ʱ
      
    MPU6050_SDA_SET() ;                    //����������
    mpu6050_soft_delay();                 //��ʱ
      
}
//**************************************
//mpu6050_I2C����Ӧ���ź�
//��ڲ���:ack (0:ACK 1:NAK)
//**************************************
#pragma optimize=none 
void mpu6050_I2C_SendACK(_Bool ack)
{
    if(ack)
    MPU6050_SDA_SET() ;
    else
    MPU6050_SDA_RST();//дӦ���ź�
    MPU6050_SCL_SET() ;                    //����ʱ����
      
    mpu6050_soft_delay();                 //��ʱ
    MPU6050_SCL_RST();                    //����ʱ����
    mpu6050_soft_delay();                 //��ʱ
      
}
//**************************************
//mpu6050_I2C����Ӧ���ź�
//**************************************
#pragma optimize=none 
uint8 mpu6050_I2C_RecvACK()
{
uint8 RF;
    MPU6050_Data_I();
    MPU6050_SCL_SET() ;
    
    MPU6050_Data_I();                    //����ʱ����
   mpu6050_soft_delay(); 
      
                    //��ʱ
    RF = MPU6050_SDA_IN();                   //��Ӧ���ź�
    MPU6050_SCL_RST();                    //����ʱ����
    mpu6050_soft_delay();
      
    MPU6050_Data_O();                 //��ʱ
    return RF;
}
//**************************************
//��mpu6050_I2C���߷���һ���ֽ�����
//**************************************
#pragma optimize=none 
void mpu6050_I2C_SendByte(uint8 dat)
{
    uint8 i;
    //DDRE=0b00001100;
    for (i=0; i<8; i++)         //8λ������
    {
        if(dat&0x80)
        MPU6050_SDA_SET() ;
        else
        MPU6050_SDA_RST();
        dat <<= 1;              //�Ƴ����ݵ����λ
                       //�����ݿ�
        MPU6050_SCL_SET() ;                //����ʱ����
        mpu6050_soft_delay();             //��ʱ
          
        MPU6050_SCL_RST();                //����ʱ����
        mpu6050_soft_delay();             //��ʱ
          
    }
    mpu6050_I2C_RecvACK();
}
//**************************************
//��mpu6050_I2C���߽���һ���ֽ�����
//**************************************
#pragma optimize=none 
uint8 mpu6050_I2C_RecvByte()
{
    uint8 i;
    uint8 dat = 0,cy;
   // DDRE=0b00001100;
    MPU6050_SDA_SET() ;  
    MPU6050_Data_I();                  //ʹ���ڲ�����,׼����ȡ����,
    for (i=0; i<8; i++)         //8λ������
    {
        dat <<= 1;
        MPU6050_SCL_SET() ;
        //DDRE=0b00000100;                 //����ʱ����
        mpu6050_soft_delay();             //��ʱ
          
        if(MPU6050_SDA_IN())
        cy=1;
        else
        cy=0;
        
        dat |= cy;             //������  
                
        MPU6050_SCL_RST();                //����ʱ����
       mpu6050_soft_delay();             //��ʱ
          
         
    }
    MPU6050_Data_O();
    return dat;
}




/*!
 *  @brief      MPU6050_SOFTд�Ĵ���
 *  @param      reg         �Ĵ���
 *  @param      dat         ��Ҫд������ݵļĴ�����ַ
 *  @since      v5.0
 *  Sample usage:       mpu6050_soft_write_reg(MPU6050_SOFT_XOFFL,0);   // д�Ĵ��� MPU6050_SOFT_XOFFL Ϊ 0
 */
void mpu6050_soft_write_reg(uint8 reg, uint8 Data)
{
    mpu6050_I2C_Start();                  //��ʼ�ź�
    mpu6050_I2C_SendByte(MPU6050_SOFT_ADRESS);   //�����豸��ַ+д�ź�
    mpu6050_I2C_SendByte(reg);    //�ڲ��Ĵ�����ַ��
    mpu6050_I2C_SendByte(Data);       //�ڲ��Ĵ������ݣ�
    mpu6050_I2C_Stop();                   //����ֹͣ�ź�
}

/*!
 *  @brief      MPU6050_SOFT���Ĵ���
 *  @param      reg         �Ĵ���
 *  @param      dat         ��Ҫ��ȡ���ݵļĴ�����ַ
 *  @since      v5.0
 *  Sample usage:       uint8 data = mpu6050_soft_read_reg(MPU6050_SOFT_XOFFL);    // ���Ĵ��� MPU6050_SOFT_XOFFL
 */
uint8 mpu6050_soft_read_reg(uint8 reg)
{
        uint8 REG_data;
	mpu6050_I2C_Start();                   //��ʼ�ź�
	mpu6050_I2C_SendByte(MPU6050_SOFT_ADRESS);    //�����豸��ַ+д�ź�
	mpu6050_I2C_SendByte(reg);     //���ʹ洢��Ԫ��ַ����0��ʼ	
	mpu6050_I2C_Start();                   //��ʼ�ź�
	mpu6050_I2C_SendByte(MPU6050_SOFT_ADRESS+1);  //�����豸��ַ+���ź�
	REG_data=mpu6050_I2C_RecvByte();       //�����Ĵ�������
	mpu6050_I2C_SendACK(1);                //����Ӧ���ź�
	mpu6050_I2C_Stop();                    //ֹͣ�ź�
	return REG_data;
}
int16 mpu6050_soft_getdata(uint8 REG_Address)
{
	uint8 H,L;
	H=mpu6050_soft_read_reg(REG_Address);
	L=mpu6050_soft_read_reg(REG_Address+1);
	return (H<<8)+L;   //�ϳ�����
}
/*!
 *  @brief      MPU6050_SOFT��ʼ�������� 2g ���̲���ģʽ
 *  @since      v5.0
 *  Sample usage:            mpu6050_soft_init();    //��ʼ�� MPU6050_SOFT
 */

void mpu6050_soft_init(void)
{
        mpu6050_soft_OPEN();        //��ʼ��mpu6050_soft�ӿ�
        mpu6050_soft_write_reg(MPU6050_SOFT_PWR_MGMT_1, 0x80);	//�������״̬
        DELAY_MS(100);

	mpu6050_soft_write_reg(MPU6050_SOFT_PWR_MGMT_1, 0x00);	//�������״̬
	DELAY_MS(1);
	 
	mpu6050_soft_write_reg(MPU6050_SOFT_SMPLRT_DIV, 0x01);
	DELAY_MS(1);
	 
	mpu6050_soft_write_reg(MPU6050_SOFT_CONFIG, 0x03);
	DELAY_MS(1);
	 
        mpu6050_soft_write_reg(MPU6050_SOFT_USER_CTRL, 0x00);
	DELAY_MS(1);
	 
        mpu6050_soft_write_reg(MPU6050_SOFT_INT_PIN_CFG, 0x02);	
	DELAY_MS(1);
	 
	mpu6050_soft_write_reg(MPU6050_SOFT_GYRO_CONFIG, 0x10);     //Range 2000d/s
	DELAY_MS(1);
	 
	mpu6050_soft_write_reg(MPU6050_SOFT_ACCEL_CONFIG, 0x01);    //Range 2g
	 
        DELAY_MS(500);
}