
/*
 * File system used here to read and write the configuration of drives and crane
 * 
 */
 #define SENSOR_ID 2  //"L&T_Crane_A1";

 /*
 * ADD wifi and data communication related functions
 * New Update V 1_10
 * A3 : 10.7.139.120
 * B3 : 10.7.139.109
 * 
 * B4 : 10.7.139.124   10.7.139.112
 * A4 : 10.7.139.140
 */
 /* Crane  SENSOR_ID
  * A3 =    1          DRIVER ID  1  TO 4
  * A4 =    2          DRIVER ID  5  TO 8
  * B3 =    3          DRIVER ID  9  TO 12
  * B4 =    4          DRIVER ID  13 TO 16
  */
 
#define MAC1 = "24:62:AB:FD:1B:80"; //A4 - BC4
#define MAC2 = "FC:F5:C4:01:4F:1C"; //B4 - BC2
#define MAC3 = "24:62:AB:FD:37:7C"; //B3 - BC3  SENSOR_ID = 2 DRIVER ID : 5,6,7,8
#define MAC4 = "F0:08:D1:D8:41:D4"; //A3 - installed
#define MAC5 = "FC:F5:C4:01:50:70"; //A1

#define YASKAWA 1
#define LNT_FLE 2

unsigned int LT_type = YASKAWA ;
unsigned int CT_type = YASKAWA ;//LNT_FLE;
unsigned int MH_type = YASKAWA ;
unsigned int AH_type = YASKAWA ; // LNT_FLE ;//

#define Device_id 0

#define D1  1
#define D2  2
#define D3  3
#define D4  4


unsigned int buffer_reg1[16];
unsigned int buffer_reg2[16];
unsigned int buffer_reg3[16];
unsigned int buffer_reg4[16];


unsigned int regs1[16];
unsigned int regs2[16];
unsigned int regs3[16];
unsigned int regs4[16];
unsigned int regs5[16];
unsigned int regs6[16];
unsigned int regs7[16];
unsigned int regs8[16];


unsigned int LT_Fault;
unsigned int CT_Fault;
unsigned int MH_Fault;
unsigned int AH_Fault;

void Task_Configuration(void);

void Task_Configuration(void){
  Serial_monitor.print("Drive LT : ");  Serial_monitor.println((LT_type==YASKAWA)?"YASKAWA":"L&T Flexi");
  Serial_monitor.print("Drive CT : ");  Serial_monitor.println((CT_type==YASKAWA)?"YASKAWA":"L&T Flexi");
  Serial_monitor.print("Drive MH : ");  Serial_monitor.println((MH_type==YASKAWA)?"YASKAWA":"L&T Flexi");
  Serial_monitor.print("Drive AH : ");  Serial_monitor.println((AH_type==YASKAWA)?"YASKAWA":"L&T Flexi");
}
