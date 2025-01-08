#include <ArduinoJson.h>
#include "crane_configuration.h"

//#define baud 19200
//
//#define YASKAWA 1
//#define LNT_FLE 2
//
//unsigned int LT_type = YASKAWA ;
//unsigned int CT_type = YASKAWA ;
//unsigned int MH_type = YASKAWA ;
//unsigned int AH_type = LNT_FLE ;
//
//#define Device_id 0
//
//#define D1  1
//#define D2  2
//#define D3  3
//#define D4  4
unsigned int crane_timer_buff =0;

#define LT_DC_BUS   0x0314
#define LT_OP_AMP   0x0008
#define LT_AC_Vtg   0x000A
#define LT_OP_Frq   0x0009
#define LT_PO_KWH   0x032D
#define LT_O_Time   0x0342
#define LT_ST_REG   0x0008  //ON-OFF  0x0309 previous 0x0304
#define LT_OTIME_FF 0x0344
#define LT_F_RESET  0x0006  // 0x0006 for CX series and 0X0381 for FX series
#define LT_FAULT_R  0x032f
#define LT_FAN_OP   0x0344

int LNT_DRIVE_ADDR[10]={
                        LT_DC_BUS,
                        LT_OP_AMP,
                        LT_AC_Vtg,
                        LT_OP_Frq,
                        LT_PO_KWH,
                        LT_O_Time,
                        LT_ST_REG,
                        LT_OTIME_FF,
                        LT_F_RESET,
                        LT_FAULT_R
                      };
                      
#define YK_DC_BUS      49
#define YK_OP_AMP       0
#define YK_AC_Vtg       0
#define YK_OP_Frq       0
#define YK_PO_KWH      92
#define YK_O_TIME      80
#define YK_ST_REG      32
#define YK_O_TIME_FF   141
#define YK_F_RESET     0x0001
#define YK_FAULT_R     192        
#define YK_FAN_OP      154
 
int YAS_DRIVE_ADDR[10]={ 
                        YK_DC_BUS,
                        YK_OP_AMP,
                        YK_AC_Vtg,
                        YK_OP_Frq,
                        YK_PO_KWH,
                        YK_O_TIME,
                        YK_ST_REG,
                        YK_O_TIME_FF,
                        YK_F_RESET,
                        YK_FAULT_R
                      };

typedef struct Drive_Status{
  int lt;
  int ct;
  int mh;
  int ah;
  unsigned int lt_time = 0;
  unsigned int ct_time = 0;
  unsigned int mh_time = 0;
  unsigned int ah_time = 0;  
}DriveStatus;

DriveStatus DSTATUS;



//YK_O_TIME_FF
#define LnT_Status_Read 8
#define Status_Read    16
#define kwh_read        2

#define YK_Status_Read    16
#define LT_Status_Read    8


#define Modbus_update_loop  16

//int LT_RS485_REG_ADDR[];
//int CT_RS485_REG_ADDR[];
//int MH_RS485_REG_ADDR[];
//int AH_RS485_REG_ADDR[];

char crane_id =1;
int Reset_flag=0;
int fault_drive_LT=0;
int fault_drive_CT=0;
int fault_drive_MH=0;
int fault_drive_AH=0;


unsigned int D1_Faults[2];
unsigned int D2_Faults[2];
unsigned int D3_Faults[2];
unsigned int D4_Faults[2];

int value =0;
unsigned int Response_fault_2 =0;
unsigned int No_Of_Fault_Regs=6;

#define modbus_timeout 220           // 151 RS485
#define polling 110                   // 50 RS485

#define retry_count 10 

#define TxEnablePin 27

//#define Status_Reg  75

#define Fault_Reg   192
#define Fault_Read  16
#define LnT_Fault_Read  8

//DC bus voltage
#define DC_bus_voltage 49

//Power consumption
#define KWH_L 0x005C
#define KWH_H 0x005D

unsigned int KWH1[2];
unsigned int KWH2[2];
unsigned int KWH3[2];
unsigned int KWH4[2];


unsigned int KWH1_BUFF[2];
unsigned int KWH2_BUFF[2];
unsigned int KWH3_BUFF[2];
unsigned int KWH4_BUFF[2];


unsigned int otf1[2];
unsigned int otf2[2];
unsigned int otf3[2];
unsigned int otf4[2];

unsigned int ot1[2];
unsigned int ot2[2];
unsigned int ot3[2];
unsigned int ot4[2];

unsigned int fot1[2];
unsigned int fot2[2];
unsigned int fot3[2];
unsigned int fot4[2];

unsigned int DCV1[2];
unsigned int DCV2[2];
unsigned int DCV3[2];
unsigned int DCV4[2];


  unsigned int drive1[16];
  unsigned int drive2[16];
  unsigned int drive3[16];
  unsigned int drive4[16];

unsigned int R_regs[7];
//cumulative operation time
#define A_COT 0x004C
#define F_COT 0x008D

#include "SimpleModbusMaster.h"

String Drive_1_fault="";
String Drive_2_fault="";
String Drive_3_fault="";
String Drive_4_fault="";

String Drive_1_status="";
String Drive_2_status="";
String Drive_3_status="";
String Drive_4_status="";
unsigned int connection_status = 0;

enum
{
  PACKET1,
//  PACKET2,
//  PACKET3,
//  PACKET4,
  TOTAL_NO_OF_PACKETS
};

Packet packets[TOTAL_NO_OF_PACKETS];

packetPointer packet1 = &packets[PACKET1];
//packetPointer packet2 = &packets[PACKET2];
//packetPointer packet3 = &packets[PACKET3];
//packetPointer packet4 = &packets[PACKET4];


#define YASKAWA_RESET_FLAG_ADDR  32
#define RESET_FLAG_NO_REGS        2

#define LnT_RESET_FLAG_ADDR       32
#define LnT_Fault_Reg             809 //0x0329
#define YKW_Fault_Reg             192

#define YKW_Fault_Reset_Reg       1 // 3rd bit 1000
#define LnT_Fault_Reset_Reg       0x0006 //809  // 3rd bit 1000

unsigned int Count;

typedef struct packet_write {
  int id;
  int function;   //=PRESET_MULTIPLE_REGISTERS;
  int addr;
  int Num_Of_Regs;
  unsigned int Array[3];
} RESET_PLC;

RESET_PLC Drive;

int Reset_Drive (int D_NO );
//String Prepare_Data_String(int id, char crane_id,int driver_id,int run_,int Drive_Output_freq,int Drive_output_vtg,int Drive_output_crnt,int DC_vtg,int Power,int Heat_Sink,int _Fault,int Drive_status);
String Prepare_Data_String(int id,  char crane_id,int driver_id,int run_,int Drive_output_vtg,int Drive_output_crnt,int Power,int Heat_Sink, int _Fault,int Drive_status);
void KWH_packets(void);
void operation_time_from_fault_packets(void);
void operation_time_packets(void);
void init_modbus_packet(void);
void Fault_packet_set(void);
void Fan_operation_time_packets(void);
int find_faults(int regC0, int regC1);
int status_faults(void);
void modbus_configuration(void);
int Modbus_Update(void);
void Modbus_Data_Com(void);
void data_str_send(void);
void set_packet_parameter(struct packet_write AC_DRIVE);
unsigned int FindDriveType(unsigned int Drive_No);
String Reg_Data_string_json(bool drun,int Drive_No,String time_str);
void Fault_Monitor(void);
void Read_fault(int driver);

