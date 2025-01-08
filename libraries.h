
//#include "crane_configuration.h"
//#include "Drive_activity.h"
//#include "SD_Card.h"
//#include "_RTC.h"

#include "pin_configurations.h"
#include "Data_Transmit.h"
#include "OTA.h"
#include "itech_modbus.h"

#define WAIT_SENTENCE "Waiting for : \"COMM_BUSY_FLAG\""

void System_Init(void);
int Serial_command(void);
void Data_Send(String Data_str,int driveid);
int Drive_activity_Detect(void);  
void store_lt(void);
void store_ct(void);
void store_mh(void);
void store_ah(void);


int Data_Push_to_Server_Flag = 0;
int COMM_BUSY_FLAG = 0;

String Main_String = "";
String Stby_String = "";
unsigned int crane_timer = 0;

//byte crane_active_status=0x0000;

void Pin_Initialize(void)
{
 
  pinMode(LT,INPUT_PULLUP);
  pinMode(CT,INPUT_PULLUP);
  pinMode(MH,INPUT_PULLUP);
  pinMode(AH,INPUT_PULLUP);
  
  pinMode(INPUT_5,INPUT_PULLUP);
  pinMode(INPUT_6,INPUT_PULLUP);
  pinMode(INPUT_7,INPUT_PULLUP);
  pinMode(INPUT_8,INPUT_PULLUP);
  
  DSTATUS.lt = 1;//digitalRead(LT);
  DSTATUS.ct = 1;//digitalRead(CT);
  DSTATUS.mh = 1;//digitalRead(MH);
  DSTATUS.ah = 1;//digitalRead(AH);
  
    LED_init(); 
  Serial_monitor.println("Pins Init.\r\n LED Init");
}

void System_Init(void){
  ota_setup();
  Pin_Initialize(); 
}
/*
unsigned long timer_lt =0;
unsigned long timer_ct =0;
unsigned long timer_mh =0;
unsigned long timer_ah =0;

long utilization_time =0;
long utilization_flag = 0;
int cumm_timer_status = 0;
long Regular_store = 0;

int Drive_activity_Detect(void){  
   // Serial_monitor.println("Detecting Drive Activity..");
   // compare and detect the drive on off status
  int drive_monitor_flag = 0;
  if((!(DSTATUS.lt == digitalRead(LT) & DSTATUS.ct == digitalRead(CT) & 
       DSTATUS.mh == digitalRead(MH) & DSTATUS.ah == digitalRead(AH))) )
//       |((millis() - Regular_store)/1000 > 30))
    {
      data_str_send();
      Serial_monitor.println("Drive Status Changed..");
      if(DSTATUS.lt != digitalRead(LT)){
        drive_monitor_flag = D1;
        DSTATUS.lt = digitalRead(LT);  
        if(DSTATUS.lt == LOW) {        timer_lt = millis();      }
        if(DSTATUS.lt == HIGH){        store_lt();               }          
        if(data_send_busy == 1){       while(data_send_busy == 1);}
        Data_Send(Drive_1_status,D1);
//      return drive_monitor_flag;
      }
      else if(DSTATUS.ct != digitalRead(CT))
      {
        drive_monitor_flag = D2;
        DSTATUS.ct = digitalRead(CT);
        if(DSTATUS.ct == LOW) {        timer_ct = millis();      }
        if(DSTATUS.ct == HIGH){        store_ct();               }          
        if(data_send_busy == 1){       while(data_send_busy == 1);}
        Data_Send(Drive_2_status,D2);
//      return drive_monitor_flag;
      }
      else if(DSTATUS.mh != digitalRead(MH))
      {
        drive_monitor_flag = D3;
        DSTATUS.mh = digitalRead(MH);
        if(DSTATUS.mh == LOW) {        timer_mh = millis();      }
        if(DSTATUS.mh == HIGH){        store_mh();               }          
        if(data_send_busy == 1){       while(data_send_busy == 1);}
        Data_Send(Drive_3_status,D3);
//      return drive_monitor_flag;
      }
      else if(DSTATUS.ah != digitalRead(AH))
      {
        drive_monitor_flag = D4;
        DSTATUS.ah = digitalRead(AH);
        if(DSTATUS.ah == LOW) {        timer_ah = millis();      }
        if(DSTATUS.ah == HIGH){        store_ah();               }          
        if(data_send_busy == 1){       while(data_send_busy == 1);}
        Data_Send(Drive_4_status,D4);
//      return drive_monitor_flag;
      }  
    }
    
    if((DSTATUS.ah==LOW) | (DSTATUS.mh==LOW) | (DSTATUS.lt==LOW) | (DSTATUS.ct==LOW)) 
    {
          if(cumm_timer_status == 0){
              cumm_timer_status = 1;
              utilization_time = millis();
          }
    }
    else {
          if(cumm_timer_status == 1){
             cumm_timer_status = 0;
             long utilization_time_buff  = (millis()-utilization_time)/1000;
             crane_timer       = crane_timer + utilization_time_buff;
             Serial_monitor.print(F("operated for time : "));Serial_monitor.println(utilization_time_buff);
             Serial_monitor.print(F("total  Cumm  time : "));Serial_monitor.println(crane_timer);
             WriteFile (String(crane_timer),"/cumm_time.txt");
             utilization_time = 0;
          }
    } 
    if(cumm_timer_status == 1 & ((millis()-Regular_store)/1000)>30) 
    {
       Regular_store = millis();
         long utilization_time_buff  = (millis()-utilization_time)/1000;
         crane_timer       = crane_timer + utilization_time_buff;
         Serial_monitor.print(F("operated for time : "));Serial_monitor.println(utilization_time_buff);
         Serial_monitor.print(F("total  Cumm  time : "));Serial_monitor.println(crane_timer);
         WriteFile (String(crane_timer),"/cumm_time.txt");
       utilization_time = millis();
    }
  return (drive_monitor_flag);
}
*/
void store_ah(void){
   timer_ah = (millis()-timer_ah)/1000;        DSTATUS.ah_time += timer_ah; 
        Serial_monitor.println("AH TOR  : " +String(timer_ah));
        Serial_monitor.println("AH TTOR : " +String(DSTATUS.ah_time));
        WriteFile(String(DSTATUS.ah_time ),"/AH_OT2.txt");
}

