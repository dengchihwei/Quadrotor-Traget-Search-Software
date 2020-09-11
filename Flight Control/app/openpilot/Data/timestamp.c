#include "timestamp.h"
uint32 timestampget(TimerData* timer)
{
  return ((timer->h*60+timer->m)*60+timer->s)*1000+timer->ms;
}

void timer_tictok(TimerData* timer,uint16 stepus)
{
  timer->ms+=stepus/1000;
  if(timer->ms>=1000)
  {
    timer->ms-=1000;
    timer->s+=1;
    if(timer->s>=60)
    {
      timer->s-=60;
      timer->m+=1;
      if(timer->m>=60)
      {
       timer->m-=60;
       timer->h+=1;
      }
    }
  }
}
