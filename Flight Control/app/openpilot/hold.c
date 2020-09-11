#include "hold.h"
#include "include.h"
#include "KS103.h"
	 

bool lowthrottle=false;
float lasts=0;//上次面积

void hold()
{
	  float temp;


	eeprom_read(1);

	//temp=sonar_distance[0]*eeprom_readdate[0]/1000+sonar_speed*eeprom_readdate[2]/100;//+sonar_acc*eeprom_readdate[2]/100;
	temp=ks103_distance/1000.0*eeprom_readdate[0]/1000+ks103_delta_distance/1000.0*eeprom_readdate[2]/100;

	  
	 actuatorDesired.Throttle=stabDesired.Throttle-temp;
		
	   
	  if(actuatorDesired.Throttle>70)
	  {
		actuatorDesired.Throttle=70;
		lowthrottle=true;
	  }
	  else
	  {
		lowthrottle=false;
	  }
	  if(actuatorDesired.Throttle<0)
		actuatorDesired.Throttle=0; 

}