void Fault_Monitor(void){ 
  Read_fault(D1);  for (int i = 0; i < RESET_FLAG_NO_REGS; i++) {     D1_Faults[i] = R_regs[i];  }
  Read_fault(D2);  for (int i = 0; i < RESET_FLAG_NO_REGS; i++) {     D2_Faults[i] = R_regs[i];  }
  Read_fault(D3);  for (int i = 0; i < RESET_FLAG_NO_REGS; i++) {     D3_Faults[i] = R_regs[i];  }
  Read_fault(D4);  for (int i = 0; i < RESET_FLAG_NO_REGS; i++) {     D4_Faults[i] = R_regs[i];  }
}

void modbus(void)
{
  modbus_configuration();
  for(int k=0;k<Modbus_update_loop;k++)
  {
    connection_status = modbus_update(packets);
//    delayMicroseconds(2000);
      delay(5);                  //for    RS485 5
  }
}

void check_response_fault(int Addr){
   Serial2.begin(19200,SERIAL_8E1);  
  
  packet1->id = D1; 
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = Addr;
  packet1->no_of_registers = No_Of_Fault_Regs;
  packet1->register_array = buffer_reg1;
  modbus();

  packet1->id = D2;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = Addr;
  packet1->no_of_registers = No_Of_Fault_Regs;
  packet1->register_array = buffer_reg2;
  modbus();
  
  packet1->id = D3;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = Addr;
  packet1->no_of_registers = No_Of_Fault_Regs;
  packet1->register_array = buffer_reg3;
  modbus();
  
  packet1->id = D4;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = LT_FAULT_R;//Addr; //
  packet1->no_of_registers = No_Of_Fault_Regs;
  packet1->register_array = buffer_reg4;
  modbus();
   
  Serial_monitor.println("Buffer Read : ");
  for(int i=0;i<16;i++){
   Serial_monitor.print(buffer_reg1[i]);Serial_monitor.print('\t');Serial_monitor.print(buffer_reg2[i]);Serial_monitor.print('\t');Serial_monitor.print(buffer_reg3[i]);Serial_monitor.print('\t');Serial_monitor.print(buffer_reg4[i]);Serial_monitor.println();
  }
  
  Serial_monitor.println();
//  http://localhost:50903/Boiler_CMS/Home/SaveFaultReading?faultReading={"craneId":1,"driverId":1,"driver_reading_id":1,"value":1637,"fault_register_Address":192}
//  http://192.168.1.100/Boiler_CMS/Home/SaveDriverReading?driverReading={"id":0,"crane_id":1,"driver_id":1,"on_off":true,"output_freq":10.31,"ac_voltage":10.2,"output_current":1.5,"dc_voltage":5.5,"heat_sink":true,"reading_time":null,"system_time":null,"fault":false,"driverReading":0}
//  String Data_str = "GET /Boiler_CMS/Home/SaveFaultReading?faultReading="
//                    "{\"craneId\":" +String(1)+
//                    ",\"driverId\":" + String(1)+
//                    ",\"driver_reading_id\":"+String(1)+
//                    ",\"value\":"+String(1637)+
//                    ",\"fault_register_Address\":" +String(192) + 
//                    "}";
}

int first_time_read = 0;

