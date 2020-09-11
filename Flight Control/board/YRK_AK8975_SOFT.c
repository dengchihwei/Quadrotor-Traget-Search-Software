#include "common.h"
 
#include "YRK_AK8975_SOFT.h"

#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"


#define    AK8975_SDA                  GPIO_PIN_7
#define    AK8975_SCL                  GPIO_PIN_6
#define    AK8975_SDA_POART            GPIO_PORTA_BASE
#define    AK8975_SCL_POART            GPIO_PORTA_BASE


#define    AK8975_SDA_SET()            HWREG(AK8975_SDA_POART + (GPIO_O_DATA + (AK8975_SDA << 2))) = AK8975_SDA			//IIC�������Ŷ���  
#define    AK8975_SDA_RST()            HWREG(AK8975_SDA_POART + (GPIO_O_DATA + (AK8975_SDA << 2))) = 0x00			//IIC�������Ŷ���  
#define    AK8975_SDA_IN()             HWREG(AK8975_SDA_POART + (GPIO_O_DATA + (AK8975_SDA << 2)))			//IIC�������Ŷ��� 
#define    AK8975_SCL_SET()            HWREG(AK8975_SCL_POART + (GPIO_O_DATA + (AK8975_SCL << 2))) = AK8975_SCL			//IICʱ�����Ŷ���
#define    AK8975_SCL_RST()            HWREG(AK8975_SCL_POART + (GPIO_O_DATA + (AK8975_SCL << 2))) = 0x00			//IICʱ�����Ŷ���
#define    AK8975_Data_O()             HWREG(AK8975_SDA_POART + GPIO_O_DIR) = (HWREG(AK8975_SDA_POART + GPIO_O_DIR) |(AK8975_SDA));
#define    AK8975_Data_I()             HWREG(AK8975_SDA_POART + GPIO_O_DIR) = (HWREG(AK8975_SDA_POART + GPIO_O_DIR) & ~(AK8975_SDA));

#pragma optimize=none
void ak8975_soft_delay()
{
asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
asm("NOP");asm("NOP");

//asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
//asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");

//AK8975_delay();
}
//**************************************
//ak8975_soft_I2C��ʼ�ź�
//**************************************
#pragma optimize=none
void ak8975_soft_OPEN()
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    // Enable pin PA6 for GPIOOutput

    //
    ROM_GPIOPinTypeGPIOOutputOD(GPIO_PORTA_BASE, GPIO_PIN_6|GPIO_PIN_7);
    
    GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_6|GPIO_PIN_7, GPIO_STRENGTH_4MA,GPIO_PIN_TYPE_STD_WPU);//GPIO��������Ϊ������ģʽ
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6|GPIO_PIN_7,GPIO_PIN_6|GPIO_PIN_7);
}
#pragma optimize=none
void ak8975_soft_I2C_Start() 
{ 
    AK8975_SDA_SET() ;                    //����������     
    AK8975_SCL_SET() ;                    //����ʱ����     
    ak8975_soft_delay();              //��ʱ
     
    AK8975_SDA_RST();                    //�����½���     
    ak8975_soft_delay();                 //��ʱ 
      
    AK8975_SCL_RST();                    //����ʱ���� 
} 
//**************************************
//ak8975_soft_I2Cֹͣ�ź�
//**************************************
#pragma optimize=none
void ak8975_soft_I2C_Stop()
{
    AK8975_SDA_RST();                    //����������
    AK8975_SCL_SET() ;                    //����ʱ����
    ak8975_soft_delay();                 //��ʱ
      
    AK8975_SDA_SET() ;                    //����������
    ak8975_soft_delay();                 //��ʱ
      
}
//**************************************
//ak8975_soft_I2C����Ӧ���ź�
//��ڲ���:ack (0:ACK 1:NAK)
//**************************************
#pragma optimize=none
void ak8975_soft_I2C_SendACK(_Bool ack)
{
    if(ack)
    AK8975_SDA_SET() ;
    else
    AK8975_SDA_RST();//дӦ���ź�
    AK8975_SCL_SET() ;                    //����ʱ����
      
    ak8975_soft_delay();                 //��ʱ
    AK8975_SCL_RST();                    //����ʱ����
    ak8975_soft_delay();                 //��ʱ
      
}
//**************************************
//ak8975_soft_I2C����Ӧ���ź�
//**************************************
#pragma optimize=none
uint8 ak8975_soft_I2C_RecvACK()
{
uint8 RF;
    AK8975_Data_I();
    AK8975_SCL_SET() ;
    
    AK8975_Data_I();                    //����ʱ����
    ak8975_soft_delay(); 
      
                    //��ʱ
    RF = AK8975_SDA_IN();                   //��Ӧ���ź�
    AK8975_SCL_RST();                    //����ʱ����
    asm("NOP");asm("NOP");
      
    AK8975_Data_O();                 //��ʱ
    return RF;
}
//**************************************
//��ak8975_soft_I2C���߷���һ���ֽ�����
//**************************************
#pragma optimize=none
void ak8975_soft_I2C_SendByte(uint8 dat)
{
    uint8 i;
    //DDRE=0b00001100;
    for (i=0; i<8; i++)         //8λ������
    {
        if(dat&0x80)
        AK8975_SDA_SET() ;
        else
        AK8975_SDA_RST();
        dat <<= 1;              //�Ƴ����ݵ����λ
                       //�����ݿ�
        AK8975_SCL_SET() ;                //����ʱ����
        ak8975_soft_delay();             //��ʱ
          
        AK8975_SCL_RST();                //����ʱ����
        ak8975_soft_delay();             //��ʱ
          
    }
    ak8975_soft_I2C_RecvACK();
}
//**************************************
//��ak8975_soft_I2C���߽���һ���ֽ�����
//**************************************
#pragma optimize=none
uint8 ak8975_soft_I2C_RecvByte()
{
    uint8 i;
    uint8 dat = 0,cy;
   // DDRE=0b00001100;
    AK8975_SDA_SET() ;  
    AK8975_Data_I();                  //ʹ���ڲ�����,׼����ȡ����,
    for (i=0; i<8; i++)         //8λ������
    {
        dat <<= 1;
        AK8975_SCL_SET() ;
        //DDRE=0b00000100;                 //����ʱ����
        ak8975_soft_delay();             //��ʱ
          
        if(AK8975_SDA_IN())
        cy=1;
        else
        cy=0;
        
        dat |= cy;             //������  
                
        AK8975_SCL_RST();                //����ʱ����
        ak8975_soft_delay();             //��ʱ
          
         
    }
    AK8975_Data_O();
    return dat;
}



