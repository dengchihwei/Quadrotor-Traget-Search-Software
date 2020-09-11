#include "control_command.h"
#include "common.h"
#include "action.h"
#include "data_transfer.h"
#include "IRQ_handler.h"
#include "Postion_Hold.h"
//#include "MKL_TPM.h"

uint16 arming=0;
float Yaw_con_before=0;
float Yaw_fixed=0;
bool fly=false;
bool exfly=false;

void command_handler()
{			  
          stabDesired.Mode=(Nrf_in_switch[6]/30.0);
            if(stabDesired.Mode<50&&stabDesired.Mode>25)   
            {
              stabDesired.Throttle=35;          
              stabDesired.Pitch=UART_Buff_In.Data_int[0]/1000.0+Nrf_Buf_In.Data_int[2]/30.0;
              stabDesired.Roll=UART_Buff_In.Data_int[1]/1000.0+Nrf_Buf_In.Data_int[3]/30.0;
              stabDesired.Yaw=UART_Buff_In.Data_int[3]/300.0+Nrf_Buf_In.Data_int[5]/30.0;
            }
            else                                          //定高模式
            {
              stabDesired.Throttle=(Nrf_in_switch[4])/20.0;
              stabDesired.Pitch=Nrf_in_switch[2]/30.0 + angle_x_out;
              stabDesired.Roll=Nrf_in_switch[3]/30.0 + angle_y_out;
              if(fabs(Nrf_in_switch[5]/30.0)>5)
              stabDesired.Yaw=Nrf_in_switch[5]/30.0;
              else
              stabDesired.Yaw=Yaw_fixed-Nrf_in_switch[9];//attitudeActual.Yaw;
            }
            
            if(fabs(Nrf_in_switch[5]/30.0)<=5 && fabs(Yaw_con_before)>5)
              Yaw_fixed=attitudeActual.Yaw;
            
            Yaw_con_before=Nrf_in_switch[5]/30.0;
            
            if(stabDesired.Mode>25)
            {
             LED1_ON();
              switch(flightStatus.Armed)
              {
              case FLIGHTSTATUS_ARMED_DISARMED:
                flightStatus.Armed=FLIGHTSTATUS_ARMED_ARMING;
                arming=0;
                break;
              case FLIGHTSTATUS_ARMED_ARMING:
                arming++;
                if(arming>50)
                 flightStatus.Armed=FLIGHTSTATUS_ARMED_ARMED; 
                break;
              case FLIGHTSTATUS_ARMED_ARMED:
                if(stabDesired.Mode<50)
                {stabDesired.StabilizationMode[0]=STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
                  stabDesired.StabilizationMode[1]=STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
                  stabDesired.StabilizationMode[2]=STABILIZATIONDESIRED_STABILIZATIONMODE_RATE;
                  stabDesired.StabilizationMode[3]=STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDEHOLD;}
                else if(stabDesired.Mode<125)                                 //这是我们要用的模式
                {stabDesired.StabilizationMode[0]=STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
                  stabDesired.StabilizationMode[1]=STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
                  if(fabs(Nrf_in_switch[3]/30.0)>5)
                  stabDesired.StabilizationMode[2]=STABILIZATIONDESIRED_STABILIZATIONMODE_RATE;
                  else
                  stabDesired.StabilizationMode[2]=STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
                  stabDesired.StabilizationMode[3]=STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDEHOLD;}
                else
                {stabDesired.StabilizationMode[0]=STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
                  stabDesired.StabilizationMode[1]=STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
                  stabDesired.StabilizationMode[2]=STABILIZATIONDESIRED_STABILIZATIONMODE_RATE;
                  stabDesired.StabilizationMode[3]=STABILIZATIONDESIRED_STABILIZATIONMODE_RATE;}
                arming=51;
                break;
              }
              
            }
            else
            {
              LED1_OFF();
               switch(flightStatus.Armed)
              {
              case FLIGHTSTATUS_ARMED_DISARMED:
                arming=0;
                break;
              case FLIGHTSTATUS_ARMED_ARMING:
                arming=0;
                flightStatus.Armed=FLIGHTSTATUS_ARMED_DISARMED; 
                break;
              case FLIGHTSTATUS_ARMED_ARMED:
                arming--;
                if(arming<10)
                 flightStatus.Armed=FLIGHTSTATUS_ARMED_DISARMED;
                break;
              }
            }
}