/****************************************************************** KWH *************************************************************/
void KWH_packets(void)
{
  
  modbus_configuration();
  
  packet1->id = D1;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = (LT_type==LNT_FLE)?LT_PO_KWH:YK_PO_KWH;
  packet1->no_of_registers = kwh_read;
  packet1->register_array = KWH1_BUFF;  //KWH1;
  modbus();
  
  packet1->id = D2;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = (CT_type==LNT_FLE)?LT_PO_KWH:YK_PO_KWH;
  packet1->no_of_registers = kwh_read;
  packet1->register_array = KWH2_BUFF; //KWH2;
  modbus();
  packet1->id = D3;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = (MH_type==LNT_FLE)?LT_PO_KWH:YK_PO_KWH;
  packet1->no_of_registers = kwh_read;
  packet1->register_array = KWH3_BUFF; //KWH3;
  modbus();
  packet1->id = D4;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = (AH_type==LNT_FLE)?LT_PO_KWH:YK_PO_KWH;//
  packet1->no_of_registers = kwh_read;
  packet1->register_array = KWH4_BUFF; //KWH4;
  modbus();
  
  // kwh change here
  (LT_type==LNT_FLE)?Serial_monitor.println("\tKWH 1 : " +String(KWH1_BUFF[1])+String(KWH1_BUFF[0]*0.1)):Serial_monitor.print("\nKWH 1 : " +String(KWH1_BUFF[1])+String(KWH1_BUFF[0]));
  (CT_type==LNT_FLE)?Serial_monitor.println("\tKWH 2 : " +String(KWH2_BUFF[1])+String(KWH2_BUFF[0]*0.1)):Serial_monitor.print("\tKWH 2 : " +String(KWH2_BUFF[1])+String(KWH2_BUFF[0]));
  (MH_type==LNT_FLE)?Serial_monitor.println("\tKWH 3 : " +String(KWH3_BUFF[1])+String(KWH3_BUFF[0]*0.1)):Serial_monitor.print("\tKWH 3 : " +String(KWH3_BUFF[1])+String(KWH3_BUFF[0]));
  (AH_type==LNT_FLE)?Serial_monitor.println("\tKWH 4 : " +String(KWH4_BUFF[1])+String(KWH4_BUFF[0]*0.1)):Serial_monitor.print("\tKWH 4 : " +String(KWH4_BUFF[1])+String(KWH4_BUFF[0]));

  if( KWH1[0]==0 & KWH1[1]==0 & KWH2[0]==0 & KWH2[1]==0 & KWH3[0]==0 & KWH3[1]==0 & KWH4[0]==0 & KWH4[1]==0 )
  {
      KWH1[0] = KWH1_BUFF[0]; KWH2[0] = KWH2_BUFF[0]; KWH3[0] = KWH3_BUFF[0]; KWH4[0] = KWH4_BUFF[0];
      KWH1[1] = KWH1_BUFF[1]; KWH2[1] = KWH2_BUFF[1]; KWH3[1] = KWH3_BUFF[1]; KWH4[1] = KWH4_BUFF[1];
  }
  else{
    if(LT_type==YASKAWA){      if(KWH1[1] > 0 & KWH1[0] > 0)      {
        if(abs(KWH1_BUFF[0] - KWH1[0])<=2)    {     KWH1[0]=KWH1_BUFF[0];     KWH1[1]=KWH1_BUFF[1];        }
        if(abs(KWH1_BUFF[0] - KWH1[0])>=2)    {
                   if(KWH1_BUFF[0] < KWH1[0]) {     KWH1[0]=KWH1_BUFF[0];     KWH1[1]=KWH1_BUFF[1];        }}
               }    
    }
    if(CT_type==YASKAWA){      if(KWH2[1] > 0 & KWH2[0] > 0)      {
        if(abs(KWH2_BUFF[0] - KWH2[0])<=2)    {     KWH2[0]=KWH2_BUFF[0];     KWH2[1]=KWH2_BUFF[1];        }
        if(abs(KWH2_BUFF[0] - KWH2[0])>=2)    {
                   if(KWH2_BUFF[0] < KWH2[0]) {     KWH2[0]=KWH2_BUFF[0];     KWH2[1]=KWH2_BUFF[1];        }}
               }    
    }
    if(MH_type==YASKAWA){      if(KWH3[1] > 0 & KWH3[0] > 0)      {
        if(abs(KWH3_BUFF[0] - KWH3[0])<=2)    {     KWH3[0]=KWH3_BUFF[0];     KWH3[1]=KWH3_BUFF[1];        }
        if(abs(KWH3_BUFF[0] - KWH3[0])>=2)    {     KWH3[0]=KWH3_BUFF[0];     KWH3[1]=KWH3_BUFF[1];         
//                   if(KWH3_BUFF[0] < KWH3[0]) {     KWH3[0]=KWH3_BUFF[0];     KWH3[1]=KWH3_BUFF[1];        }}
                      } 
               }    
    }
    if(AH_type==YASKAWA){      if(KWH4[1] > 0 & KWH4[0] > 0)      {
        if(abs(KWH4_BUFF[0] - KWH4[0])<=2)    {     KWH4[0]=KWH4_BUFF[0];     KWH4[1]=KWH4_BUFF[1];        }
        if(abs(KWH4_BUFF[0] - KWH4[0])>=2)    {
                   if(KWH4_BUFF[0] < KWH4[0]) {     KWH4[0]=KWH4_BUFF[0];     KWH4[1]=KWH4_BUFF[1];        }}
               }    
    }
  
}

  (LT_type==LNT_FLE)?Serial_monitor.println("\tKWH 1 : " +String(KWH1[1])+String(KWH1[0]*0.1)):Serial_monitor.print("\nKWH 1 : " +String(KWH1[1])+String(KWH1[0]));
  (CT_type==LNT_FLE)?Serial_monitor.println("\tKWH 2 : " +String(KWH2[1])+String(KWH2[0]*0.1)):Serial_monitor.print("\tKWH 2 : " +String(KWH2[1])+String(KWH2[0]));
  (MH_type==LNT_FLE)?Serial_monitor.println("\tKWH 3 : " +String(KWH3[1])+String(KWH3[0]*0.1)):Serial_monitor.print("\tKWH 3 : " +String(KWH3[1])+String(KWH3[0]));
  (AH_type==LNT_FLE)?Serial_monitor.println("\tKWH 4 : " +String(KWH4[1])+String(KWH4[0]*0.1)):Serial_monitor.println("\tKWH 4 : " +String(KWH4[1])+String(KWH4[0]));


/*
 * YASKAWA Addr  92,93
 * L&T Addr       0x032D
 * 
 * D1_Reg_addr[0] = 92; 
 * D1_Reg_addr[1] = 93; 
 * 
 * D1_Reg_Data[0] = KWH[0];
 * D1_Reg_Data[1] = KWH[0];
 * 
 *  D1_Reg_addr[0] = 92;   D1_Reg_Data[0] = KWH1[0]; 
 *  D1_Reg_addr[1] = 93;   D1_Reg_Data[1] = KWH1[1];
 * 
 */
 /*
  if(LT_type == YASKAWA){
    D1_Reg_addr[0] = 92;   D1_Reg_Data[0] = KWH1[0]; 
    D1_Reg_addr[1] = 93;   D1_Reg_Data[1] = KWH1[1];
  }
  else{
    D1_Reg_addr[0] = 92;   D1_Reg_Data[0] = KWH1[0]; 
    D1_Reg_addr[1] = 93;   D1_Reg_Data[1] = KWH1[1];    
  }
  if(LT_type == YASKAWA){
    D2_Reg_addr[0] = 92;   D2_Reg_Data[0] = KWH2[0]; 
    D2_Reg_addr[1] = 93;   D2_Reg_Data[1] = KWH2[1];
  }
  else{
    D2_Reg_addr[0] = 92;   D2_Reg_Data[0] = KWH2[0]; 
    D2_Reg_addr[1] = 93;   D2_Reg_Data[1] = KWH2[1];    
  }
  if(LT_type == YASKAWA){
    D3_Reg_addr[0] = 92;   D3_Reg_Data[0] = KWH3[0]; 
    D3_Reg_addr[1] = 93;   D3_Reg_Data[1] = KWH3[1];
  }
  else{
    D3_Reg_addr[0] = 92;   D3_Reg_Data[0] = KWH3[0]; 
    D3_Reg_addr[1] = 93;   D3_Reg_Data[1] = KWH3[1];    
  }
  if(LT_type == YASKAWA){
    D4_Reg_addr[0] = 92;   D4_Reg_Data[0] = KWH4[0]; 
    D4_Reg_addr[1] = 93;   D4_Reg_Data[1] = KWH4[1];
  }
  else{
    D4_Reg_addr[0] = 92;   D4_Reg_Data[0] = KWH4[0]; 
    D4_Reg_addr[1] = 93;   D4_Reg_Data[1] = KWH4[1];  
  }
  */ 
}

void DC_BUS_VTG(void)
{
  Serial2.begin(19200,SERIAL_8E1);  
   packet1->id = D1;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = (LT_type==LNT_FLE)?LT_DC_BUS:YK_DC_BUS;
  packet1->no_of_registers = 2;
  packet1->register_array = DCV1;
  modbus();
  packet1->id = D2;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = (CT_type==LNT_FLE)?LT_DC_BUS:YK_DC_BUS;
  packet1->no_of_registers = 2;
  packet1->register_array = DCV2;
  modbus();
  packet1->id = D3;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = (MH_type==LNT_FLE)?LT_DC_BUS:YK_DC_BUS;
  packet1->no_of_registers = 2;
  packet1->register_array = DCV3;
  modbus();
  packet1->id = D4;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = (AH_type==LNT_FLE)?LT_DC_BUS:YK_DC_BUS;//
  packet1->no_of_registers = 2;
  packet1->register_array = DCV4;
  modbus();

  Serial_monitor.print("\nDCV 1 : " +String(DCV1[0]));
  Serial_monitor.print("\tDCV 2 : " +String(DCV2[0]));
  Serial_monitor.print("\tDCV 3 : " +String(DCV3[0]));
  Serial_monitor.print("\tDCV 4 : " +String(DCV4[0]));
  Serial_monitor.println("\r\n");
  return;
  delayMicroseconds(50);  delayMicroseconds(50);
}

