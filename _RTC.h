
#include <RTClib.h>
#include "time.h"

const char* ntpServer = "10.7.74.179";//"pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 19800;

RTC_DS3231 rtc;

typedef struct RTC_TIME{
  int Hr=0;
  int Min=0;
  int Sec=0;
  int Dt=0; 
  int Mnth=0;
  int yr=0;  
} TIME;

TIME Time;

void RTC_init(void);
void update_rtc(void);
void NTP_init(void);
String NTP_time(void);

void RTC_init(void){
  rtc.begin();
  DateTime now = rtc.now();
  Serial_monitor.println(String(now.day()) + "/" + String(now.month()) + "/" + String(now.year()) + ',' + String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second()));
//  rtc.adjust(DateTime(2021, 9, 06, 14, 45, 30));
}

void NTP_init(void){
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    Serial_monitor.println(NTP_time());
}

void update_rtc(void){
  DateTime now = rtc.now();
  Time.Dt   = now.day(); 
  Time.Mnth = now.month();
  Time.yr   =now.year();
  Time.Hr   =now.hour();
  Time.Min  =now.minute();
  Time.Sec  =now.second();
}

void check_response_RTC(String str){
//  Serial_monitor.println(str);
    DateTime now = rtc.now();
    String Buff = "";
    int str_len =str.length();
    for(int i=1;i<str_len-1;i++)
    {          
      char ch = str[i];
      if(ch!=0x5C)
        Buff.concat(ch);
    }
    str = "";        str.concat(Buff);        Buff="";
//"reading_time": "30 Jun, 2022 17:46:47"

    Serial_monitor.println("\r\n");
    if(str.indexOf("\"readindTime\":\"")>-1)
    {
        String settime="";
        int ptr = str.indexOf("\"readindTime\":\"")+15;
        for(int i=ptr;str[i]!='.';i++)
        {
          char ch = str[i];//              Serial_monitor.print(ch);
          settime.concat(ch);
        }
       String set_hr="",set_mn="",set_sec="",set_date="",set_month="",set_year="";
       int i=ptr;
       for(;str[i];i++)          {
          char ch = str[i];
          if(ch=='-')
            break;//              Serial_monitor.print(ch);
          set_year.concat(ch);
        }i++;
       for(;str[i];i++)          {
          char ch = str[i];
          if(ch=='-')
            break;//              Serial_monitor.print(ch);
          set_month.concat(ch);
        }i++;
        for(;str[i];i++)          {
          char ch = str[i];
          if(ch=='T')
            break;//              Serial_monitor.print(ch);
          set_date.concat(ch);
        }i++;
        
        for(;str[i];i++)          {
          char ch = str[i];
          if(ch==':')
            break;//              Serial_monitor.print(ch);
          set_hr.concat(ch);
        }i++;
        for(;str[i];i++)          {
          char ch = str[i];
          if(ch==':')
            break;//              Serial_monitor.print(ch);
          set_mn.concat(ch);
        }i++;
        for(;str[i];i++)          {
          char ch = str[i];
          if(ch=='.')
            break;//              Serial_monitor.print(ch);
          set_sec.concat(ch);
        }
        Serial_monitor.println("SET Parameter: ");
        Serial_monitor.print("Date: "); Serial_monitor.print(set_year);Serial_monitor.print("/");Serial_monitor.print(set_month);Serial_monitor.print("/");Serial_monitor.println(set_date);
        Serial_monitor.print("Time: "); Serial_monitor.print(set_hr);  Serial_monitor.print(":");Serial_monitor.print(set_mn);   Serial_monitor.print(":");Serial_monitor.println(set_sec);
        int check_year = set_year.toInt();
        
        if(check_year >= 2021){
          if(now.minute()==15 | now.minute()==30 | now.minute()==45 | now.minute()==00){
            Serial_monitor.println(F("Updating RTC Time"));
            rtc.adjust(DateTime(set_year.toInt(), set_month.toInt(), set_date.toInt(), set_hr.toInt(), set_mn.toInt(), set_sec.toInt()));
          }
        } 
    }
}


String NTP_time(void)
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial_monitor.println("Failed to obtain time");
    return "TIME FETCH FAILED!";
  }
  Serial_monitor.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial_monitor.print("Day of week: ");
  Serial_monitor.println(&timeinfo, "%A");
  Serial_monitor.print("Month: ");
  Serial_monitor.println(&timeinfo, "%B");
  Serial_monitor.print("Day of Month: ");
  Serial_monitor.println(&timeinfo, "%d");
  Serial_monitor.print("Year: ");
  Serial_monitor.println(&timeinfo, "%Y");
  Serial_monitor.print("Hour: ");
  Serial_monitor.println(&timeinfo, "%H");
  Serial_monitor.print("Hour (12 hour format): ");
  Serial_monitor.println(&timeinfo, "%I");
  Serial_monitor.print("Minute: ");
  Serial_monitor.println(&timeinfo, "%M");
  Serial_monitor.print("Second: ");
  Serial_monitor.println(&timeinfo, "%S"); 
  Serial_monitor.println("Time variables");
//  char timeHour[3];
//  strftime(timeHour,3, "%H", &timeinfo);
//  Serial_monitor.println(timeHour);
//  char timeWeekDay[10];
//  strftime(timeWeekDay,10, "%A", &timeinfo);
//  Serial_monitor.println(timeWeekDay);
  
  char timeHour[3];  strftime(timeHour,3, "%I", &timeinfo);    //  Serial_monitor.println(timeHour);
  char timeMinute[3];  strftime(timeMinute,3, "%M", &timeinfo);  //  Serial_monitor.println(timeMinute);
  char timeSeconds[3];  strftime(timeSeconds,3, "%S", &timeinfo); //  Serial_monitor.println(timeSeconds);
  char timeYear[10];  strftime(timeYear,10, "%Y", &timeinfo);   //  Serial_monitor.println(timeHour);
  
  char timeMonth[10];
  strftime(timeMonth,10, "%B", &timeinfo);
  Serial_monitor.println(timeMinute);
    
  char timeWeekDay[10];
  strftime(timeWeekDay,10, "%d", &timeinfo);
  Serial_monitor.println(timeWeekDay);
  Serial_monitor.println();
  String returnstring =String(String(timeYear) +"-"+ String(timeMonth) +"-"+ String(timeWeekDay)+" "+String(timeHour)+":"+String(timeMinute)+":"+String(timeSeconds)); 
//  Serial_monitor.println(returnstring);
//  Serial_monitor.println();
  return returnstring;
}
