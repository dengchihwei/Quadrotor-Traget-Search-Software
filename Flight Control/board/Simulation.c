#include <stdbool.h>
#include <stdint.h>
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"
#include "Simulation.h"
#include "math.h"
#include "common.h"
#include "IRQ_handler.h"
#include "data_common.h"
#include "YRK_SONAR.h"

#define pos_x_i_max 5000
#define pos_mov_i_max 800
#define angel_out_max 10.0f

float last_roll = 0;
float last_pitch = 0;
float change_x, change_y;
float radians_to_pixels_x = 2.5435f,radians_to_pixels_y = 2.5435f;
float conv_factor = 0.008f; 
float High_Now;


int8_t  x=0;
int8_t  y=0;
//float  SumX;
//float SumY;
//float  SumX1;
//float SumY1;
 
typedef enum {
	Mode_One,					 //CPOL=0,CPHA=0
	Mode_Two,					//CPOL=0,CPHA=1
	Mode_Three,				        //CPOL=1,CPHA=0
	Mode_Four				        //CPOL=1,CPHA=1
}SPI_Mode;	

SPI_Mode SPI_Ope_Mode;					//SPI Mode

void Sim_SPI_GPIO_Init(uint8 CPOL,uint8 CPHA,uint32 Speed)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);  
    SysCtlDelay(3);
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_6|GPIO_PIN_7);
     GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_6|GPIO_PIN_7, GPIO_STRENGTH_4MA,GPIO_PIN_TYPE_STD_WPU);//GPIO��������Ϊ������ģʽ
    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);  
    SysCtlDelay(3);
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_5);
    
    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE); 
    SysCtlDelay(3);
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_1);
     GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_1, GPIO_STRENGTH_4MA,GPIO_PIN_TYPE_STD_WPU);//GPIO��������Ϊ������ģʽ
    
    if(CPOL == 0)
	{
		if(CPHA == 0)
			SPI_Ope_Mode = Mode_One;
		else
			SPI_Ope_Mode = Mode_Two;
	}
	else
	{
		if(CPHA == 0)
			SPI_Ope_Mode = Mode_Three;
		else
			SPI_Ope_Mode = Mode_Four;
	}

}

void ADNS3080_Init(void)
{
  uint8 temp_spi;
  CLR_SPI_NCS;
 // SysCtlDelay(SysCtlClockGet()/150);                    //ʵ���ϲ��Ϊ20ms
  delay(67000);
  // set frame rate to manual
  temp_spi = SPI_Read(ADNS3080_EXTENDED_CONFIG);
  temp_spi = (temp_spi & ~0x01) | 0x01;
  delay(67000);                                           //ʵ���ϲ��Ϊ20ms
  SPI_Write(ADNS3080_EXTENDED_CONFIG,temp_spi);
  delay(67000);                                     //ʵ���ϲ��Ϊ20ms
  temp_spi=SPI_Read(ADNS3080_EXTENDED_CONFIG);
  // set frame period to 12000 (0x2EE0)	
  SPI_Write(ADNS3080_FRAME_PERIOD_MAX_BOUND_LOWER,0xE0);
  delay(67000);                                           //ʵ���ϲ��Ϊ20ms
  SPI_Write(ADNS3080_FRAME_PERIOD_MAX_BOUND_UPPER,0x2E);
  delay(67000);                                         //ʵ���ϲ��Ϊ20ms
  // set 1600 resolution bit
  temp_spi = SPI_Read(ADNS3080_CONFIGURATION_BITS);
  temp_spi |= 0x10;
 delay(67000);                                          //ʵ���ϲ��Ϊ20ms
  SPI_Write(ADNS3080_CONFIGURATION_BITS,temp_spi);	
  delay(67000);                                     //ʵ���ϲ��Ϊ20ms
  SET_SPI_NCS;
  
}

uint8 SPI_Basic_RW(uint8 data_write)
{
  uint8 count_spi;
  uint8 data_read;
  switch(SPI_Ope_Mode)
  {
  case Mode_One:											//???D��????a�̨���???
    if(data_write & 0x80)							//CSN��?��-�̨���????��?a��?D��3?��y?Y��?��?o��?���̨���???��?��y??��?��??a��?2��?����y?Y��??��
      SET_SPI_MOSI;
    else
      CLR_SPI_MOSI;
    for(count_spi = 0;count_spi < 8;count_spi ++)
    {
      delay(28);
      SET_SPI_SCL;										//��?��y????o��?����??����?��y?Y
      if(READ_MISO_DATA)						                 
        data_read |= 0x01;
      if(count_spi<7)											//??D����a������?7��?
      {
        data_write = data_write << 1; 
        data_read = data_read << 1; 
      }
      delay(28);
      
      CLR_SPI_SCL;										//???��????o��?����?D��3?��y?Y
      
      if(data_write & 0x80)						//D��3?��?�̨�????��??TD���?��?�䨮������?��2??��D��?��y??����?����?��?CSN����?����?��-??��?��????����??��D��2�����¨�??-����3��
       SET_SPI_MOSI;
      else
        CLR_SPI_MOSI;
    }
    break;
  case Mode_Two:											//???D��????a�̨���???
    for(count_spi = 0;count_spi < 8;count_spi ++)
    {
      SET_SPI_SCL;										//��?��y????o����?3?��y?Y
      
      if(data_write & 0x80)
        SET_SPI_MOSI;
      else
        CLR_SPI_MOSI;				
      delay(28);
      CLR_SPI_SCL;										//???��????o��?����?��y?Y				
      if(READ_MISO_DATA)						                
        data_read |= 0x01;
      if(count_spi<7)											//??D����a������?7��?
      {
        data_write = data_write << 1; 
        data_read = data_read << 1;
	
      }
      delay(28);
    }
    break;	
  case Mode_Three:										//???D��????a??��???
    if(data_write & 0x80)
      SET_SPI_MOSI;
    else
      CLR_SPI_MOSI;	
    for(count_spi = 0;count_spi < 8;count_spi ++)
    {
      delay(28);
      CLR_SPI_SCL;										//???��???����y?Y	
				
      if(READ_MISO_DATA)						                 
        data_read |= 0x01;				
      if(count_spi<7)											//??D����a������?7��?
      {
        data_write = data_write << 1; 
        data_read = data_read << 1; 
				
      }
      delay(28);
      SET_SPI_SCL;										//��?��y??D�䨺y?Y				
      if(data_write & 0x80)						//bit9D�䨨?��?��y?Y?TD���?��2??��D???��??���䨨????����?��?CSN��?��-??��?��????����??��D��2�����¨�??-����3��
        SET_SPI_MOSI;
      else
        CLR_SPI_MOSI;								
    }
    break;
  case Mode_Four:											//???D��????a??��???
    for(count_spi = 0;count_spi < 8;count_spi ++)
    {
      CLR_SPI_SCL;	
      if(data_write & 0x80)						//???��??D�䨺y?Y
        SET_SPI_MOSI;
      else
        CLR_SPI_MOSI;
      delay(28);
      SET_SPI_SCL;										//��?��y???����?��y?Y									
      if(READ_MISO_DATA)						               
        data_read |= 0x01;
      if(count_spi<7)											//??D����a������?7��?
      {
        data_write = data_write << 1; 
        data_read = data_read << 1;
      }
		
      delay(28);

    }

    break;
  default:
    break;
  }
  return(data_read); 
}