void operation_time_from_fault_packets(void)
{  
  Serial2.begin(19200,SERIAL_8E1);  
  packet1->id = D1;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = (LT_type==LNT_FLE)?LT_OTIME_FF:YK_O_TIME_FF;
  packet1->no_of_registers = 8;
  packet1->register_array = otf1;
modbus();
  packet1->id = D2;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = (CT_type==LNT_FLE)?LT_OTIME_FF:YK_O_TIME_FF;
  packet1->no_of_registers = 8;
  packet1->register_array = otf2;
  modbus();
  packet1->id = D3;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = (MH_type==LNT_FLE)?LT_OTIME_FF:YK_O_TIME_FF;
  packet1->no_of_registers = 8;
  packet1->register_array = otf3;
modbus();
  packet1->id = D4;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = (AH_type==LNT_FLE)?LT_OTIME_FF:YK_O_TIME_FF; //
  packet1->no_of_registers = 8;
  packet1->register_array = otf4;
modbus();

  Serial_monitor.print("OT frm Fault 1 : "+ String(otf1[0]));
  Serial_monitor.print("\tOT frm Fault 2 : "+ String(otf2[0]));
  Serial_monitor.print("\tOT frm Fault 3 : "+ String(otf3[0]));
  Serial_monitor.println("\tOT frm Fault 4 : "+ String(otf4[0]));
  
  Serial_monitor.println();
return;

  delayMicroseconds(50);  delayMicroseconds(50);
}

void operation_time_packets(void)
{
  Serial2.begin(19200,SERIAL_8E1);  
  packet1->id = D1;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = (LT_type==LNT_FLE)?LT_O_Time:YK_O_TIME;
  packet1->no_of_registers = 2;
  packet1->register_array = ot1;
  modbus();
  packet1->id = D2;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = (CT_type==LNT_FLE)?LT_O_Time:YK_O_TIME;
  packet1->no_of_registers = 2;
  packet1->register_array = ot2;
  modbus();
  packet1->id = D3;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = (MH_type==LNT_FLE)?LT_O_Time:YK_O_TIME;
  packet1->no_of_registers = 2;
  packet1->register_array = ot3;
  modbus();
  packet1->id = D4;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address =  (AH_type==LNT_FLE)?LT_O_Time:YK_O_TIME; //
  packet1->no_of_registers = 2;
  packet1->register_array = ot4;
  modbus();
  
  
  Serial_monitor.println("O time 1 : " +String(ot1[0]));
  Serial_monitor.println("\tO time 2 : " +String(ot2[0]));
  Serial_monitor.println("\tO time 3 : " +String(ot3[0]));
  Serial_monitor.println("\tO time 4 : " +String(ot4[0]));
  return;
  delayMicroseconds(50);  delayMicroseconds(50);
}

void init_modbus_packet(void)
{
  long timer_run =millis();
  Serial_monitor.println("Timer Sec : " + String(timer_run)); delayMicroseconds(50);
  Serial2.begin(19200,SERIAL_8E1); delayMicroseconds(500);
  packet1->id = D1;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address =         (LT_type==LNT_FLE) ? LT_ST_REG:YK_ST_REG;
  packet1->no_of_registers = (LT_type==LNT_FLE) ? LT_Status_Read:YK_Status_Read;
  packet1->register_array = drive1;
  modbus();

  packet1->id = D2;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address =         (CT_type==LNT_FLE) ? LT_ST_REG:YK_ST_REG;
  packet1->no_of_registers = (CT_type==LNT_FLE) ? LT_Status_Read:YK_Status_Read;
  packet1->register_array = drive2;
  modbus();
  
  packet1->id = D3;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address =         (MH_type==LNT_FLE) ? LT_ST_REG:YK_ST_REG;
  packet1->no_of_registers = (MH_type==LNT_FLE) ? LT_Status_Read:YK_Status_Read;
  packet1->register_array = drive3;
  modbus();
  
  packet1->id = D4;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address =         (AH_type==LNT_FLE) ?  LT_ST_REG:YK_ST_REG; //YK_ST_REG; //
  packet1->no_of_registers = (AH_type==LNT_FLE) ?  LT_Status_Read:YK_Status_Read;
  packet1->register_array = drive4;
  modbus(); 

//  float secs = (millis()-timer_run);
//  Serial_monitor.print("MODBUS READ : ");Serial_monitor.println(secs/1000,3);
//  Serial_monitor.println("Timer Sec : " + String(millis()));
Serial_monitor.println('\t');
  // AC_VTG = regs1[5]  AC_CUR = regs1[6]
  
  for(int i=0; i<YK_Status_Read;i++){
    if(i==5 | i==6){
    if(LT_type==YASKAWA){  if(drive1[i]!=0 & drive1[i]<10000){regs1[i] = drive1[i];}  Serial_monitor.print(regs1[i]);Serial_monitor.print('\t');  }
    if(CT_type==YASKAWA){  if(drive2[i]!=0 & drive2[i]<10000){regs2[i] = drive2[i];}  Serial_monitor.print(regs2[i]);Serial_monitor.print('\t');  }
    if(MH_type==YASKAWA){  if(drive3[i]!=0 & drive3[i]<10000){regs3[i] = drive3[i];}  Serial_monitor.print(regs3[i]);Serial_monitor.print('\t');  }
    if(AH_type==YASKAWA){  if(drive4[i]!=0 & drive4[i]<10000){regs4[i] = drive4[i];}  Serial_monitor.print(regs4[i]);Serial_monitor.println('\t');}
    }
    else{
      regs1[i] = drive1[i];
      regs2[i] = drive2[i];
      regs3[i] = drive3[i];
      regs4[i] = drive4[i];
    }
  }
  
//  Serial_monitor.println("FUNCTION RUN : ");
//  secs = (millis()-timer_run);  Serial_monitor.println(secs/1000,3);
//  Serial_monitor.println("Timer Sec : " + String(millis()));
}

void Fault_packet_set (int Fault)// set packet at Fault_1 & Fault_2
{
  if(!Fault)  {
    Fault = Fault_Reg;
  }
  Serial2.begin(19200,SERIAL_8E1);
  packet1->id = D1;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = (LT_type==LNT_FLE)?LT_FAULT_R:Fault_Reg;
  packet1->no_of_registers = Fault_Read;
  packet1->register_array = regs5;
  modbus();
  
  packet1->id = D2;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = (CT_type==LNT_FLE)?LT_FAULT_R:Fault_Reg;
  packet1->no_of_registers = Fault_Read;
  packet1->register_array = regs6;
  modbus();
  
  packet1->id = D3;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = (MH_type==LNT_FLE)?LT_FAULT_R:Fault_Reg;
  packet1->no_of_registers = Fault_Read;
  packet1->register_array = regs7;
  modbus();
  packet1->id = D4;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = (AH_type==LNT_FLE)?LT_FAULT_R:Fault_Reg;
  packet1->no_of_registers = Fault_Read;
  packet1->register_array = regs8;
  modbus();
}

