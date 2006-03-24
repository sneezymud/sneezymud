/* ************************************************************************
*  file: weather.c , Weather and time module              Part of DIKUMUD *
*  Usage: Performing the clock and the weather                            *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"

/* uses */

extern struct time_info_data time_info;
extern struct weather_data weather_info;

/*In this part. */

void weather_and_time(int mode);
void another_hour(int mode);
void weather_change(void);
void GetMonth( int month);
void ChangeWeather( int change);

/* what stage is moon in?  (1 - 32) */
unsigned char moontype;   

/* Here comes the code */

void weather_and_time(int mode)
{
	another_hour(mode);
	if(mode)
		weather_change();
}



void another_hour(int mode)
{
  char moon[20], buf[100];

	time_info.hours++;

	if (mode) {
		switch (time_info.hours) {
	         	case 3 :
                          {
			    send_to_outdoor("The moon sets.\n\r");
                            break;
			  }
			case 5 :
			{
				weather_info.sunlight = SUN_RISE;
				send_to_outdoor("The sun rises in the east.\n\r");
				break;
			}
			case 7 :
			{
				weather_info.sunlight = SUN_LIGHT;
				send_to_outdoor("The day has begun.\n\r");
				break;	
			}
  		        case 12 :
			  {
                                send_to_outdoor("It is noon.\n\r");
				break;
			  }
			case 18 :
			{
				weather_info.sunlight = SUN_SET;
				send_to_outdoor(
				"The sun slowly disappears in the west.\n\r");
				break;
			}
			case 20 :
			{
				weather_info.sunlight = SUN_DARK;
				send_to_outdoor("The night has begun.\n\r");
				break;
			}
		        case 22 :
			  {
                            if (moontype < 4) {
                               strcpy(moon, "new");
                             } else if (moontype < 12) {
                               strcpy(moon, "waxing");
                             } else if (moontype < 20) {
                               strcpy(moon, "full");
                             } else  if (moontype < 28) {
                               strcpy(moon, "waning");
                             } else {
                               strcpy(moon, "new");
                             }
                             sprintf(buf,"The %s moon rises in the east\n\r",moon);
                             send_to_outdoor(buf);
                             break;
			  }
			default : break;
		}
	}

	if (time_info.hours > 23)  /* Changed by HHS due to bug ???*/
	{
		time_info.hours -= 24;
		time_info.day++;
                moontype++;
                if (moontype > 32)
                   moontype = 1;

		if (time_info.day>34)		{
			time_info.day = 0;
			time_info.month++;
                        GetMonth(time_info.month);

			if(time_info.month>16)	       	{
				time_info.month = 0;
				time_info.year++;
			}
		}
	}
}

void weather_change(void)
{
	int diff, change;

	if((time_info.month>=9)&&(time_info.month<=16))
		diff=(weather_info.pressure>985 ? -2 : 2);
	else
		diff=(weather_info.pressure>1015? -2 : 2);

	weather_info.change += (dice(1,4)*diff+dice(2,6)-dice(2,6));

	weather_info.change = MIN(weather_info.change,12);
	weather_info.change = MAX(weather_info.change,-12);

	weather_info.pressure += weather_info.change;

	weather_info.pressure = MIN(weather_info.pressure,1040);
	weather_info.pressure = MAX(weather_info.pressure,960);

	change = 0;

	switch(weather_info.sky){
		case SKY_CLOUDLESS :
		{
			if (weather_info.pressure<990)
				change = 1;
			else if (weather_info.pressure<1010)
				if(dice(1,4)==1)
					change = 1;
			break;
		}
		case SKY_CLOUDY :
		{
			if (weather_info.pressure<970)
				change = 2;
			else if (weather_info.pressure<990)
				if(dice(1,4)==1)
					change = 2;
				else
					change = 0;
			else if (weather_info.pressure>1030)
				if(dice(1,4)==1)
					change = 3;

			break;
		}
		case SKY_RAINING :
		{
			if (weather_info.pressure<970)
				if(dice(1,4)==1)
					change = 4;
				else
					change = 0;
			else if (weather_info.pressure>1030)
					change = 5;
			else if (weather_info.pressure>1010)
				if(dice(1,4)==1)
					change = 5;

			break;
		}
		case SKY_LIGHTNING :
		{
			if (weather_info.pressure>1010)
					change = 6;
			else if (weather_info.pressure>990)
				if(dice(1,4)==1)
					change = 6;

			break;
		}
		default : 
		{
			change = 0;
			weather_info.sky=SKY_CLOUDLESS;
			break;
		}
	}

        ChangeWeather(change);

}

void ChangeWeather( int change)
{

   if (change < 0)
      change = 0;
   if (change > 7)
      change = 6;

	switch(change){
		case 0 : break;
		case 1 :
		{
			send_to_outdoor("The sky is getting cloudy.\n\r");
			weather_info.sky=SKY_CLOUDY;
			break;
		}
		case 2 :
		{
                        if ((time_info.month > 3) && (time_info.month < 14)) 
			   send_to_outdoor("It starts to rain.\n\r");
                        else 
                           send_to_outdoor("It starts to snow. \n\r");
			weather_info.sky=SKY_RAINING;
			break;
		}
		case 3 :
		{
			send_to_outdoor("The clouds disappear.\n\r");
			weather_info.sky=SKY_CLOUDLESS;
			break;
		}
		case 4 :
		{
                        if ((time_info.month > 3) && (time_info.month < 14)) 
			   send_to_outdoor("You are caught in lightning storm.\n\r");
                        else 
                           send_to_outdoor("You are caught in a blizzard. \n\r");
			weather_info.sky=SKY_LIGHTNING;
			break;
		}
		case 5 :
		{
                        if ((time_info.month > 3) && (time_info.month < 14)) 
			   send_to_outdoor("The rain has stopped.\n\r");
                        else 
                           send_to_outdoor("The snow has stopped. \n\r");
			weather_info.sky=SKY_CLOUDY;
			break;
		}
		case 6 :
		{
                        if ((time_info.month > 3) && (time_info.month < 14)) 
			   send_to_outdoor("The lightning has gone, but it is still raining.\n\r");
                        else 
                           send_to_outdoor("The blizzard is over, but it is still snowing.\n\r");
			weather_info.sky=SKY_RAINING;
			break;
		}
		default : break;
	}
}

void GetMonth( int month)
{
   if (month < 0)
      return;

   if (month <= 1)
       send_to_outdoor(" It is bitterly cold outside\n\r");
   else if (month <=2)
       send_to_outdoor(" It is very cold \n\r");
   else if (month <=4)
       send_to_outdoor(" It is chilly outside \n\r");
   else if (month <= 5)
        send_to_outdoor(" The flowers start to bloom \n\r");
   else if (month <= 11)
        send_to_outdoor(" It is warm and humid. \n\r");
   else if (month <= 12)
        send_to_outdoor(" It starts to get a little windy \n\r");
   else if (month <= 13)
        send_to_outdoor(" The air is getting chilly \n\r"); 
   else if (month <= 14)
        send_to_outdoor(" The leaves start to change colors. \n\r");
   else if (month <= 15)
        send_to_outdoor(" It starts to get cold \n\r");
   else if (month <= 16)
        send_to_outdoor(" It is bitterly cold outside \n\r");

}
