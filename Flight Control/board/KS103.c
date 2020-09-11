#include "KS103.h"



#include "common.h"
#include "data_common.h"
#include <math.h>


#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "stdlib.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_timer.h"
//#include "inc/hw_uart.h"
#include "inc/hw_gpio.h"
#include "inc/hw_pwm.h"
#include "inc/hw_types.h"
#include "driverlib/pin_map.h"

#include "driverlib/timer.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
//#include "driverlib/uart.h"
//#include "driverlib/udma.h"
//#include "driverlib/pwm.h"
//#include "driverlib/ssi.h"
#include "driverlib/systick.h"


//#include "utils/uartstdio.c"
#include <string.h>


#define    KS103_SDA                  GPIO_PIN_1
#define    KS103_SCL                  GPIO_PIN_0
#define    KS103_SDA_PORT            GPIO_PORTD_BASE
#define    KS103_SCL_PORT            GPIO_PORTD_BASE


#define    KS103_SDA_SET()            HWREG(KS103_SDA_PORT + (GPIO_O_DATA + (KS103_SDA << 2))) = KS103_SDA			//IIC�������Ŷ���  
#define    KS103_SDA_RST()            HWREG(KS103_SDA_PORT + (GPIO_O_DATA + (KS103_SDA << 2))) = 0x00			//IIC�������Ŷ���  
#define    KS103_SDA_IN()             HWREG(KS103_SDA_PORT + (GPIO_O_DATA + (KS103_SDA << 2)))			//IIC�������Ŷ��� 
#define    KS103_SCL_SET()            HWREG(KS103_SCL_PORT + (GPIO_O_DATA + (KS103_SCL << 2))) = KS103_SCL			//IICʱ�����Ŷ���
#define    KS103_SCL_RST()            HWREG(KS103_SCL_PORT + (GPIO_O_DATA + (KS103_SCL << 2))) = 0x00			//IICʱ�����Ŷ���
#define    KS103_SCL_HIGH()           HWREG(KS103_SCL_PORT + (GPIO_O_DATA + (KS103_SCL << 2)))	//IICʱ�����Ÿߵ�ƽ
#define    KS103_Data_O()             HWREG(KS103_SDA_PORT + GPIO_O_DIR) = (HWREG(KS103_SDA_PORT + GPIO_O_DIR) |(KS103_SDA));
#define    KS103_Data_I()             HWREG(KS103_SDA_PORT + GPIO_O_DIR) = (HWREG(KS103_SDA_PORT + GPIO_O_DIR) & ~(KS103_SDA));


float ks103_distance;//KS103��õľ��룬��λΪmm
float ks103_last_distance,ks103_delta_distance;
float fused_height,last_fused_height,delta_fused_height;
float yyy;

#pragma optimize=none 
void KS103_soft_delay()
{
  DELAY_US(8);
  asm("NOP");asm("NOP");//asm("NOP");asm("NOP");


}
//**************************************
//KS103_I2C��ʼ�ź�
//**************************************
#pragma optimize=none 
void KS103_soft_OPEN()
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    // Enable pin PD1 PD0 for GPIOOutput

    //
    ROM_GPIOPinTypeGPIOOutputOD(GPIO_PORTD_BASE, GPIO_PIN_1|GPIO_PIN_0);
    
    GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_1|GPIO_PIN_0, GPIO_STRENGTH_4MA,GPIO_PIN_TYPE_STD_WPU);//GPIO��������Ϊ������ģʽ
    GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_0|GPIO_PIN_1,GPIO_PIN_1|GPIO_PIN_0);
}
#pragma optimize=none 
void KS103_I2C_Start() 
{ 
    KS103_SDA_SET() ;                    //����������     
    KS103_SCL_SET() ;                    //����ʱ����     
    KS103_soft_delay();              //��ʱ
     
    KS103_SDA_RST();                    //�����½���     
    KS103_soft_delay();                 //��ʱ 
      
    KS103_SCL_RST();                    //����ʱ���� 
} 
//**************************************
//KS103_I2Cֹͣ�ź�
//**************************************
#pragma optimize=none 
void KS103_I2C_Stop()
{
    KS103_SDA_RST();                    //����������
    KS103_SCL_SET() ;                    //����ʱ����
   KS103_soft_delay();                 //��ʱ
      
    KS103_SDA_SET() ;                    //����������
    KS103_soft_delay();                 //��ʱ
      
}
//**************************************
//KS103_I2C����Ӧ���ź�
//��ڲ���:ack (0:ACK 1:NAK)
//**************************************
#pragma optimize=none 
void KS103_I2C_SendACK(_Bool ack)
{
    if(ack)
    KS103_SDA_SET() ;
    else
    KS103_SDA_RST();//дӦ���ź�
    KS103_SCL_SET() ;                    //����ʱ����
      
    KS103_soft_delay();                 //��ʱ
    KS103_SCL_RST();                    //����ʱ����
    KS103_soft_delay();                 //��ʱ
      
}
//**************************************
//KS103_I2C����Ӧ���ź�
//**************************************
#pragma optimize=none 
uint8 KS103_I2C_RecvACK()
{
uint8 RF;
    KS103_Data_I();
    KS103_SCL_SET() ;
    
    KS103_Data_I();                    //����ʱ����
   KS103_soft_delay(); 
      
                    //��ʱ
    RF = KS103_SDA_IN();                   //��Ӧ���ź�
    KS103_SCL_RST();                    //����ʱ����
    KS103_soft_delay();
      
    KS103_Data_O();                 //��ʱ
    return RF;
}
//**************************************
//��KS103_I2C���߷���һ���ֽ�����
//**************************************
#pragma optimize=none 
void KS103_I2C_SendByte(uint8 dat)
{
    uint8 i;
    //DDRE=0b00001100;
    for (i=0; i<8; i++)         //8λ������
    {
        if(dat&0x80)
        KS103_SDA_SET() ;
        else
        KS103_SDA_RST();
        dat <<= 1;              //�Ƴ����ݵ����λ
                       //�����ݿ�
        KS103_SCL_SET() ;                //����ʱ����
        KS103_soft_delay();             //��ʱ
          
        KS103_SCL_RST();                //����ʱ����
        KS103_soft_delay();             //��ʱ
          
    }
    KS103_I2C_RecvACK();
}
//**************************************
//��KS103_I2C���߽���һ���ֽ�����
//**************************************
#pragma optimize=none 
uint8 KS103_I2C_RecvByte()
{
    uint8 i;
    uint8 dat = 0,cy;
   // DDRE=0b00001100;
    KS103_SDA_SET() ;  
    KS103_Data_I();                  //ʹ���ڲ�����,׼����ȡ����,
    for (i=0; i<8; i++)         //8λ������
    {
        dat <<= 1;
        KS103_SCL_SET() ;
        //DDRE=0b00000100;                 //����ʱ����
        KS103_soft_delay();             //��ʱ
          
        if(KS103_SDA_IN())
        cy=1;
        else
        cy=0;
        
        dat |= cy;             //������  
                
        KS103_SCL_RST();                //����ʱ����
       KS103_soft_delay();             //��ʱ
          
         
    }
    KS103_Data_O();
    return dat;
}