void Fan_operation_time_packets(void)
{
  Serial2.begin(19200,SERIAL_8E1);
  packet1->id = D1;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = (LT_type==LNT_FLE)?0x0344:154;
  packet1->no_of_registers = 2;
  packet1->register_array = fot1;
modbus();
  packet1->id = D2;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = (CT_type==LNT_FLE)?0x0344:154;
  packet1->no_of_registers = 2;
  packet1->register_array = fot2;
modbus();
  packet1->id = D3;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = (MH_type==LNT_FLE)?0x0344:154;
  packet1->no_of_registers = 2;
  packet1->register_array = fot3;
modbus();
  packet1->id = D4;
  packet1->function = READ_HOLDING_REGISTERS;
  packet1->address = (AH_type==LNT_FLE)?0x0344:154;
  packet1->no_of_registers = 2;
  packet1->register_array = fot4;
modbus();

  Serial_monitor.println("FanO time 1 : " +String(fot1[1]) +String(fot1[0]));
  Serial_monitor.println("FanO time 2 : " +String(fot2[1]) +String(fot2[0]));
  Serial_monitor.println("FanO time 3 : " +String(fot3[1]) +String(fot3[0]));
  Serial_monitor.println("FanO time 4 : " +String(fot4[1]) +String(fot4[0]));
return;
  delayMicroseconds(50);    delayMicroseconds(50);
}

int find_faults(int regC0, int regC1)
{
  int Fault=0;
         if(regC0 & 0B0000000000000010){    Fault=1;  }
    else if(regC0 & 0B0000000000000100){    Fault=2;  }
    else if(regC0 & 0B0000000000100000){    Fault=3;  }
    else if(regC0 & 0B0000000001000000){    Fault=4;  }
    else if(regC0 & 0B0000000010000000){    Fault=5;  }
    else if(regC0 & 0B0000000100000000){    Fault=6;  }
    else if(regC0 & 0B0000001000000000){    Fault=7;  }
    else if(regC0 & 0B0000010000000000){    Fault=8;  }
    else if(regC0 & 0B0000100000000000){    Fault=9;  }
    else if(regC0 & 0B0001000000000000){    Fault=10; }
    else if(regC0 & 0B0010000000000000){    Fault=11; }
    else if(regC0 & 0B0100000000000000){    Fault=12; }
    else if(regC0 & 0B1000000000000000){    Fault=13; }
    else if(regC1 & 0B0000000001000000){    Fault=14; }
    else if(regC1 & 0B0000000010000000){    Fault=15; }
    else if(regC1 & 0B0000000100000000){    Fault=16; }
    else if(regC1 & 0B0000001000000000){    Fault=17; }
    else if(regC1 & 0B0000010000000000){    Fault=18; }
    else if(regC1 & 0B0000100000000000){    Fault=19; }
    else if(regC1 & 0B0001000000000000){    Fault=20; }
    Serial_monitor.print(String(packet1->id)+ "\tfault" + Fault);
    delayMicroseconds(50);
    return Fault;
}

int status_faults(void)
{
      Fault_packet_set(Fault_1);
//      check_response_fault(Fault_1);
      vTaskDelay(10);
      for(int k=0;k<Modbus_update_loop;k++){
        connection_status = modbus_update(packets);
        vTaskDelay(50);
      }
      LT_Fault = find_faults(regs5[0],regs5[1]);
      CT_Fault = find_faults(regs6[0],regs6[1]);
      MH_Fault = find_faults(regs7[0],regs7[1]);
      AH_Fault = find_faults(regs8[0],regs8[1]);
}

int check_status(int Drive_id)
{
  Serial_monitor.println("FUNC: CHECK STATUS");
  int During_run=0,zero_speed=0,reverse_run=0,fault_reset=0,DataS_err=0,drive_ready=0,Alarm=0,fault=0,operation_error=0,powerloss=0;
  int Status_reg = 0;
  int Drive_Status_Type=0;
  switch(Drive_id){
    case 1: Status_reg = regs1[0];     break;
    case 2: Status_reg = regs2[0];     break;
    case 3: Status_reg = regs3[0];     break;
    case 4: Status_reg = regs4[0];     break;
    default:          Serial_monitor.println("\t\tERR: No Drive id found");
  }
  
  Drive_Status_Type = FindDriveType(Drive_id);
  if(Drive_Status_Type == YASKAWA)
  {
      if((Status_reg& 0B0000001111111111))//|(regs5[1] & 0B0001111111000000))
      {  Serial_monitor.print("YASKAWA Drive :");
         if(Status_reg & 0B0000000000000001)  {    During_run=1;    Serial_monitor.print("Drive Run\t");    }
         if(Status_reg & 0B0000000000000010)  {    reverse_run=1;   Serial_monitor.print("Reverse Run\t");  }
         if(Status_reg & 0B0000000000000100)  {    drive_ready=1;   Serial_monitor.print("Drive Ready\t");  }
         if(Status_reg & 0B0000000000001000)  {    fault=1;         Serial_monitor.print("Fault\t");        }
         if(Status_reg & 0B0000000000010000)  {    DataS_err=1;     Serial_monitor.print("Data Set Err\t"); } 
         Serial_monitor.println();
         delayMicroseconds(50);
          if(fault==1 ){
//            Fault_Monitor();
            return 1;
          }
        else               return 0;        
      }
  }
  
  if(Drive_Status_Type == LNT_FLE)
  {
     if((Status_reg& 0B0001001111111111))//|(regs5[1] & 0B0001111111000000))
      {  Serial_monitor.print("LnT Drive :");
         if(Status_reg & 0B0000000000000001)    {    During_run=1;    Serial_monitor.print("Drive Run\t");     }
         if(Status_reg & 0B0000000000000010)    {    reverse_run=1;   Serial_monitor.print("Reverse Run\t");   }
         if(!(Status_reg & 0B0001000000000000)) {    drive_ready=1;   Serial_monitor.print("Drive Ready\t");   }
         if(Status_reg & 0B1000000000000000)    {    fault=1;         Serial_monitor.print("Fault\t");         }
         if(Status_reg & 0B0100000000010000)    {    DataS_err=1;     Serial_monitor.print("Warning Status\t");}
         Serial_monitor.println();
         delayMicroseconds(50);
          if(fault==1 )     return 1;
        else                return 0;
      }
  } 
}

void modbus_configuration(void){
  modbus_configure(19200, modbus_timeout, polling, retry_count, TxEnablePin, packets, TOTAL_NO_OF_PACKETS);       
}

int Modbus_Update(void){
  connection_status = modbus_update(packets);
}

void Modbus_Data_Com(void){
  //    init_modbus_packet();
  vTaskDelay(10);
  for(int k=0;k<Modbus_update_loop;k++)
  {
    connection_status = modbus_update(packets);
    delay(50);
  } 
  Serial_monitor.println("Status: ");
  for(int j=0;j<16;j++){
    Serial_monitor.print(regs1[j]);Serial_monitor.print('\t');
    Serial_monitor.print(regs2[j]);Serial_monitor.print('\t');
    Serial_monitor.print(regs3[j]);Serial_monitor.print('\t');
    Serial_monitor.print(regs4[j]);Serial_monitor.println();  
  }
  Serial_monitor.println();  
//  data_str_send();
}

