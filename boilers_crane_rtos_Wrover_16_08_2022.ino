//565.9992
//SaveIotDriverReading
#define Serial_monitor  Serial2

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#define CRANE_ID  "CRANE A4"
#include "libraries.h"
#include "file_system.h"
#define ALL_REPORT  6
long DataSend_Time =0;
int Send_data_from_another_task = 0;
int Activated_drive = 0;
void DRIVE_ACTIVE_TASK( void *pvParameters );
void MODBUS_READ_TASK( void *pvParameters );
void DATA_COMMUNICATION_TASK( void *pvParameters );

void Send_Drive_Data_To_Server(byte dri);
void log_data_to_file(void);
long system_uptime = 0;
long second_timeout = 0;
 
unsigned int drive_staus[6];
unsigned int run_timer[6];
int _cum = 5;
int _lt = 1, _ct = 2, _mh = 3, _ah = 4;

void setup() 
{
  system_uptime = millis();
  Serial_monitor.begin(115200);
  Serial_monitor.println("CRANE MONITORING SYSTEM FOR : " + String(CRANE_ID));
//  Serial_monitor.println("Configuring System");
  NTP_init();
  Task_Configuration();
  Pin_Initialize();
  WiFi_RTC_SD_Init();
  System_Init();
  
//  SPIFFS_setup();
  pinMode(LT,INPUT_PULLUP);
  pinMode(CT,INPUT_PULLUP);
  pinMode(MH,INPUT_PULLUP);
  pinMode(AH,INPUT_PULLUP);  
  delay(100);
  
        
  xTaskCreatePinnedToCore(DRIVE_ACTIVE_TASK       ,"ReadDriveActivity"    ,25720 , NULL , 3 ,  NULL  ,  ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(MODBUS_READ_TASK        ,"ReadModbusSlaves"     ,45720 , NULL , 3 ,  NULL  ,  ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(DATA_COMMUNICATION_TASK ,"ServerDataSend"       ,45720 , NULL , 3 ,  NULL  ,  ARDUINO_RUNNING_CORE);
}

void loop()
{
}

void DRIVE_ACTIVE_TASK(void *pvParameters){
  (void) pvParameters;
  Serial_monitor.print(F("Drive MONITOR TASK READY"));
  for (;;)                                              // A Task shall never return or exit.
  {    
    update_rtc();
    ota_loop();
    Serial_command();                                   //    check_connection();
//    Data_Push_to_Server_Flag = Drive_activity_Detect(); //    Activated_drive = Data_Push_to_Server_Flag;
    
    if(millis() - second_timeout >1000){
      Data_Push_to_Server_Flag = Drive_run_timer(); 
      second_timeout = millis(); 
    }
    
    crane_timer_buff = crane_timer;
    if(Data_Push_to_Server_Flag>0)
    {      
        if(Send_data_from_another_task>0)        {
            long s = millis();
            while((millis()-s) <5000)                 {
               if(Send_data_from_another_task == 0)              break;
               else                                            vTaskDelay(15);
            }
        }
      Serial_monitor.print(F("Drive ON : "));      
      Serial_monitor.println(Data_Push_to_Server_Flag); 
      Send_data_from_another_task = 1;
      Serial_monitor.println("Data_Push_to_Server_Flag : " + String (Data_Push_to_Server_Flag));
      Activated_drive = Data_Push_to_Server_Flag;
    }
    
    if((millis()-system_uptime)/1000 > 3600)
    {
      if(digitalRead(LT)==HIGH & digitalRead(CT)==HIGH & digitalRead(MH)==HIGH & digitalRead(AH)==HIGH){
         Operation_time_write();
         ESP.restart();
      }
    }
//    else

    vTaskDelay(100);   // one tick delay(15ms) in between reads for stability
  }
}

void MODBUS_READ_TASK(void *pvParameters)  // This is a task.
{
  (void) pvParameters;  
  init_modbus_packet();  
  Fault_Monitor();  
  modbus_configuration();
  modbus_configuration();  // making Sure it is done
  KWH_packets();
  init_modbus_packet();  
  Operation_time_read();
  operation_time_from_fault_packets();//    operation_time_packets();
  DC_BUS_VTG();
  Fan_operation_time_packets();
  
  Serial_monitor.println("TASK : MODBUS READ READY");
  for (;;)  
  {
    Read_Modbus_Registers(); 
    if(Send_data_from_another_task ==1){
//      long o_time = millis();
//      while(millis()-o_time <3500){
      init_modbus_packet();  
//      }
    } 
    vTaskDelay(100);  // one tick delay (15ms) in between reads for stability
  }
}

void DATA_COMMUNICATION_TASK(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  long time_to_send   = 0;
  long check_timeout  = 0;
  int data_timeout    = 30;
  Serial_monitor.println("DATA COM TASK READY");
  unsigned int data_log_status[5]={0,0,0,0,0};
  for (;;)
  {
    if(millis() - check_timeout > 5000){
      Serial_monitor.println(NTP_time());
      check_timeout = millis();
      check_connection();
//      Serial_monitor.print("LT : ");  Serial_monitor.print((DSTATUS.lt)?"OFF":"ON"); Serial_monitor.print("\tTime : ");Serial_monitor.println(DSTATUS.lt_time);
//      Serial_monitor.print("CT : ");  Serial_monitor.print((DSTATUS.ct)?"OFF":"ON"); Serial_monitor.print("\tTime : ");Serial_monitor.println(DSTATUS.ct_time);
//      Serial_monitor.print("MH : ");  Serial_monitor.print((DSTATUS.mh)?"OFF":"ON"); Serial_monitor.print("\tTime : ");Serial_monitor.println(DSTATUS.mh_time);
//      Serial_monitor.print("AH : ");  Serial_monitor.print((DSTATUS.ah)?"OFF":"ON"); Serial_monitor.print("\tTime : ");Serial_monitor.println(DSTATUS.ah_time);
    }
    
    if((((millis()-time_to_send)/1000 > data_timeout) | (Send_data_from_another_task == 1)))
    {
      Serial_monitor.println(F("Sending data to the Server"));
//      delay(2000);
//      init_modbus_packet();
      data_str_send();                        //      Data_Send(Main_String,Data_Push_to_Server_Flag);
      
      if((millis()-time_to_send)/1000 > data_timeout){
//        init_modbus_packet();
        Send_Drive_Data_To_Server(ALL_REPORT); 
      }
      
      init_modbus_packet();
      if((drive_staus[_lt]!=data_log_status[_lt])|(drive_staus[_lt]==0))      {      data_log_status[_lt] = drive_staus[_lt]; Send_Drive_Data_To_Server(_lt); }
      if((drive_staus[_ct]!=data_log_status[_ct])|(drive_staus[_ct]==0))      {      data_log_status[_ct] = drive_staus[_ct]; Send_Drive_Data_To_Server(_ct); }
      if((drive_staus[_mh]!=data_log_status[_mh])|(drive_staus[_mh]==0))      {      data_log_status[_mh] = drive_staus[_mh]; Send_Drive_Data_To_Server(_mh); }
      if((drive_staus[_ah]!=data_log_status[_ah])|(drive_staus[_ah]==0))      {      data_log_status[_ah] = drive_staus[_ah]; Send_Drive_Data_To_Server(_ah); }
      
      data_str_send();    //      Send_Drive_Data_To_Server(Data_Push_to_Server_Flag);
      Send_data_from_another_task = 0;

//      if((millis()-restart_time)/1000>900)    
//      {
//        Operation_time_write();
//        ESP.restart();      
//      }  
      time_to_send = millis();
    }
//    vTaskDelay(100);  // one tick delay (15ms) in between reads for stability
  }
}


void Operation_time_read(void){
  String ReadString = "";
  if(SD_Write_flag==1){    while(SD_Write_flag==1);    vTaskDelay(500);  }
  SD_Write_flag =1;
    ReadString  = readFile("/LT_OT2.txt");   DSTATUS.lt_time = ReadString.toInt();
    ReadString  = readFile("/CT_OT2.txt");   DSTATUS.ct_time = ReadString.toInt();
    ReadString  = readFile("/MH_OT2.txt");   DSTATUS.mh_time = ReadString.toInt();
    ReadString  = readFile("/AH_OT2.txt");   DSTATUS.ah_time = ReadString.toInt();
    ReadString  = readFile("/cumm_time.txt");    crane_timer = ReadString.toInt();
  SD_Write_flag =0;
  
  Serial_monitor.print("LT_O_time: ");Serial_monitor.print(DSTATUS.lt_time);Serial_monitor.println(" Sec");
  Serial_monitor.print("CT_O_time: ");Serial_monitor.print(DSTATUS.ct_time);Serial_monitor.println(" Sec");
  Serial_monitor.print("MH_O_time: ");Serial_monitor.print(DSTATUS.mh_time);Serial_monitor.println(" Sec");
  Serial_monitor.print("AH_O_time: ");Serial_monitor.print(DSTATUS.ah_time);Serial_monitor.println(" Sec");
}

void Operation_time_write(void){
  if(SD_Write_flag==1){    while(SD_Write_flag==1);    vTaskDelay(500);  }
  SD_Write_flag =1;
    WriteFile(String(DSTATUS.lt_time ),"/LT_OT2.txt");
    WriteFile(String(DSTATUS.ct_time ),"/CT_OT2.txt");
    WriteFile(String(DSTATUS.mh_time ),"/MH_OT2.txt");
    WriteFile(String(DSTATUS.ah_time ),"/AH_OT2.txt");  
  SD_Write_flag =1;  
}

void Read_Modbus_Registers (void){
  if(Send_data_from_another_task==1){
//    delay(3500);
//    init_modbus_packet();  
//    Send_data_from_another_task = 2;
  } 
    if((millis()-DataSend_Time)/1000>3600)
    {
      ESP.restart();
    }
    init_modbus_packet();  
    Fault_Monitor(); 
    DataSend_Time=millis();delay(5);
    
    if(!(digitalRead(LT)| digitalRead(CT)| digitalRead(MH)| digitalRead(AH))){
      KWH_packets();
    }
    delay(5);
    operation_time_from_fault_packets();//    operation_time_packets();
    DC_BUS_VTG();
    Fan_operation_time_packets();
//  }
//    data_str_send();  
//  log_data_to_file(); 
}

void log_data_to_file(void){
  String log_str = "Cumulative Op Time : ";  
  log_str.concat(crane_timer_buff);log_str.concat("\r\n\r\nLT : Op. T : ");log_str.concat(DSTATUS.lt_time);log_str.concat("\r\n\r\n");
  log_str.concat(Drive_1_status);  log_str.concat("\r\n\r\nCT : Op. T : ");log_str.concat(DSTATUS.ct_time);log_str.concat("\r\n\r\n");
  log_str.concat(Drive_2_status);  log_str.concat("\r\n\r\nMH : Op. T : ");log_str.concat(DSTATUS.mh_time);log_str.concat("\r\n\r\n");
  log_str.concat(Drive_3_status);  log_str.concat("\r\n\r\nAH : Op. T : ");log_str.concat(DSTATUS.ah_time);log_str.concat("\r\n\r\n");
  log_str.concat(Drive_4_status);  log_str.concat("\r\n");
                          //  WriteFile(String(log_str),"/log_data.txt");
  SD.remove("/data_log.txt");
  Save_to_SD(log_str);    //  Operation_time_write();
}

void Send_Drive_Data_To_Server(byte dri)
{
    Serial_monitor.println("Activated Drive : " + String(dri));delay(50);
    data_str_send(); 
    
    log_data_to_file(); 
     String postData = "["+Drive_1_status +","+Drive_2_status +","+Drive_3_status +","+Drive_4_status+"]";
    if(dri==1 | dri==6){
      Data_Send(postData/*Drive_1_status*/,D1);    if(Drive_RESET_FLAG==1) {  Reset_Drive(D1); Drive_RESET_FLAG=0;  }  //    vTaskDelay(500);
    }
    if(dri==2| dri==6){
      Data_Send(postData/*Drive_2_status*/,D2);    if(Drive_RESET_FLAG==1) {  Reset_Drive(D2); Drive_RESET_FLAG=0;  }  //    vTaskDelay(500);  
    }
    if(dri==3| dri==6){
       Data_Send(postData/*Drive_3_status*/,D3);    if(Drive_RESET_FLAG==1) {  Reset_Drive(D3); Drive_RESET_FLAG=0;  }  //    vTaskDelay(500);
    }
    if(dri==4| dri==6){    
      Data_Send(postData/*Drive_4_status*/,D4);    if(Drive_RESET_FLAG==1) {  Reset_Drive(D4); Drive_RESET_FLAG=0;  }  
    }
}

long time_gap = 0;
int Drive_run_timer(void)
{
//  int offset_sec
  Serial_monitor.println("Drive Run TIme Counter :)");  
  drive_staus[_lt] = digitalRead(LT);
  drive_staus[_ct] = digitalRead(CT);
  drive_staus[_mh] = digitalRead(MH);
  drive_staus[_ah] = digitalRead(AH);
  
  int gap_time = ((millis()-time_gap));
  time_gap = millis();
  unsigned long duration = pulseIn(LT, HIGH);
  Serial_monitor.print("Pulse HIGH : ");
  Serial_monitor.println(duration);
  if(drive_staus[_lt] == 0){ 
    run_timer[_lt]+=gap_time;  
    }
  else{ 
    if(run_timer[_lt] > 0) { 
//      int offset_sec = (run_timer[_lt]/60)*7;
      DSTATUS.lt_time = DSTATUS.lt_time + (run_timer[_lt]/1000); 
      WriteFile(String(DSTATUS.lt_time),"/LT_OT2.txt"); 
      run_timer[_lt]=0;
    }
  }
  if(drive_staus[_ct] == 0){ run_timer[_ct]+=gap_time;  }
  else{ 
    if(run_timer[_ct] > 0) {
//      int offset_sec = (run_timer[_ct]/60)*7;
      DSTATUS.ct_time = DSTATUS.ct_time + (run_timer[_ct]/1000);
      WriteFile(String(DSTATUS.ct_time),"/CT_OT2.txt");
      run_timer[_ct]=0;
    }
  }
  if(drive_staus[_mh] == 0){ run_timer[_mh]+=gap_time;  }
  else{ 
    if(run_timer[_mh] > 0) {
//      int offset_sec = (run_timer[_mh]/60)*7; 
      DSTATUS.mh_time = DSTATUS.mh_time + (run_timer[_mh]/1000);
      WriteFile(String(DSTATUS.mh_time),"/MH_OT2.txt");
      run_timer[_mh]=0;
    }
  }
  if(drive_staus[_ah] == 0){ run_timer[_ah]+=gap_time;  }
  else{
    if(run_timer[_ah] > 0) {
//      int offset_sec = (run_timer[_ah]/60)*7;
      DSTATUS.ah_time = DSTATUS.ah_time + (run_timer[_ah]/1000);
      WriteFile(String(DSTATUS.ah_time ),"/AH_OT2.txt");
      run_timer[_ah]=0;
    }
  }
//  vTaskDelay(10);
  Serial_monitor.println("\r\nINPUT STATUS : \t");
  Serial_monitor.print("Drives :\t"); 
  Serial_monitor.println("LT\tCT\tMH\tAH");
  Serial_monitor.print("Status :\t"); 
  for(int i=1;i<5;i++)  {   (!drive_staus[i])?Serial_monitor.print("ON"):Serial_monitor.print("OFF");  Serial_monitor.print('\t');}
  Serial_monitor.println();
  
  Serial_monitor.print("On time:\t"); 
  Serial_monitor.print(run_timer[_lt]);Serial_monitor.print('\t');
  Serial_monitor.print(run_timer[_ct]);Serial_monitor.print('\t');
  Serial_monitor.print(run_timer[_mh]);Serial_monitor.print('\t');
  Serial_monitor.print(run_timer[_ah]);Serial_monitor.println("\r\n");

  Serial_monitor.print("Total On:\t");
  Serial_monitor.print(DSTATUS.lt_time);Serial_monitor.print('\t');
  Serial_monitor.print(DSTATUS.ct_time);Serial_monitor.print('\t');
  Serial_monitor.print(DSTATUS.mh_time);Serial_monitor.print('\t');
  Serial_monitor.print(DSTATUS.ah_time);Serial_monitor.println("\r\n");
  
  Serial_monitor.print("Cum:\t ");  
  Serial_monitor.print(run_timer[_cum]);Serial_monitor.print('\t');Serial_monitor.println(crane_timer);
  Serial_monitor.println();
  int current_status[5];
  
  if(drive_staus[_lt]==0 | drive_staus[_ct]==0 | drive_staus[_mh]==0 | drive_staus[_ah]==0)
  {
    run_timer[_cum]+=gap_time; 
    if(drive_staus[_lt]==0) return 1;
    if(drive_staus[_ct]==0) return 2;
    if(drive_staus[_mh]==0) return 3;
    if(drive_staus[_ah]==0) return 4; 
  }
  else
  {
    if(run_timer[_cum] > 0) {
      crane_timer = crane_timer + (run_timer[_cum]/1000);
      WriteFile(String(crane_timer),"/cumm_time.txt");
      run_timer[_cum]=0;
    }
  }
  return 0;
}
