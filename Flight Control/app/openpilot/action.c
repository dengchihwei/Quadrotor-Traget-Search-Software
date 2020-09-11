#include "action.h"
#include "tracking.h"

int sign_counter=0;
int sign_flag=1;
int sign_flag_before=1;
int height_control=0;
int height_control_apply=0;
int forward_control=0;
int time_status;

void findsign()
{
  if(ccd1_width>35)
    sign_flag=1;
  else
    sign_flag=0;
  if(sign_flag>sign_flag_before)
    sign_counter++;
  
  sign_flag_before=sign_flag;
}

void updown()
{
  if(sign_counter<=0)
    height_control=0;
  else 
  {
    switch(sign_counter){
  case 1:{height_control=9481; break;}
  case 2:{height_control=11000; break;}
  case 3:{height_control=9481; break;}
  case 4:{height_control=9481; break;}
  case 5:{height_control=8333; break;}
  case 6:{height_control=7222; break;}
  case 7:{height_control=4500; break;}  
    }
  }
}

void fade()
{
  if(flightStatus.Armed==FLIGHTSTATUS_ARMED_ARMED)
  {if(height_control>height_control_apply)
    height_control_apply+=20;
  else if(height_control<height_control_apply)
    height_control_apply-=20;
  }
}

void fowardInitialize()
{
  timer.stamp1=timestampget(&timer);  
}

void fowardControl()
{
  time_status=(timestampget(&timer)-timer.stamp1)%500;
  if(time_status>0 && time_status<250)
    switch(sign_counter){
      case 1:{forward_control=220; break;}
      case 2:{forward_control=220; break;}
    case 3:{if(attitudeActual.Yaw<90)
        forward_control=-340;
    else{
         forward_control=110;
         break;}}
      case 4:{forward_control=220;YAW_=2.47;break;}
      case 5:{forward_control=220; break;}
      case 6:{forward_control=220; break;}
      case 7:{forward_control=220; break;}  
    }
  else if(time_status>250 && time_status<500)
    switch(sign_counter){
      case 1:{forward_control=-270; break;}
      case 2:{forward_control=-360; break;}
    case 3:{if(attitudeActual.Yaw<90)
        forward_control=-400;
    else{
         forward_control=-450;
         YAW_=2.47;
         break;}}
      case 4:{forward_control=-360;YAW_=2.47;break;}
      case 5:{forward_control=-270; break;}
      case 6:{forward_control=-250; break;}
      case 7:{forward_control=-230; break;}  
    }
}