void data_str_send(void){ 
      
    int LT_st_flag = check_status(D1);//(regs1[0]);
    int CT_st_flag = check_status(D2);//(regs2[0]);
    int MH_st_flag = check_status(D3);//(regs3[0]);
    int AH_st_flag = check_status(D4);//(regs4[0]);

    int Drive_status; 
    int Drive_output_vtg;
    int Drive_output_crnt;  //  int Drive_output_power= regs1[7];
    int id;
    int driver_id;                      //  int DC_vtg = DCV1[0];
    int run_;    
    unsigned int Power;
    String fan_; 
    int Heat_Sink; 

    Drive_status = regs1[0]; 
    Drive_output_vtg  = regs1[5]*(0.1);
    Drive_output_crnt = regs1[6]*(0.1);  //  Drive_output_power= regs1[7];
    id = Device_id;
    driver_id = D1;                      //  DC_vtg = DCV1[0];
    run_ = digitalRead(LT);    
    Power = (KWH1[1]*10000)+KWH1[0];
    fan_ =(String(fot4[1])+String(fot4[0])); 
    Heat_Sink = fan_.toInt(); 
    
    Drive_1_status = "";
    Drive_1_status  = Prepare_Data_String(id, crane_id, driver_id, run_, Drive_output_vtg, Drive_output_crnt, Power, Heat_Sink, LT_st_flag, Drive_status);
      
    Drive_status = regs2[0]; 
    Drive_output_vtg  = regs2[5]*(0.1);
    Drive_output_crnt = regs2[6]*(0.1);
  //  Drive_output_power= regs2[7];
    driver_id = D2;
  //  DC_vtg = DCV2[0];
    run_ = digitalRead(CT);
    Power = (KWH2[1]*10000)+KWH2[0];
    fan_ =(String(fot2[1])+String(fot2[0])); 
    Heat_Sink = fan_.toInt();   
    
    Drive_2_status = "";
    Drive_2_status  = Prepare_Data_String(id, crane_id, driver_id, run_, Drive_output_vtg, Drive_output_crnt, Power, Heat_Sink, CT_st_flag, Drive_status ); 
   
    Drive_status = regs3[0]; 
    Drive_output_vtg  = regs3[5]*(0.1);
    Drive_output_crnt = regs3[6]*(0.1);
  //  Drive_output_power= regs3[7]; 
    driver_id = D3;
  //  DC_vtg = DCV3[0];
    run_ = digitalRead(MH);
    Power = (KWH3[1]*10000)+KWH3[0];
    fan_ =(String(fot3[1])+String(fot3[0])); 
    Heat_Sink = fan_.toInt(); 
    
    Drive_3_status = "";
    Drive_3_status  = Prepare_Data_String(id, crane_id, driver_id, run_, Drive_output_vtg, Drive_output_crnt, Power, Heat_Sink, MH_st_flag, Drive_status ); 
   
    Drive_status = regs4[0]; 
    Drive_output_vtg  = regs4[15];    //  Drive_output_vtg  = regs3[5]*(0.1);
    Drive_output_crnt = regs4[11];    //  Drive_output_crnt = regs3[6]*(0.1);
  //  Drive_output_power= regs4[7];
    driver_id = D4;
  //  DC_vtg = DCV4[0];
    run_ = digitalRead(AH);
    Power = KWH4[0]*0.1;              //(KWH4[1]*10000)+KWH4[0];
    fan_ =(String(fot4[1])+String(fot4[0])); 
    Heat_Sink = fan_.toInt();
    Drive_4_status = "";
    Drive_4_status  = Prepare_Data_String(id, crane_id, driver_id,  run_, Drive_output_vtg, Drive_output_crnt, Power, Heat_Sink, AH_st_flag, Drive_status); 

  if(LT_st_flag | CT_st_flag | MH_st_flag | AH_st_flag ){
    if(LT_st_flag){fault_drive_LT = 1;}   else{fault_drive_LT = 0;}
    if(CT_st_flag){fault_drive_CT = 1;}   else{fault_drive_CT = 0;}
    if(MH_st_flag){fault_drive_MH = 1;}   else{fault_drive_MH = 0;}
    if(AH_st_flag){fault_drive_AH = 1;}   else{fault_drive_AH = 0;} 
    Response_fault_2 = 1;     //status_faults();      //init_modbus_packet();
  }
  else  
  {
    Response_fault_2 = 0;
  } 
}

String Prepare_Data_String(int id,  char crane_id, int driver_id, int run_, int Drive_output_vtg, int Drive_output_crnt, int Power, int Heat_Sink, int _Fault, int Drive_status )
{
  DateTime now = rtc.now();
  String time_str = "";
  String Zero = "0";
  time_str.concat(String(now.year()));    
  time_str.concat(F("-"));                          
  if(now.month()<10)time_str.concat(Zero);           time_str.concat(String(now.month()));
  time_str.concat(F("-"));                          
  if(now.day()<10)time_str.concat(Zero);             time_str.concat(String(now.day()));  
  time_str.concat(F(" "));                        
  if(now.hour()<10)time_str.concat(Zero);            time_str.concat(String(now.hour())); 
  time_str.concat(F(":"));
  if(now.minute()<10)time_str.concat(Zero);          time_str.concat(String(now.minute()));
  time_str.concat(F(":"));                          
  if(now.second()<10)time_str.concat(Zero);          time_str.concat(String(now.second()));  
//  Serial_monitor.println(time_str);

 /* DynamicJsonDocument  doc(400);
  int operation_time;
  int Drive_Output_freq;
  int DC_vtg;

       if(driver_id==1){operation_time = ot1[0]; Drive_Output_freq = regs1[3]; DC_vtg = DCV1[0];}
  else if(driver_id==2){operation_time = ot2[0]; Drive_Output_freq = regs2[3]; DC_vtg = DCV2[0];}
  else if(driver_id==3){operation_time = ot3[0]; Drive_Output_freq = regs3[3]; DC_vtg = DCV3[0];}
  else if(driver_id==4){operation_time = ot4[0]*24; Drive_Output_freq = regs4[12]; DC_vtg = DCV4[0];}
  //  else if(driver_id==4){operation_time = ot4[0]; Drive_Output_freq = regs4[3]; DC_vtg = DCV4[0];}
  
  doc["id"]               =     id;
  doc["crane_id"]         =     SENSOR_ID;
  doc["driver_id"]        =     ((SENSOR_ID-1)*4)+driver_id;
  if(driver_id == D4){
    doc["drive_ready"]    =     (((Drive_status & 0xF000)|0xF000)==0x0000)?true:false;
  }
  else{
    doc["drive_ready"]    =     (Drive_status & 0B0000000000000100)?true:false;
  }
  doc["on_off"]           =     !((bool)run_);
  doc["output_freq"]      =     Drive_Output_freq;  //random(1000,9000);//
  doc["ac_voltage"]       =     Drive_output_vtg;   //random(0,220);//
  doc["output_current"]   =     Drive_output_crnt;  //random(0,100);//
  doc["dc_voltage"]       =     DC_vtg;             //random(0,440);//
  doc["drive_power"]      =     Power;              //random(400,460);//
  doc["operation_time"]   =     operation_time;     //random(400,460);//
//  doc["cum_power"]        =     Power;            //random(400,460);// change remaining 
  doc["heat_sink"]        =     (bool)Heat_Sink;    //random(0,1);//
  doc["reading_time"]     =     time_str;           //  doc["system_time"]      =     time_str;
  doc["fault"]            =     (bool)_Fault;
  doc["driverReading"]    =     Drive_status;
  
  String p_str="";
  serializeJson(doc, p_str);*/
  String J_str = Reg_Data_string_json(!((bool)run_),driver_id,time_str);
  vTaskDelay(50);
  
//  return p_str;
  return J_str;
}  