void store_mh(void){
   timer_mh = (millis()-timer_mh)/1000;        DSTATUS.mh_time += timer_mh; 
        Serial_monitor.println("MT TOR  : " +String(timer_mh));
        Serial_monitor.println("MT TTOR : " +String(DSTATUS.mh_time));
        WriteFile(String(DSTATUS.mh_time ),"/MH_OT2.txt");
}

void store_ct(void){
   timer_ct = (millis()-timer_ct)/1000;        DSTATUS.ct_time += timer_ct; 
        Serial_monitor.println("CT TOR  : " +String(timer_ct));
        Serial_monitor.println("CT TTOR : " +String(DSTATUS.ct_time));
        WriteFile(String(DSTATUS.ct_time ),"/CT_OT2.txt");
}

void store_lt(void){
   timer_lt = (millis()-timer_lt)/1000;        DSTATUS.lt_time += timer_lt; 
        Serial_monitor.println("LT TOR  : " +String(timer_lt));
        Serial_monitor.println("LT TTOR : " +String(DSTATUS.lt_time));
        WriteFile(String(DSTATUS.lt_time ),"/LT_OT2.txt");
}
int Serial_command(void)
{
  DateTime now = rtc.now();
  if (Serial_monitor.available())
  {
    String Serial_str = Serial_monitor.readString();
    if ((Serial_str.indexOf("restart") > -1)|(Serial_str.indexOf("RESTART") > -1))
    { 
      if(digitalRead(LT)==HIGH){      }
      if(digitalRead(CT)==HIGH){      }
      if(digitalRead(MH)==HIGH){      }
      if(digitalRead(AH)==HIGH){      }
      utilization_time = (millis()-utilization_time)/1000;
      crane_timer      += utilization_time;
      Serial_monitor.print(F("operated for time : "));Serial_monitor.println(utilization_time);    
      Serial_monitor.print(F("total  Cumm  time : "));Serial_monitor.println(crane_timer);    
      WriteFile (String(crane_timer),"/cumm_time.txt");
      ESP.restart();
    }
    if ((Serial_str.indexOf("STOP") > -1)|(Serial_str.indexOf("stop") > -1))
      return 0; 
    if(Serial_str.indexOf("time")>-1){
      Serial_monitor.println(String(now.day()) + "/" + String(now.month()) + "/" + String(now.year()) + ',' + String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second()));
    } 
    if (Serial_str.indexOf("RTC") > -1)
    {
      Serial_monitor.println(Serial_str);
      int rt = Serial_str.indexOf("(") + 1;

      String def;
      for (rt; Serial_str[rt]; rt++)
      {
        if (Serial_str[rt] >= '0' && Serial_str[rt] <= '9')
          def.concat(Serial_str[rt]);
      }
      String dt_str, tm_str;
      for (int i = 0; def[i]; i++)
      {
        if (i < 6)
          dt_str.concat(def[i]);
        else
          tm_str.concat(def[i]);
      }
      Serial_monitor.println("Date:" + dt_str);
      Serial_monitor.println("Time:" + tm_str);
      long timeVar = tm_str.toInt();
      long dateVar = dt_str.toInt();
      Serial_monitor.println(timeVar);

      int yr =  dateVar / 10000;
      int mo = ((int)(dateVar / 100)) % 100;
      int dy =  dateVar % 100;
      int hr = timeVar / 10000;
      int mn = ((int)(timeVar / 100)) % 100;
      int se = timeVar % 100;

//      Serial_monitor.println(String(yr) + "/" + String(mo) + "/" + String(dy) + " " + String(hr) + ":" + String(mn) + ":" + String(se));
      rtc.adjust(DateTime(yr, mo, dy, hr, mn, se));
      vTaskDelay(500);
      now = rtc.now();
      Serial_monitor.print("New Time :");
      Serial_monitor.println(String(now.day()) + "/" + String(now.month()) + "/" + String(now.year()) + ',' + String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second()));
      //  rtc.adjust(DateTime(19,12,25,12,52,00));
      //  RTC(19,12,25,12,52,00)
    }
    if((Serial_str.indexOf("reset_time") > -1)){
      int reset_count = 0;
      SD.remove("/MH_OT2.txt");
      SD.remove("/AH_OT2.txt");
      SD.remove("/LT_OT2.txt");
      SD.remove("/CT_OT2.txt");
      SD.remove("/cumm_time.txt");
      
      WriteFile(String(reset_count),"/MH_OT2.txt");
      WriteFile(String(reset_count),"/AH_OT2.txt");
      WriteFile(String(reset_count),"/CT_OT2.txt");
      WriteFile(String(reset_count),"/LT_OT2.txt");
      WriteFile(String(reset_count),"/cumm_time.txt");
    }
    if ((Serial_str.indexOf("DELETE") > -1)|(Serial_str.indexOf("delete") > -1))
    {
      SD.remove("/data.txt");
      SD.remove("/prg_ptr.txt");
      Serial_monitor.println(F("\r\n\r\n /data.txt and /prg_ptr.txt Files deleted"));
    }
  }
  return 1;
}
