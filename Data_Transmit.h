 /*
 * ADD wifi and data communication related functions
 * 
 * A4 : 10.7.139.140
 * A3 : 10.7.139.122
 * B4 : 10.7.139.112
 * B3 : 10.7.139.109
 */

#include "configure_wifi.h"
//#include "pin_configurations.h"

#include "_RTC.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiMulti.h>
#include "SD_Card.h" 
#include <ArduinoJson.h>
#include <HTTPClient.h> 

//#define DATA_STRING  "HELLO WORLD"

#define D1  1
#define D2  2
#define D3  3
#define D4  4

WiFiClient client;
//
//char* ssid           =  "i Technology";
//char* password       =  "qwerty1234"; 
//char* password       =  "12345678"; 

//char* ssid     = "JioFiber-exyKz";        //"**LMB-HZW-M/C**";**LMB-HZW-WCMS**
//char* password = "ahm2cai7QuahShib";

//const char* IP       =  "i-technology.in";//  "192.168.1.7";//"192.168.1.2";//
//const int  PORT      =  80;//45455;//

char* ssid     = "**LMB-HZW-WCMS**"; //"**LMB-HZW-M/C**";**LMB-HZW-WCMS**
//char* password = "Power@1234";
char* IP       = "10.7.74.158";
int  PORT      = 80;


char *access_point   = "**LMB-HZW-WCMS**";
char *password       = "Power@1234";
char *access_point_2 = "**LMB-HZW-WCMS**";
char *access_point_3 = "**LMB-HZW-WCMS**";
char *access_point_4 = "i Technology";
char *itech_pass     = "qwerty1234";


WiFiMulti wifiMulti;

unsigned long restart_time = 0; 

int Wifi_data_send(String Data_str,int drive);
int check_connection(void);
void Data_Send(String Data_str,int drive);
void check_status_response(String str,int driveid);

void WiFi_RTC_SD_Init(void);
void LED_init(void);
void WIFI_init(char* wifi_ssid, char* wifi_pwd);
int SD_Wifi_Configuration(void);  
void check_response(String str);

int Drive_RESET_FLAG=0;
int Data=0; 
long driver_reading_id=0;
long driver_reading_id_LT=0,driver_reading_id_CT=0,driver_reading_id_MH=0,driver_reading_id_AH=0;
int Fault_1=0;
int Fault_2=0;
unsigned int Response_fault =0;

void WiFi_RTC_SD_Init(void){
  SD_init();
  LED_init();
//  SD_Wifi_Configuration();

  WIFI_init(ssid,password);
  RTC_init();
  Serial_monitor.println(F("Init done!"));
  vTaskDelay(50);
}

void WIFI_init(char* wifi_ssid, char* wifi_pwd){
  wifiMulti.addAP(access_point, password);
  wifiMulti.addAP(access_point_2, password);
  wifiMulti.addAP(access_point_3, password);
  wifiMulti.addAP(access_point_4, itech_pass);
  
  Serial_monitor.print(" Wifi Mac Address: ");  Serial_monitor.println(WiFi.macAddress());
     
  int n = WiFi.scanNetworks();
  Serial_monitor.println("\r\nscan done\r\n");
  if (n == 0)
  {
    Serial_monitor.println("no networks found");
  }
  else
  {
    Serial_monitor.print(" networks found: "); Serial_monitor.println(n);
    for (int i = 0; i < n; ++i)
    {
      Serial_monitor.print(i + 1); Serial_monitor.print(": "); Serial_monitor.print(WiFi.SSID(i)); Serial_monitor.print(" (");  Serial_monitor.print(WiFi.RSSI(i)); Serial_monitor.print(")");
      Serial_monitor.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");            vTaskDelay(100);
    }
  } 
  Serial_monitor.println("\r\n");
  Serial_monitor.print("Connecting to ");
  Serial_monitor.println(wifi_ssid);

  if(wifiMulti.run() == WL_CONNECTED) {
        Serial_monitor.println("");
        Serial_monitor.println("WiFi connected");
        Serial_monitor.println("IP address: ");
        Serial_monitor.println(WiFi.localIP());
    }
    
  int count = 0;
  long  timecount = millis();
  while(wifiMulti.run() != WL_CONNECTED)
  {
    vTaskDelay(500);
    Serial_monitor.print(".");
    if ((millis() - timecount) > 30000)
      break;
  }
  Serial_monitor.print("Local IP: ");
  Serial_monitor.println(WiFi.localIP());
  
}

