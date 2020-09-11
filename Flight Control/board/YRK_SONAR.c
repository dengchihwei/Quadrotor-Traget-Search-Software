
#include "YRK_SONAR.h"

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
#include "data_common.h"


//#include "utils/uartstdio.c"
#include <string.h>


#define    SONAR_TR                  GPIO_PIN_1
#define    SONAR_EC                  GPIO_PIN_2
#define    SONAR_TR_BASE             GPIO_PORTD_BASE
#define    SONAR_EC_BASE             GPIO_PORTD_BASE
#define    SONAR_IRQ_INT_VECTOR              INT_GPIOD
#define    SONAR_IRQ_INT_PIN                 GPIO_INT_PIN_2

extern float cos_lookup_deg(float angle);
extern float sin_lookup_deg(float angle);
#define    SONAR_TR_SET()            HWREG(SONAR_TR_BASE + (GPIO_O_DATA + (SONAR_TR << 2))) = SONAR_TR			//IIC�������Ŷ���  
#define    SONAR_TR_RST()            HWREG(SONAR_TR_BASE + (GPIO_O_DATA + (SONAR_TR << 2))) = 0x00			//IIC�������Ŷ���  
static uint64 sonar_start,sonar_end;
float sonar_distance[3],sonar_speed,sonar_acc;
uint32 ii;
const double temp = 1.0/80.0;
float pulse=0;
float a,b;
//float filter[2];
void sonar_handler()
{
   GPIOIntClear(GPIO_PORTD_BASE, GPIO_PIN_2);
  if ( GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_2) == GPIO_PIN_2){                    //���˸ߵ�ƽ����ʼ����
	HWREG(TIMER2_BASE + TIMER_O_TAV) = 0; 
    TimerEnable(TIMER2_BASE,TIMER_A);
    //echowait=1;
  }
  else{                                                                            //ֹͣ����
    pulse = TimerValueGet(TIMER2_BASE,TIMER_A); 
    TimerDisable(TIMER2_BASE,TIMER_A);
    //echowait=0;
    pulse =(uint32_t)(temp * pulse);
    pulse = pulse / 5800;
      
    sonar_distance[2]=sonar_distance[1];
    sonar_distance[1]=sonar_distance[0];
    sonar_distance[0]=pulse;
    if(attitudeActual.Pitch<0)
      a=attitudeActual.Pitch+360;
    else
      a=attitudeActual.Pitch;
    if(attitudeActual.Roll<0)
      b=attitudeActual.Roll+360;
    else
      b=attitudeActual.Roll;
    sonar_distance[0]=sonar_distance[0]*0.7+sonar_distance[1]*0.3;
    sonar_distance[0]=sonar_distance[0]*cos_lookup_deg(a)+0.016f*sin_lookup_deg(a);
    sonar_distance[0]=sonar_distance[0]*cos_lookup_deg(b)-0.046f*sin_lookup_deg(b);
      
      
      //pulse*(cos_lookup_deg(fabs(attitudeActual.Pitch))-a*sin_lookup_deg(fabs(attitudeActual.Pitch)))*cos_lookup_deg(fabs(attitudeActual.Roll));
      
      
       sonar_speed=sonar_distance[0]-sonar_distance[1];
       sonar_acc=sonar_distance[0]-sonar_distance[1]-sonar_distance[1]+sonar_distance[2];
    }
}
void tim2_init()
{
    
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
  SysCtlDelay(3);
  TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC_UP);
  TimerEnable(TIMER2_BASE,TIMER_A);
}
void sonar_init()
{
     SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);                              //���÷�������    
     SysCtlDelay(3);
     GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_1);

      SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);                              //���û�������  
      SysCtlDelay(3);
      GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_2);
      GPIOIntEnable(GPIO_PORTD_BASE, GPIO_PIN_2);
      GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_2,GPIO_BOTH_EDGES);              //����˫��Ե�ж�
      GPIOIntRegister(GPIO_PORTD_BASE,sonar_handler);
        
      tim2_init();
}
void sonar_triger()
{
      GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, GPIO_PIN_1);                //�������
      SysCtlDelay(366);    
      GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, ~GPIO_PIN_1);


}