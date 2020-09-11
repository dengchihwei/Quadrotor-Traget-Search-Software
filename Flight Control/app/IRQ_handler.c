#include "common.h"
#include "include.h"
#include "IRQ_handler.h"

#include "attitudesolving.h"
#include "stabilization.h"
#include "sensorfetch.h"
#include "actuator.h"
#include "motor_control.h"
#include "control_command.h"
#include "mixer.h"
#include "data_transfer.h"
#include "data_common.h"
#include "param_common.h"
#include "Postion_Hold.h"
#include "inc/hw_memmap.h"
#include "PX4Flow.h"
#include "KS103.h"


/*!
 *  @brief      PIT0�жϷ�����
 *  @since      v5.0
 */

void receive_date_check(void);

uint16 count0=0;
uint16 count1=0;
uint8 ADNS3080_Data_Buffer[7];
uint16 IMU_ext_flag=0;
uint16 ddd;
uint32 autostart_count=0;


//time1,2.5ms
void PIT_IRQHandler(void)
{
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

      //LED0_OFF();

 
    attsolving();   //��̬����                           

 
          
    count0++;
      
    
    stabilize();    //PID����
    
   
    hold();           //����
	
   
    if(count0%10==3)
    {
	px4_data_fix();		
    }
	
	
    mixing(flightStatus.Armed==FLIGHTSTATUS_ARMED_ARMED);             //���
      
    

    
    if(count0%10==3)
    {
        Control();                         //��λ
    }

    if(count0%50==2)
    {
        KS103_get_distance();                         //���
    }
    
    if(count0%50==49)
    {
	ks103_handler();
    }

       /*******************nrf************************/

    if(nrf_getcmd())
    {
      receive_date_check();
      nrf_sendstate();                      //��nrf��������,���ǿ����ڷ����ϼ��ϸ߶�,Data_int[8]���鳤����8
    }
       /*******************nrf************************/
    
	
     command_handler();


         /*************ERROR*******************/
      if(fabs(attitudeActual.Pitch)>40 || fabs(attitudeActual.Roll)>40 )
         flightStatus.Armed=FLIGHTSTATUS_ARMED_DISARMED; 
        /***********************lost signal***********/
     if(nrf_flag<=1)
         flightStatus.Armed=FLIGHTSTATUS_ARMED_DISARMED; 
     if(nrf_flag<=395) 
         LED0_OFF();
     else 
         LED0_ON();
         
     mixing(flightStatus.Armed==FLIGHTSTATUS_ARMED_ARMED);
   
     timer_tictok(&timer,CTL_RATE);    //��timer�����������2.5ms
        // LED0_ON();
}

//NRF�ж�
void PORTC_IRQHandler()
{
    uint32 ulStatus;
    ulStatus = GPIOIntStatus(GPIO_PORTC_BASE, true); // ��ȡ�ж�״̬
    GPIOIntClear(GPIO_PORTC_BASE, ulStatus);
    

    if(ulStatus&GPIO_PIN_5)
    {

      nrf_irq_flag=1;
    }
}



void UART6_IRQHandler()
{
    unsigned char Uart6Date; 
    uint32_t ui32Status;
    ui32Status = ROM_UARTIntStatus(UART6_BASE, true);
    ROM_UARTIntClear(UART6_BASE, ui32Status);

    while(ROM_UARTCharsAvail(UART6_BASE))
    {  
      Uart6Date = ROM_UARTCharGet(UART6_BASE);
      FLOW_MAVLINK(Uart6Date);
    }
 }