void LED_init(void){  
  pinMode(Wifi_LED,  OUTPUT);
  pinMode(DATA_PUSH, OUTPUT);
}


int check_connection(void){
  if(wifiMulti.run() != WL_CONNECTED)
  {
//    WiFi.begin(ssid, password);
    digitalWrite(Wifi_LED, HIGH);
    Serial_monitor.println("WIFI : NO\t");
    vTaskDelay(500);
    return 0;
  }
  else
  {
    restart_time=millis();
    digitalWrite(Wifi_LED, LOW);
    Serial_monitor.print("WIFI : YES \t");Serial_monitor.println(WiFi.RSSI());
    Serial_monitor.print("Local IP: ");
    Serial_monitor.println(WiFi.localIP());
    return 1;
  }
}

int data_send_busy = 0;
void Data_Send(String Data_str,int drive){
  data_send_busy =1;
  Serial_monitor.print("Pushing Data to Server: ");  Serial_monitor.println(Data_str);
//  Read_SD_to_send();
  
  if (Wifi_data_send(Data_str,drive))
  {
    digitalWrite(DATA_PUSH, !digitalRead(DATA_PUSH));
    Serial_monitor.println("Data Sent through WiFi");      
  }
  else
  {
    if(Data_str.indexOf("GetFaultRegisters")>-1)
    {
      return ;
    }
    else
    {      
      Serial_monitor.println("Data Send Failed !!");
      Serial_monitor.println("Storing Data !!!");
      Save_to_SD(Data_str);
    }
  }
  data_send_busy =0;
}

int sendData(String url, String postData)
{ 
//  url = "http://www.i-technology.in/BOILER_CMS/Home/SaveIotDriverReading";
  
  url = "http://10.7.74.158/Boiler_CMS/Home/SaveIotDriverReading";
//  postData = Drive_1_status +","+Drive_2_status +","+Drive_3_status +","+Drive_4_status;
  Serial_monitor.print("Request URL ->" + url);
  
//  Serial_monitor.println(url); //Post Data
  String address =  url;
  WiFiClient client;
//  WiFiClientSecure client;
  HTTPClient http;

//  client.setCACert(root_ca);
  Serial_monitor.println("\nStarting connection to server...");
  if (!client.connect(IP, PORT)) {
    Serial_monitor.println("Connection failed!  :( " + String(address));
    return 0;
  } 
  else 
  {
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    auto httpCode = http.POST(postData);
    Serial_monitor.println(httpCode); //Print HTTP return code
    if (httpCode == 200) {
      String payload = http.getString();
      check_response_RTC(payload);  //"LOGTIMESTAMP": "8 Feb, 2022 17:58:29",
      Serial_monitor.flush();
//      Serial_monitor.println("Response: ");Serial_monitor.println(payload); //Print request response payload
    } 
    else 
    {
      Serial_monitor.printf("[HTTP] failed, error: %s\n", http.errorToString(httpCode).c_str());
      return 0;
    }
    http.end(); //Close connection Serial_monitor.println();
    Serial_monitor.println("closing connection");
    return httpCode;
  }
}