String Reg_Data_string_json(bool drun,int Drive_No,String time_str){
  DynamicJsonDocument  Mreg(1500);
  int data_array[22];
  int DCV ;
  int KWH_1;
  int KWH_2;
  int OT_1;
  int FOT_1;
  int FOT_2;
  int DriveType = FindDriveType(Drive_No);
  int reg_addr =0;
  int reg_No =  0;
  int fault_1 = 0;
  int fault_2 = 0;
  int OPTIME = 0;
  switch (Drive_No){
    case D1:  (LT_type==LNT_FLE) ?  reg_addr=LT_ST_REG  :   reg_addr=YK_ST_REG;
              (LT_type==LNT_FLE) ?  reg_No=8            :   reg_No=16;
              DCV  = DCV1[0];
              KWH_1 = KWH1[0];
              KWH_2 = KWH1[1];
              OT_1  = ot1[0];
              FOT_1 = fot1[0];
              FOT_2 = fot1[1];
              fault_1 = D1_Faults[0];
              fault_2 = D1_Faults[1];
              OPTIME = DSTATUS.lt_time;
              break;
    case D2:  (CT_type==LNT_FLE) ?  reg_addr=LT_ST_REG  :   reg_addr=YK_ST_REG;
              (CT_type==LNT_FLE) ?  reg_No=8            :   reg_No=16;
              DCV  = DCV2[0]; 
              KWH_1 = KWH2[0];
              KWH_2 = KWH2[1];
              OT_1  = ot2[0]; 
              FOT_1 = fot2[0];
              FOT_2 = fot2[1];
              fault_1 = D2_Faults[0];
              fault_2 = D2_Faults[1];
              OPTIME = DSTATUS.ct_time;
              break;
    case D3:  (MH_type==LNT_FLE) ?  reg_addr=LT_ST_REG  :   reg_addr=YK_ST_REG;
              (MH_type==LNT_FLE) ?  reg_No=8            :   reg_No=16;
              DCV  = DCV3[0]; 
              KWH_1 = KWH3[0];
              KWH_2 = KWH3[1];
              OT_1  = ot3[0]; 
              FOT_1 = fot3[0];
              FOT_2 = fot3[1];
              fault_1 = D3_Faults[0];
              fault_2 = D3_Faults[1];
              OPTIME = DSTATUS.mh_time;
              break;
    case D4:  (AH_type==LNT_FLE) ?  reg_addr=LT_ST_REG  :   reg_addr=YK_ST_REG;
              (AH_type==LNT_FLE) ?  reg_No=8            :   reg_No=16;
              DCV  = DCV4[0];
              KWH_1 = KWH4[0];
              KWH_2 = KWH4[1];
              OT_1  = ot4[0];
              FOT_1 = fot4[0];
              FOT_2 = fot4[1];
              fault_1 = D4_Faults[0];
              fault_2 = D4_Faults[1];
              OPTIME = DSTATUS.ah_time;
              break;
    default:
            Serial_monitor.println("Unknown Drive ID :(");
            break; 
  }
  
  Mreg["id"]               =     Device_id;
  Mreg["on_off"]           =     drun;
  Mreg["crane_id"]         =     SENSOR_ID;
  Mreg["driver_id"]        =     ((SENSOR_ID-1)*4)+Drive_No;
  Mreg["Drive_Type"]       =     (DriveType==LNT_FLE)?"LnT_FLEXI":"YASKAWA";
  Mreg["operation_time"]   =     ((float)OPTIME/3600);
  Mreg["cumm_time"]        =     ((float)crane_timer_buff/3600);
  
    for(int i = reg_addr; i < reg_addr+reg_No ; i++)
    {
           if(Drive_No==D1)    Mreg[String("R_")+String(i,HEX)] = regs1[i-reg_addr];
      else if(Drive_No==D2)    Mreg[String("R_")+String(i,HEX)] = regs2[i-reg_addr];
      else if(Drive_No==D3)    Mreg[String("R_")+String(i,HEX)] = regs3[i-reg_addr];
      else if(Drive_No==D4)    Mreg[String("R_")+String(i,HEX)] = regs4[i-reg_addr];
    }
    // A3 : KWH 1 : 95036  KWH 2 : 14012 KWH 3 : 28468 KWH 4 : 0633.10
      
  if (DriveType==YASKAWA) {
    Mreg[String("R_")+String(32,HEX)]              =   0;
    Mreg[String("R_")+String(YK_DC_BUS,HEX)]       =   DCV;
    Mreg[String("R_")+String(YK_PO_KWH,HEX)]       =   KWH_1; //24741000
    Mreg[String("R_")+String((YK_PO_KWH+1),HEX)]   =   KWH_2;
    Mreg[String("R_")+String(YK_O_TIME,HEX)]       =   OT_1; 
    Mreg[String("R_")+String(YK_FAN_OP,HEX)]       =   FOT_1;
    Mreg[String("R_")+String((YK_FAN_OP+1),HEX)]   =   FOT_2;
    Mreg[String("R_")+String((YKW_Fault_Reg),HEX)]   =   fault_1;
    Mreg[String("R_")+String((YKW_Fault_Reg+1),HEX)] =  fault_2;
  }
  else
  {
    Mreg[String("R_")+String(LT_DC_BUS,HEX)]       =   DCV;
    Mreg[String("R_")+String(LT_PO_KWH,HEX)]       =   KWH_1;
    Mreg[String("R_")+String((LT_PO_KWH+1),HEX)]   =   KWH_2;
    Mreg[String("R_")+String(LT_O_Time,HEX)]       =   OT_1; 
    Mreg[String("R_")+String(LT_FAN_OP,HEX)]       =   FOT_1;
    Mreg[String("R_")+String((LT_FAN_OP+1),HEX)]   =   FOT_2;
    Mreg[String("R_")+String((LnT_Fault_Reg),HEX)] =   fault_1;
    Mreg[String("R_")+String((LnT_Fault_Reg+1),HEX)] =  fault_2;
  }
  
  Mreg["reading_time"]        =  NTP_time();  //  time_str;  //    "2022-06-13%2014:13:11";//
  String Data_Json_String;
//  serializeJsonPretty(Mreg, Data_Json_String);
  serializeJson(Mreg, Data_Json_String);
//  Serial_monitor.println(Data_Json_String);
  return Data_Json_String;
}

