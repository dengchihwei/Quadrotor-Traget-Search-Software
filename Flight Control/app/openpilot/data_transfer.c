#include "data_transfer.h"
#include "common.h"
#include "utils/uartstdio.h"
#include "inc/hw_memmap.h"
#include "YRK_SONAR.h"
#include "include.h"
#include "Postion_Hold.h"
#include "PX4Flow.h"   //  PX4FLOW
#include "KS103.h"

uint16 nrf_flag;

uint16 nrf_irq_flag;
int flag_nrf=0;
int16 Nrf_in_switch[16];


void receive_date_check()
{
  if(flag_nrf==1)
  {
     eeprom_read(1);
           Nrf_Buf_Out.Data_int[0]=(int16)NRF_DATA_PACKET;
           Nrf_Buf_Out.Data_int[1]=17;
           Nrf_Buf_Out.Data_int[2]=(int16)(eeprom_readdate[0]);
           Nrf_Buf_Out.Data_int[3]=(int16)(eeprom_readdate[1]); 
           Nrf_Buf_Out.Data_int[4]=(int16)(eeprom_readdate[2]);
           Nrf_Buf_Out.Data_int[5]=(int16)(eeprom_readdate[3]);
           Nrf_Buf_Out.Data_int[6]=(int16)(eeprom_readdate[4]); 
           Nrf_Buf_Out.Data_int[7]=(int16)(eeprom_readdate[5]);
           Nrf_Buf_Out.Data_int[8]=(int16)(eeprom_readdate[6]);
           Nrf_Buf_Out.Data_int[9]=(int16)(eeprom_readdate[7]); 
           Nrf_Buf_Out.Data_int[10]=(int16)(eeprom_readdate[8]);

           
           nrf_tx(Nrf_Buf_Out.Data_char,NRF_DATA_PACKET);
            flag_nrf=0;
  }
  
  switch(Nrf_Buf_In.Data_int[1]){
    case 12:
             eeprom_write(2);break;
    case 13://遥控数据
              Nrf_in_switch[0]=Nrf_Buf_In.Data_int[0];//数据长度——26字节
              Nrf_in_switch[1]=Nrf_Buf_In.Data_int[1]; //包类型标志位
              Nrf_in_switch[2]=Nrf_Buf_In.Data_int[2]; //pitch
              Nrf_in_switch[3]=Nrf_Buf_In.Data_int[3];//roll
              Nrf_in_switch[4]=Nrf_Buf_In.Data_int[4];//throttle
              Nrf_in_switch[5]=Nrf_Buf_In.Data_int[5];//yaw
              Nrf_in_switch[6]=Nrf_Buf_In.Data_int[6];//开关信号
              Nrf_in_switch[7]=Nrf_Buf_In.Data_int[7];
              Nrf_in_switch[8]=Nrf_Buf_In.Data_int[8];
              Nrf_in_switch[9]=Nrf_Buf_In.Data_int[9];
              Nrf_in_switch[10]=Nrf_Buf_In.Data_int[10];//自动起降信号
              Nrf_in_switch[11]=Nrf_Buf_In.Data_int[11];//pid-p
              Nrf_in_switch[12]=Nrf_Buf_In.Data_int[12];//pid-d
              break;
    case 14:
             eeprom_write(1);
              Nrf_Buf_Out.Data_int[0]=(int16)NRF_DATA_PACKET; //第一字节存放发送的总字节数
              Nrf_Buf_Out.Data_int[1]=19;//包数据类型标志位
             nrf_tx(Nrf_Buf_Out.Data_char,NRF_DATA_PACKET); 
             break;
    case 15:
           eeprom_read(2);
           Nrf_Buf_Out.Data_int[0]=(int16)NRF_DATA_PACKET; //第一字节存放发送的总字节数
           Nrf_Buf_Out.Data_int[1]=18;//包数据类型标志位
           Nrf_Buf_Out.Data_int[2]=(int16)(eeprom_readdate[0]);
           Nrf_Buf_Out.Data_int[3]=(int16)(eeprom_readdate[1]); 
           Nrf_Buf_Out.Data_int[4]=(int16)(eeprom_readdate[2]);
           Nrf_Buf_Out.Data_int[5]=(int16)(eeprom_readdate[3]);
           Nrf_Buf_Out.Data_int[6]=(int16)(eeprom_readdate[4]); 
           Nrf_Buf_Out.Data_int[7]=(int16)(eeprom_readdate[5]);
           Nrf_Buf_Out.Data_int[8]=(int16)(eeprom_readdate[6]);
           Nrf_Buf_Out.Data_int[9]=(int16)(eeprom_readdate[7]); 
           Nrf_Buf_Out.Data_int[10]=(int16)(eeprom_readdate[8]);
           nrf_tx(Nrf_Buf_Out.Data_char,NRF_DATA_PACKET); 
           flag_nrf=1;
         break;   
}

}

  
uint8 nrf_getcmd()
{
  uint8 relen=0;
         if(nrf_irq_flag>0)
        {
          nrf_irq_flag=0;
          nrf_handler();
        }
        relen = nrf_rx(Nrf_Buf_In.Data_char,NRF_DATA_PACKET);               //等待接收一个数据包，数据存储在buff里
        if(relen != 0)
        {
         
          nrf_flag=400;
          //if(count0%2==0)

          //}
        }
        else if(nrf_flag>0)
        nrf_flag--; 
        return relen;
}