int Wifi_data_send(String Data_str, int driveid)  {
//  const char IP[] = ip_LnT;
//  int PORT  = port_LnT; SaveIotDriverReading
//   String App_Link = "GET /Boiler_CMS/Home/SaveDriverReading?driverReading="; //old
   
   if(Data_str.length()< 10) return 0;
   
//   String App_Link = "GET /Boiler_CMS/Home/SaveIotDriverReading?driverReading=";
   String App_Link = "GET /BOILER_CMS/Home/SaveIotDriverReading?driverReading=";
   sendData(App_Link,Data_str);
   return 1;
   
   App_Link.concat(Data_str);
   
   //Samole
//   String App_Link  = "GET /Boiler_CMS/Home/SaveIotDriverReading?driverReading={\"id\":0,\"crane_id\":1,\"driver_id\":4,"
//                      "\"Drive_Type\":\"YASKAWA\",\"Operation_time\":0,\"R_20\":0,\"R_21\":0,\"R_22\":0,\"R_23\":0,\"R_24\":0,\"R_25\":0,\"R_26\":0,\"R_27\":0,\"R_28\":0,\"R_29\":0,\"R_2a\":0,\"R_2b\":0,\"R_2c\":0,\"R_2d\":0,\"R_2e\":0,\"R_2f\":0,\"R_31\":0,\"R_5c\":0,\"R_5d\":0,\"R_50\":0,\"R_9a\":0,\"R_9b\":0,\"R_c1\":0,\"Reading%20Time\":\"2021-11-15%2011:44:11\"}";


//[{\"id\":0,\"crane_id\":1,\"driver_id\":_drive_id + 1,\"Drive_Type\":\"YASKAWA\",\"Operation_time\":0,\"R_20\":0,\"R_21\":0,\"R_22\":0,\"R_23\":0,\"R_24\":0,\"R_25\":0,\"R_26\":0,\"R_27\":0,\"R_28\":0,\"R_29\":0,\"R_2a\":0,\"R_2b\":0,\"R_2c\":0,\"R_2d\":0,\"R_2e\":0,\"R_2f\":0,\"R_31\":0,\"R_5c\":0,\"R_5d\":0,\"R_50\":0,\"R_9a\":0,\"R_9b\":0,\"R_c1\":0,\"Reading%20Time\":\"2021-11-15%2011:44:11\"}",
//[{\"id\":0,\"crane_id\":1,\"driver_id\":_drive_id + 2,\"Drive_Type\":\"YASKAWA\",\"Operation_time\":0,\"R_20\":0,\"R_21\":0,\"R_22\":0,\"R_23\":0,\"R_24\":0,\"R_25\":0,\"R_26\":0,\"R_27\":0,\"R_28\":0,\"R_29\":0,\"R_2a\":0,\"R_2b\":0,\"R_2c\":0,\"R_2d\":0,\"R_2e\":0,\"R_2f\":0,\"R_31\":0,\"R_5c\":0,\"R_5d\":0,\"R_50\":0,\"R_9a\":0,\"R_9b\":0,\"R_c1\":0,\"Reading%20Time\":\"2021-11-15%2011:44:11\"}",
//[{\"id\":0,\"crane_id\":1,\"driver_id\":_drive_id + 3,\"Drive_Type\":\"YASKAWA\",\"Operation_time\":0,\"R_20\":0,\"R_21\":0,\"R_22\":0,\"R_23\":0,\"R_24\":0,\"R_25\":0,\"R_26\":0,\"R_27\":0,\"R_28\":0,\"R_29\":0,\"R_2a\":0,\"R_2b\":0,\"R_2c\":0,\"R_2d\":0,\"R_2e\":0,\"R_2f\":0,\"R_31\":0,\"R_5c\":0,\"R_5d\":0,\"R_50\":0,\"R_9a\":0,\"R_9b\":0,\"R_c1\":0,\"Reading%20Time\":\"2021-11-15%2011:44:11\"}",


  if(wifiMulti.run() != WL_CONNECTED)  {
    DateTime now = rtc.now();
    Serial_monitor.println("WiFi Connected.");
    if (client.available()  | client.connect(IP, PORT))
    {
      Serial_monitor.println(App_Link);
      Serial_monitor.print(F("Server Connected, Pushing local data to server: "));
      Serial_monitor.print(client.remoteIP());
      Serial_monitor.print(':');Serial_monitor.print(PORT);
      vTaskDelay(50);
//      client.print();      
      client.println(App_Link); 
//      Serial_monitor.print("Wifi_Data_send: ");

      long timeout3 = millis();
      while (client.available() == 0)      
      { 
        if (millis() - timeout3 > Server_Timeout)
        {
          Serial_monitor.println(F(">>> Client Timeout !"));
          client.stop();
          String NoResponse = ""; 
          NoResponse.concat("TX : \r\n\r\n"); NoResponse.concat(App_Link);  NoResponse.concat("\r\n\r\nRX : \r\n\r\n No Response");  
          
          if(driveid == D1)      {    WriteFile(NoResponse,"/lt_com.txt");     }
          else  if(driveid == D2){    WriteFile(NoResponse,"/ct_com.txt");     }
          else  if(driveid == D3){    WriteFile(NoResponse,"/mh_com.txt");     }
          else  if(driveid == D4){    WriteFile(NoResponse,"/ah_com.txt");     }
          return 0;
        }
        
        String str = client.readString();
        Serial_monitor.print(str);
        
        if(Data_str.indexOf("driverReading")>-1)
        {
          Serial_monitor.println("\r\n\r\nSaveIotDriverReading\r\n\r\n");
//          check_status_response(str,driveid);
//          if(resetFlag==1)
//          {
//            Reset_Drive(driveid);
//            resetFlag = 0;
//          }
        }
        else if(Data_str.indexOf("GetFaultRegisters")>-1)
        {
          check_response(str);
        }

        if(str.indexOf("Bad Request")>-1){
          return 0;
        }
        
        if (str.indexOf(F(Response)) > -1)
        {
          Serial_monitor.print("\r\nString Captured 1 : OK"); //Serial_monitor.println(str);
          check_response_RTC(str);
          client.stop();
          String NoResponse = ""; 
          NoResponse.concat("TX : \r\n\r\n"); NoResponse.concat(App_Link);  NoResponse.concat("\r\n\r\nRX : \r\n\r\n");
          NoResponse.concat(str);  
          
          if(driveid == D1)      {    WriteFile(NoResponse,"/lt_com.txt");     }
          else  if(driveid == D2){    WriteFile(NoResponse,"/ct_com.txt");     }
          else  if(driveid == D3){    WriteFile(NoResponse,"/mh_com.txt");     }
          else  if(driveid == D4){    WriteFile(NoResponse,"/ah_com.txt");     }
          
          if(str.indexOf("resetFlag\": true")>-1) {
            Drive_RESET_FLAG = 1;
            Serial_monitor.println("Drive reset flag Received");
          }
          else
          {
            Drive_RESET_FLAG = 0; 
            Serial_monitor.println("Drive reset not flag Received");
          }
          return 1;
        }
        if (str.indexOf(F("Bad")) > -1)
        {
          Serial_monitor.println("\r\nString Captured : BAD REQUEST");
          client.stop();
          String NoResponse = ""; 
          NoResponse.concat("TX : \r\n\r\n"); NoResponse.concat(App_Link);  NoResponse.concat("\r\n\r\nRX : \r\n\r\n");
          NoResponse.concat(Response);            
          if(driveid == D1)      {    WriteFile(NoResponse,"/lt_com.txt");     }
          else  if(driveid == D2){    WriteFile(NoResponse,"/ct_com.txt");     }
          else  if(driveid == D3){    WriteFile(NoResponse,"/mh_com.txt");     }
          else  if(driveid == D4){    WriteFile(NoResponse,"/ah_com.txt");     }
          
          return 1;
        }
      }
      client.stop();
    }
    else
    {       
      Serial_monitor.println("Client connect failed");
      client.stop();
      vTaskDelay(50);
      return 0;
    }
  }
  else
  {
    Serial_monitor.println("Wifi Not connected");
    return 0;
  }
}