uint8 SPI_Read(uint8 reg)
{
	uint8 reg_val;
	uint16 count_spi;
	CLR_SPI_NCS;                
	SPI_Basic_RW(ADNS3080_READ_REG + reg);       
	// SysCtlDelay(SysCtlClockGet()/1000);                  //ʵ���ϲ��Ϊ3ms
        delay(9900);
	reg_val = SPI_Basic_RW(0x00);   
	SET_SPI_NCS;               
	return(reg_val);       
}

void SPI_Write(uint8 reg,uint8 value)
{
	uint16 count_spi;
	CLR_SPI_NCS; 
	SPI_Basic_RW(ADNS3080_WRITE_REG + reg);
	// SysCtlDelay(SysCtlClockGet()/1000);              //ʵ���ϲ��Ϊ3ms
        delay(9900);
	SPI_Basic_RW(value);
	SET_SPI_NCS;
}

void motion_read(uint8 * data_buffer)
{
       int32 i=8000;
	uint8 count_spi;
	CLR_SPI_NCS;                
	SPI_Basic_RW(ADNS3080_READ_REG + ADNS3080_MOTION_BURST);
	// SysCtlDelay(SysCtlClockGet()/30000);                //ʵ���ϲ��sΪ100u
        delay(320);
	for(count_spi = 0;count_spi < 7;count_spi++)
	{
		*(data_buffer ++) = SPI_Basic_RW(0x00);
	}
	
	SET_SPI_NCS;
        
}

void data_fix(void)
{
  float diff_roll     = attitudeActual.Roll - last_roll;	//��a???a???����??��������a?��������?diff_roll?a?o?�̡�??��������a?������?a?y?��
  float diff_pitch    = attitudeActual.Pitch - last_pitch;
  last_roll   = attitudeActual.Roll;
  last_pitch  = attitudeActual.Pitch;
  float x_mm,y_mm;

  //float sum_x,sum_y;
   x=0;
  y=0;
  float High_Now;
  unsigned char move=0;
   move=ADNS3080_Data_Buffer[0];
   x=ADNS3080_Data_Buffer[1];
   y=ADNS3080_Data_Buffer[2];
   	if(x&0x80)
	  {
	  //x��??t21??��a??	
	  x -= 1;
	  x = ~x;	
	  x=(-1)*x;
	  x-=256;
 	  }
	if(y&0x80)
	  {
	  //y��??t21??��a??	
	  y -= 1;
	  y = ~y;	
	  y=(-1)*y;
	  y-=256;
	  } 
   
   if(move&0x10!=1)
     if(move&0x80)
     {
       // printf("%d,%d\n",sum_x,sum_y);
     }
     else
     {
       x=0;
       y=0;
     }
     x=x^y;
     y=x^y;
     x=x^y;
     
    x_mm =(float)x + (diff_pitch * radians_to_pixels_x);   //diff_pitch x_mm
    y_mm = (float)y + (diff_roll * radians_to_pixels_y); 
      High_Now= sonar_distance[0]*1000;
    // x=(25.4*(float)x *High_Now)/(12*1600);//?����?=d_x*(25.4/1600)*n   ???Dn=????:????=8o��?��:??3��
    //y=(25.4*(float)y *High_Now)/(12*1600);   
      x_mm=x_mm*High_Now * conv_factor;
      y_mm=y_mm*High_Now * conv_factor;
      if(sonar_distance[0]*100>=20)
      {
    SumX=SumX+x_mm;             //��??��X?����?��?��??����y?Y
     SumY=SumY+y_mm;			 //��??��Y?����?��?��??����y?Y
      }
      else if (sonar_distance[0]*100<=15)
      {
        SumX=0;
        SumY=0;
      }
    x=0;
    y=0;
}

void delay(int32 count_delay)
{
  int32 i=count_delay;   // 28  10us
               //320  100us     9900  3ms  67000  20ms
   //SysCtlDelay(SysCtlClockGet()/600000);
  //  SysCtlDelay(SysCtlClockGet()/6000000);
   while(i>=0)
          i--;
}