void nrf_sendstate()
{
	    Nrf_Buf_Out.Data_int[0]=(int16)NRF_DATA_PACKET; //第一字节存放发送的总字节数
            Nrf_Buf_Out.Data_int[1]=16;//包数据类型标志位
            Nrf_Buf_Out.Data_int[2]=(int16)(attitudeActual.Pitch*100);
            Nrf_Buf_Out.Data_int[3]=(int16)(attitudeActual.Roll*100);
            Nrf_Buf_Out.Data_int[4]=(int16)(attitudeActual.Yaw*100);
	    Nrf_Buf_Out.Data_int[5]=(int16)ks103_distance/10;
            Nrf_Buf_Out.Data_int[6]=(int16)(SumX_amend);                        //(int16)(attitudeActual.Pitch*100);
            Nrf_Buf_Out.Data_int[7]=(int16)(SumY_amend); 
            Nrf_Buf_Out.Data_int[8]=lowthrottle;     
            nrf_tx(Nrf_Buf_Out.Data_char,NRF_DATA_PACKET); 
}



uint8 UART_P_R=0;
uint8 UART_P_W=0;
char UART_FIFO[256];
static char buf;
static char sync=0;
static char data_cont=0;
static char chk=0;
static char check[4]="_CMD";
static uint16 sync_count=0;
static uint16 lose_sync=50;

uint16 uart_flag;




uint8 uart_getcmd()
{
  uint8 relen=0;
 for(;;)
    {
          
          if(UART_P_R!=UART_P_W)
          {
            buf=UART_FIFO[(UART_P_R)];
            UART_P_R++;
            //sprintf(txt,"%d %d %d ",w,chk,rea);
            //LCD_P6x8Str(35,6,txt);
            //readl++;
            if(sync==1)//同步成功
            {
                sync_count=lose_sync;
                UART_Buff_In.Data_char[data_cont]=buf;
                data_cont++;
              if(data_cont>UART_DATA_PACKET-1-4)
              {
                relen=data_cont;
                sync=0;
                data_cont=0;
                chk=0;
         //       uint8 k;
       //         for(k=0;k<SERVOSETTINGS_CH_NUMELEM;k++)
        //        servoData[k].cmd=UART_Buff_In.Data_int[k];
              }
              //else
              //{
                
              //}
            }
            else//sync==0
            {
              if(check[chk]==buf)
              {
                chk++;
                if(chk>3)
                {
                  sync=1;
                }
              }
              else
              {
                if(buf==check[0])
                {
                  chk=1;
                }
                else
                {
                  sync=0;
                  data_cont=0;
                  chk=0;
                }
              }
            }
          }
          else
            break;
        }
         if(relen != 0)
        {
         
          uart_flag=400;
          //if(count0%2==0)

          //}
        }
        else if(uart_flag>0)
        uart_flag--; 
 return relen;
}
/******************************************************************************/
void uart_sendstate()
{
 //  LOGO_LED=!!sync_count;
  if(sync_count==0)return;
  sync_count--;
        UART_Buff_Out.Data_char[0]='_';
        UART_Buff_Out.Data_char[1]='C';
        UART_Buff_Out.Data_char[2]='M';
        UART_Buff_Out.Data_char[3]='D';
        UART_Buff_Out.Data_int[2]=(int16)(attitudeActual.Roll*100);
        UART_Buff_Out.Data_int[3]=(int16)(attitudeActual.Pitch*100);
        UART_Buff_Out.Data_int[4]=(int16)(attitudeActual.Yaw*100);
      //  UART_Buff_Out.Data_int[3]=servoData[1].pos;
      //  UART_Buff_Out.Data_int[4]=servoData[2].pos;
      //  UART_Buff_Out.Data_int[5]=servoData[3].pos;
      //  UART_Buff_Out.Data_int[6]=servoData[4].pos;
      //  UART_Buff_Out.Data_int[7]=servoData[5].pos;
      //  UART_Buff_Out.Data_int[8]=servoData[6].pos;
      //  UART_Buff_Out.Data_int[9]=servoData[7].pos;
      //  UART_Buff_Out.Data_int[10]=servoData[8].pos;
      uint16 i=0;
      for(i=0;i<10;i++)
      UARTCharPut(UART5_BASE,UART_Buff_Out.Data_char[i]);
      //  uart_putbuff(UART0,UART_Buff_Out.Data_char,UART_DATA_PACKET);
}