int Read_SD_to_send(void){
//  Serial_monitor.println(F("Readig SD card"));
  vTaskDelay(50);
//  if(!check_connection())  {return 0;}
  
  File myFile;
  int data_ptr=0;
//  myFile.close();
  if (SD.exists(Data_File))/**/
  {
    if (SD.exists(Ptr_File))
    {
      String ptr = "";
      myFile = SD.open(Ptr_File);
      if (myFile)
      {
        while (myFile.available())
        {
          ptr.concat((char)myFile.read());
        }
        data_ptr = ptr.toInt();
        Serial_monitor.println("Pointer position: " + String(data_ptr));
        myFile.close();
      }
      else
      {
        Serial_monitor.println("Prg_ptr.txt opening error");
      }
    }

    myFile = SD.open(Data_File);
    if (myFile.size() == data_ptr)
    {
      myFile.close(); vTaskDelay(100);
      SD.remove(Data_File);
      SD.remove(Ptr_File);
      data_ptr = 0;
      Serial_monitor.println("File Deleted !!");
      myFile = SD.open("/");
//      printDirectory(myFile, 0);       
      return 0;
    }
    
     if (myFile.size() < data_ptr)
    {
      data_ptr=0;
    }
    String Data_Read_str = "";

    myFile.seek(data_ptr);
    while (myFile.available())
    {
      char ch = myFile.read();
      if (ch == '@')
      {
        Data = Data + 1;
        myFile.read();
        myFile.read();                 //send data to serever
        
        if (Wifi_data_send( Data_Read_str,1) == 1)
        {
          Serial_monitor.println(F("Data Sent over WiFi"));
        }
        else
        {
          myFile.close();
          Serial_monitor.println(F("Client connect failed returning.."));
          return 0;
        }
        Serial_monitor.println("Number of Data is : " + String(Data));
        int pos = myFile.position();
        myFile.close();

        Serial_monitor.println("Current position is: " + String(pos));
        SD.remove(Ptr_File);
        vTaskDelay(100);
        
        myFile = SD.open(Ptr_File, FILE_WRITE);
        if (myFile)
        {
          myFile.print(pos);
          myFile.close();
        }
        else
        {
          Serial_monitor.println(F("prg_ptr.txt opening Error...."));
        }
        vTaskDelay(100);
        Serial_monitor.println("\r\n* Read & Send SD Program End *\r\n");
        return 1;
      }
      else
      {
        Data_Read_str.concat(ch);
      }
    }
  }
  else
  {
    //    Serial_monitor.println(F("Data File does not exists"));
  }
}

