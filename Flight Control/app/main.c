#include "common.h"
#include "include.h"
#include "IRQ_handler.h"
//#include "YRK_DATA_COMMON.h"
#include "main.h"
   
#include "data_common.h"
#include "param_common.h"
#include "eeprom.h"
#include "attitudesolving.h"
#include "stabilization.h"
#include "motor_control.h"
#include "sensorfetch.h"
#include "PX4Flow.h"
#include "KS103.h"



#define SHOW_RPY



int main()
{
    IntMasterDisable();
    lowlevel_init();
    DELAY_MS(100);
    led_init();
    PX4Flow_uart_init(115200,UART6_IRQHandler);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    EEPROMInit();
 	
    KS103_init();
    //sonar_init();
    mpu6050_soft_init();

    tim1_init(PIT_IRQHandler);
    
    IntPriorityGroupingSet(3);

    ROM_IntPrioritySet(INT_UART6, 0x00);//ͼ�񴫸����ж�
    ROM_IntPrioritySet(INT_TIMER1A, 0x02<<6);
    ROM_IntPrioritySet(INT_GPIOC, 0x01<<6);
	
  
    data_common_init();//���ݳ�����ʼ��
    param_common_init();//����������ʼ��
    motorcontrol_init();

    //motor_reset();//���ڵ����ʼ��                   
    DELAY_MS(5000);
    AttitudeInitialize();
    StabilizationInitialize();
    	
    while(!nrf_init(PORTC_IRQHandler)); 

    IntMasterEnable();

     UARTprintf("SYSTEM_BOOT\n");

    while(1);
 // return 0;
}