// Fault Read and Write functions

unsigned int FindDriveType(unsigned int Drive_No){
  switch (Drive_No) {
    case 1:
          if (LT_type == YASKAWA)           return YASKAWA;           
          else if (LT_type == LNT_FLE)      return LNT_FLE;
          break;
    case 2:
          if (CT_type == YASKAWA)           return YASKAWA;  
          else if (CT_type == LNT_FLE)      return LNT_FLE;
          break;
    case 3:
          if (MH_type == YASKAWA)           return YASKAWA;
          else if (MH_type == LNT_FLE)      return LNT_FLE;
          break;
    case 4:
          if (AH_type == YASKAWA)           return YASKAWA;
          else if (AH_type == LNT_FLE)      return LNT_FLE;
          break;
    default:
      Serial_monitor.println(" Reset Drive not identified");
    }
}

void set_packet_parameter(struct packet_write AC_DRIVE)//(int id, int function, int addr, int reg_no, unsigned int regs[]){
{
  packet1->id = AC_DRIVE.id;
  packet1->function = AC_DRIVE.function;
  packet1->address = AC_DRIVE.addr;
  packet1->no_of_registers = AC_DRIVE.Num_Of_Regs;
  packet1->register_array = AC_DRIVE.Array;
}

void Reset_from_faults(unsigned int Drive_id) {
  Serial_monitor.println("Func: Drive Reset Write\r\n");
  unsigned int regs[9];
  RESET_PLC YASKAWA_reset_params  = {Drive_id, PRESET_MULTIPLE_REGISTERS,  YASKAWA_RESET_FLAG_ADDR,  RESET_FLAG_NO_REGS,   regs[6]};
  RESET_PLC LnT_reset_params      = {Drive_id, PRESET_MULTIPLE_REGISTERS,   LnT_RESET_FLAG_ADDR,       RESET_FLAG_NO_REGS,   regs[6]};

  unsigned int DriveType = FindDriveType(Drive_id);

  if(DriveType == YASKAWA)       {YASKAWA_reset_params.id = Drive_id;         Drive = YASKAWA_reset_params;}
  else if(DriveType == LNT_FLE)  {LnT_reset_params.id = Drive_id;             Drive = LnT_reset_params;}
  else {Serial_monitor.println("ERR : Drive Type Not found!!");}
  
  Drive.Array[0] = Count;// RESET_CMD;   
  set_packet_parameter(Drive);

  for (int i = 0; i < RESET_FLAG_NO_REGS; i++) {
    Serial_monitor.print(Drive.Array[i]);  Serial_monitor.print('\t');
  } 
  modbus();
  vTaskDelay(200);
}


void Read_fault(int driver) { 
  unsigned int fault_addr;  
  unsigned int WRITE_MULTI_REGS = 16;
  unsigned int READ_HOLDING_REGS = 3;
  unsigned int Fault_REGS = 0;
  unsigned int DriveType = FindDriveType(driver);

       if(DriveType == YASKAWA)  {fault_addr = YKW_Fault_Reg;  Fault_REGS = YKW_Fault_Reset_Reg;}
  else if(DriveType == LNT_FLE)  {fault_addr = LnT_Fault_Reg;  Fault_REGS = LnT_Fault_Reset_Reg;}
  else {Serial_monitor.println("ERR : Drive Type Not found!!");}
      
    unsigned int reset_reg_buff[2]={0,0};
    packet1->id = driver;
    packet1->function = READ_HOLDING_REGS;
    packet1->address = fault_addr;
    packet1->no_of_registers = RESET_FLAG_NO_REGS;
    packet1->register_array = R_regs;
    vTaskDelay(50);
    modbus();

  Serial_monitor.println("Fault Register Data : ");
  for (int i = 0; i < RESET_FLAG_NO_REGS; i++) {
    Serial_monitor.print(R_regs[i]);  Serial_monitor.print('\t');
  } 
  if (R_regs[0]>=1){                         //(millis() - timeout2 > 5000) {
    Serial_monitor.println(F("Fault Found"));
  }
  Serial_monitor.println();
}

int Reset_Drive (int D_NO ){
    unsigned int Reset_reg_data[8]={0,0,0,0,0,0,0,0};

//    packet1->id = D_NO;
//    packet1->function = 4;
//    packet1->address = 0x0008;
//    packet1->no_of_registers = 8;
//    packet1->register_array = Reset_reg_data;
//    delay(50);
//    modbus();
//  
//    Serial_monitor.println("Before fault Register Data : ");  
//    for(int i=0;i<8;i++){Serial_monitor.print (6 + i);Serial_monitor.print('\t');Serial_monitor.print(Reset_reg_data[i]); Serial_monitor.print('\t');Serial_monitor.println(Reset_reg_data[0],BIN);}

    unsigned int RESET_DATA[1] = {8};
    
    packet1->id = D_NO;
    packet1->function = PRESET_MULTIPLE_REGISTERS;//READ_HOLDING_REGS;
    packet1->address = (FindDriveType(D_NO)==YASKAWA) ?  YK_F_RESET:LT_F_RESET; /* ;*/// 0x0005; 
    packet1->no_of_registers =1;
    packet1->register_array = RESET_DATA;
    delay(50);
    modbus(); 
    Serial_monitor.println("Drive Reset done Reading register Data\r\n");

    return 1;
//    packet1->id = D_NO;
//    packet1->function = 4;
//    packet1->address = 0x0008;
//    packet1->no_of_registers = 8;
//    packet1->register_array = Reset_reg_data;
//    delay(50);
//    modbus();
//  
//    Serial_monitor.println("Afetr Reset Register Data : "); for(int i=0;i<8;i++){Serial_monitor.print(6 + i);Serial_monitor.print('\t');Serial_monitor.print (Reset_reg_data[i]); Serial_monitor.print('\t');Serial_monitor.println(Reset_reg_data[0],BIN);} 
}

//  prepare all strings here
//  http://localhost:50903/Boiler_CMS/Home/SaveDriverReading?driverReading={"id":0,"crane_id":1,"driver_id":1,"on_off":true,"output_freq":10.31,"ac_voltage":10.2,"output_current":1.5,"dc_voltage":5.5,"heat_sink":true,"reading_time":null,"system_time":null,"fault":false,"driverReading":0}
//  http://192.168.1.100/Boiler_CMS/Home/SaveDriverReading?driverReading={"id":0,"crane_id":1,"driver_id":2,"on_off":1,"output_freq":3000,"ac_voltage":204,"output_current":0,"dc_voltage":0,"heat_sink":0,"reading_time":"2020-10-5%2018:56:45","system_time":"2020-10-5%2018:56:45","fault":0,"driverReading":37}