void check_status_response(String str,int driveid){
  StaticJsonDocument<100> doc;
  DeserializationError error = deserializeJson(doc, str);

  if (error) {
    Serial_monitor.print(F("deserializeJson() failed: "));
    Serial_monitor.println(error.c_str());
    return;
  }
  
//  const char* errorMessage = doc["errorMessage"];
  String responseStatus = doc["responseStatus"];
  bool resetFlag = doc["resetFlag"];
  driver_reading_id = doc["responseMessage"]["id"];

  if(driveid == 1){ driver_reading_id_LT = doc["responseMessage"]["id"];}
  if(driveid == 2){ driver_reading_id_CT = doc["responseMessage"]["id"];}
  if(driveid == 3){ driver_reading_id_MH = doc["responseMessage"]["id"];}
  if(driveid == 4){ driver_reading_id_AH = doc["responseMessage"]["id"];}
  
//  Serial_monitor.print("errorMessage:\t ");             Serial_monitor.print(errorMessage);
  Serial_monitor.print("responseStatus:\t ");           Serial_monitor.println(responseStatus);
  Serial_monitor.print("driver_reading_id:\t ");        Serial_monitor.println(driver_reading_id);
  Serial_monitor.print("resetFlag: \t");                Serial_monitor.println(resetFlag);

  if(resetFlag==true ){
    Serial_monitor.println("Reset Fault Command found!!");
    Drive_RESET_FLAG = 1;
    vTaskDelay(1000);
  }
  else
  {
    Drive_RESET_FLAG = 0;
  }
}

void scan_connect(){
  Serial_monitor.print(" Wifi Mac Address: ");  Serial_monitor.println(WiFi.macAddress());
  int n = WiFi.scanNetworks();
  Serial_monitor.println("\r\nscan done\r\n");
  if (n == 0)
  {
    Serial_monitor.println("no networks found");
  }
  else
  {
    Serial_monitor.print(" networks found: "); Serial_monitor.println(n);
    for (int i = 0; i < n; ++i)
    {
      String wifiStr = WiFi.SSID(i);      
      Serial_monitor.print(i + 1); Serial_monitor.print(": "); Serial_monitor.print(WiFi.SSID(i)); Serial_monitor.print(" (");  Serial_monitor.print(WiFi.RSSI(i)); Serial_monitor.print(")");
      Serial_monitor.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");            vTaskDelay(100);       
    }
  }
}