/*!
 *  @brief      KS103_SOFTд�Ĵ���
 *  @param      reg         �Ĵ���
 *  @param      dat         ��Ҫд������ݵļĴ�����ַ
 *  @since      v5.0
 *  Sample usage:       KS103_soft_write_reg(KS103_SOFT_XOFFL,0);   // д�Ĵ��� KS103_SOFT_XOFFL Ϊ 0
 */
void KS103_soft_write_reg(uint8 reg, uint8 Data)
{
    KS103_I2C_Start();                  //��ʼ�ź�
    KS103_I2C_SendByte(KS103_SOFT_ADRESS);   //�����豸��ַ+д�ź�
    KS103_I2C_SendByte(reg);    //�ڲ��Ĵ�����ַ��
    KS103_I2C_SendByte(Data);       //�ڲ��Ĵ������ݣ�
    KS103_I2C_Stop();                   //����ֹͣ�ź�
}

/*!
 *  @brief      KS103_SOFT���Ĵ���
 *  @param      reg         �Ĵ���
 *  @param      dat         ��Ҫ��ȡ���ݵļĴ�����ַ
 *  @since      v5.0
 *  Sample usage:       uint8 data = KS103_soft_read_reg(KS103_SOFT_XOFFL);    // ���Ĵ��� KS103_SOFT_XOFFL
 */
uint8 KS103_soft_read_reg(uint8 reg)
{
        uint8 REG_data;
	KS103_I2C_Start();                   //��ʼ�ź�
	KS103_I2C_SendByte(KS103_SOFT_ADRESS);    //�����豸��ַ+д�ź�
	KS103_I2C_SendByte(reg);     //���ʹ洢��Ԫ��ַ����0��ʼ	
	KS103_I2C_Start();                   //��ʼ�ź�
	KS103_I2C_SendByte(KS103_SOFT_ADRESS+1);  //�����豸��ַ+���ź�
	DELAY_US(90);						//����ͨ��Э�����ӣ��������ݻ����
	REG_data=KS103_I2C_RecvByte();       //�����Ĵ�������
	KS103_I2C_SendACK(1);                //����Ӧ���ź�
	KS103_I2C_Stop();                    //ֹͣ�ź�
	return REG_data;
}
int16 KS103_soft_getdata(uint8 REG_Address)
{
	uint8 H,L;
	H=KS103_soft_read_reg(REG_Address);
	L=KS103_soft_read_reg(REG_Address+1);
	return (H<<8)+L;   //�ϳ�����
}


void ks103_handler(void)
{
	float a,b;
	uint16 temp;
	
//	GPIOIntClear(GPIO_PORTD_BASE, GPIO_PIN_0);//����жϱ�־λ
//	GPIOIntDisable(GPIO_PORTD_BASE, GPIO_PIN_0);//���ж�
    temp=KS103_soft_read_reg(2);
	temp=temp<<8;
	temp=temp+KS103_soft_read_reg(3);
	
	ks103_distance=temp;
	
	ks103_delta_distance = ks103_distance - ks103_last_distance;
	ks103_last_distance = ks103_distance;
}

/*!
 *  @brief      KS103_SOFT��ʼ�������� 2g ���̲���ģʽ
 *  @since      v5.0
 *  Sample usage:            KS103_soft_init();    //��ʼ�� KS103_SOFT
 */

void KS103_init(void)
{
        KS103_soft_OPEN();        //��ʼ��KS103_soft�ӿ�

	 	//KS103_soft_write_reg(2,0xC2);
//                KS103_soft_write_reg(2,0x9C);
//                KS103_soft_write_reg(2,0x95);
//                KS103_soft_write_reg(2,0x98);
//                KS103_soft_write_reg(2,0x73);
	 	DELAY_MS(2000);
		
}

void KS103_get_distance(void)//��ȡ�߶�
{
	
	KS103_soft_write_reg(2,0xb8);

}