/*!
 *  @brief      AK8975_SOFTд�Ĵ���
 *  @param      reg         �Ĵ���
 *  @param      dat         ��Ҫд������ݵļĴ�����ַ
 *  @since      v5.0
 *  Sample usage:       ak8975_soft_write_reg(AK8975_SOFT_XOFFL,0);   // д�Ĵ��� AK8975_SOFT_XOFFL Ϊ 0
 */
void ak8975_soft_write_reg(uint8 reg, uint8 Data)
{
    ak8975_soft_I2C_Start();                  //��ʼ�ź�
    ak8975_soft_I2C_SendByte(AK8975_SOFT_ADRESS);   //�����豸��ַ+д�ź�
    ak8975_soft_I2C_SendByte(reg);    //�ڲ��Ĵ�����ַ��
    ak8975_soft_I2C_SendByte(Data);       //�ڲ��Ĵ������ݣ�
    ak8975_soft_I2C_Stop();                   //����ֹͣ�ź�
}

/*!
 *  @brief      AK8975_SOFT���Ĵ���
 *  @param      reg         �Ĵ���
 *  @param      dat         ��Ҫ��ȡ���ݵļĴ�����ַ
 *  @since      v5.0
 *  Sample usage:       uint8 data = ak8975_soft_read_reg(AK8975_SOFT_XOFFL);    // ���Ĵ��� AK8975_SOFT_XOFFL
 */
uint8 ak8975_soft_read_reg(uint8 reg)
{
        uint8 REG_data;
	ak8975_soft_I2C_Start();                   //��ʼ�ź�
	ak8975_soft_I2C_SendByte(AK8975_SOFT_ADRESS);    //�����豸��ַ+д�ź�
	ak8975_soft_I2C_SendByte(reg);     //���ʹ洢��Ԫ��ַ����0��ʼ	
	ak8975_soft_I2C_Start();                   //��ʼ�ź�
	ak8975_soft_I2C_SendByte(AK8975_SOFT_ADRESS+1);  //�����豸��ַ+���ź�
	REG_data=ak8975_soft_I2C_RecvByte();       //�����Ĵ�������
	ak8975_soft_I2C_SendACK(1);                //����Ӧ���ź�
	ak8975_soft_I2C_Stop();                    //ֹͣ�ź�
	return REG_data;
}
int16 ak8975_soft_getdata(uint8 REG_Address)
{
	uint8 H,L;
	H=ak8975_soft_read_reg(REG_Address);
	L=ak8975_soft_read_reg(REG_Address-1);
	return (H<<8)+L;   //�ϳ�����
}
/*!
 *  @brief      AK8975_SOFT��ʼ�������� 2g ���̲���ģʽ
 *  @since      v5.0
 *  Sample usage:            ak8975_soft_init();    //��ʼ�� AK8975_SOFT
 */

void ak8975_soft_init(void)
{
        ak8975_soft_OPEN();        //��ʼ��ak8975_soft�ӿ�
	 
	ak8975_soft_write_reg(AK8975_SOFT_CNTL, 0x0f);    //Range 2g
	 
        DELAY_MS(500);
}