void check_response(String str){
  StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, str); 
  // Test if parsing succeeds.
  if (error) {
    Serial_monitor.print(F("deserializeJson() failed: "));
    Serial_monitor.println(error.c_str());
    return;
  }
  Fault_1=0;
  Fault_2=0;
  const char* errorMessage = doc["errorMessage"];
  String responseStatus = doc["responseStatus"];
  bool resetFlag = doc["resetFlag"];
  Fault_1 = doc["responseMessage"]["FAULT_REGISTERS"][0];
  Fault_2 = doc["responseMessage"]["FAULT_REGISTERS"][1];
  Serial_monitor.print("errorMessage:\t ");   Serial_monitor.print(errorMessage);
  Serial_monitor.print("responseStatus:\t "); Serial_monitor.println(responseStatus);
  Serial_monitor.print("resetFlag: \t");      Serial_monitor.println(resetFlag);
  Serial_monitor.print("Fault_1:\t ");        Serial_monitor.println(Fault_1);
  Serial_monitor.print("Fault_2:\t ");        Serial_monitor.println(Fault_2);

  if(resetFlag){  // do fault reset
//    reset_cmd = 1;
  }
  if(Fault_1 !=0)
  {
    Response_fault = 1; 
  }
}

int SD_Wifi_Configuration(void){
/*   
   readFile("/file_pwd.txt");   
//   file_pwd = "";
   for(int i=0;ReadString[i];i++)
   {
     char ch = ReadString[i];
      if(ch!='\r')
      {
        file_pwd[i]=ch;
      }    
   }     
   Serial_monitor.println("wifi pwd : " + String(file_pwd));
   
   readFile("/file_ssid.txt");
//   file_ssid = "";
   for(int i=0;ReadString[i];i++)
   {
     char ch = ReadString[i];
    if(ch!= '\r')
    {
      if(ch=='+')
        file_ssid[i]=' ';
      else
        file_ssid[i]=ch;
    }
   }
   Serial_monitor.println("Wifi SSID: " + String(file_ssid)); 

   readFile("/IP.txt");   
//   file_pwd = "";
   for(int i=0;ReadString[i];i++)
   {
     char ch = ReadString[i];
      if(ch!='\r')
      {
        IP_[i]=ch;
      }    
   }     
   Serial_monitor.println("sever IP : " + String(IP_));


   readFile("/PORT.txt");   
   String PORT_str ="";// ReadString.substring(pos1,pos2);
     for(int i=0;ReadString[i];i++)
     {
       char ch = ReadString[i];
      if(ch>='0' & ch<='9')
      {
        PORT_str.concat(ch);
      }
     }
     PORT = PORT_str.toInt();
     Serial_monitor.println("PORT: " + String(PORT));

    File pfile = SD.open("/PATH.txt",FILE_READ);
    if(pfile){
      PATH = "";
      while(pfile.available())
      {
        char ch = pfile.read();
        if(ch!='\r' & ch !='\n')
        {
          PATH.concat(ch);
        }
      }
      pfile.close();
      Serial_monitor.println("Server Path: " + PATH);
    }
    else{
      Serial_monitor.println(F("path.txt not found"));
    }
    
    Serial_monitor.print("Connecting to :");vTaskDelay(5);  
    Serial_monitor.println(file_ssid);vTaskDelay(5);
                                                //  Serial_monitor.println(ssid);
                                                //  WiFi.begin(ssid, password);
    WiFi.begin(file_ssid, file_pwd); 
    long timeout=millis();
   
    while (WiFi.status() != WL_CONNECTED)
    {
        vTaskDelay(500);
        Serial_monitor.print("."); 
        if((millis()-timeout)>20000)
          break;
    }
    
    if(WiFi.status() == WL_CONNECTED){
    Serial_monitor.println("");
    Serial_monitor.println("WiFi connected");
    Serial_monitor.println("IP address: ");
     ip = WiFi.localIP();
    Serial_monitor.println(ip);
    }
    */
//    file_ssid = "**LMB-HZW-WCMS**";
//    file_pwd =  "Power@1234";
//    IP = "10.7.74.158";
//    PORT      = 80;
}
