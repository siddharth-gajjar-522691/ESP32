
//String ReadString = "";
//String file_pwd = "";
//String file_ssid = "";
//String IP_="";
//String PORT_str="";
//String PATH = "";


#include <SPI.h> 
#include <SD.h>

#define Data_File  "/data_log.txt"
#define Ptr_File   "/prg_ptr.txt"

void SD_init();
int Save_to_SD(String Data_str);
int Read_SD_to_send(void);
void printDirectory(File dir, int numTabs);

int WriteFile (String data_str, const char * path);
String readFile( const char * path);


 
void printDirectory(File dir, int numTabs){
  // Begin at the start of the directory
  dir.rewindDirectory();

  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial_monitor.print('\t');   // we'll have a nice indentation
    }
    // Print the 8.3 name
    Serial_monitor.print(entry.name());
//    SD.remove(entry.name());
    // Recurse for directories, otherwise print the file size
    if (entry.isDirectory()) {
      Serial_monitor.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial_monitor.print("\t\t");
      Serial_monitor.println(entry.size(), DEC);
    }
    entry.close();
  }
}

void SD_init(void){//if (!SD.begin(15, 27, 26, 14))

  if (!SD.begin(5))//if(!SD.begin(SD_CS, SD_MISO, SD_MOSI, SD_SCK))      // 
  {
    Serial_monitor.println(F("SD CARD FAILED!"));
  }
  else{
    Serial_monitor.println(F("SD CARD OK."));
  }
  
  File myFile = SD.open("/");
  printDirectory(myFile, 0);
  myFile = SD.open(Data_File);

#ifdef PRINT_FILE_DATA
  if (myFile) {
    Serial_monitor.println(Data_File); 
    
    while (myFile.available()) {
      Serial_monitor.write(myFile.read());
      if(Serial_command()==0) break;
    }
    myFile.close();
  }
  else {   
    Serial_monitor.println("error opening");
  }
#endif
}

int Save_to_SD(String Data_str){  
  if (SD.exists(Data_File)){
    Serial_monitor.println(F(" File exists -> Save SD Card Loop"));
    Serial_monitor.println(("Appending to file to :")+ String(Data_File));
    File file = SD.open(Data_File, FILE_APPEND);// "FILE_APPEND" FOR SD.H
    if (!file)
      Serial_monitor.println(F("Failed to open file for append."));
    Data_str.concat("@\r\n");
    if (file.print(Data_str))
      {Serial_monitor.println(F("Data append success"));}
    else
      Serial_monitor.println(F("Data append failed"));

    Serial_monitor.print("File Size: "); Serial_monitor.println(file.size(),DEC);
    Serial_monitor.println();
    file.close(); 
  }
  else
  {
    Serial_monitor.println(F("file not exists -> Writing to file:"));
    File file = SD.open(Data_File, FILE_WRITE);
    if (file){
      Data_str.concat("@\r\n");
      if (file.print(Data_str))
      {  Serial_monitor.println(F("Data write success.")); }        
      else
        Serial_monitor.println(F("Data write failed"));
    }
    else
      Serial_monitor.println(F("Failed to open file for write."));
    Serial_monitor.println("File Size: " + String(file.size()));
    file.close();     
  }
}

String readFile( const char * path){
   Serial_monitor.printf("Reading file: %s\r\n", path);
    File file = SD.open(path);
    if(!file || file.isDirectory()){
        Serial_monitor.println("- failed to open file for reading");
        return "Failed to open file";
    }
    Serial_monitor.println("- read from file:");
    String ReadString="";
    while(file.available()){
      char ch = file.read();
      ReadString.concat(ch);
    }
    file.close();
    Serial_monitor.print("\t"); Serial_monitor.print(ReadString);
    return ReadString;
}

int WriteFile (String Data_str, const char * path){
  
  SD.remove(path); 
  Serial_monitor.printf("Storing Number : %s\r\n",path);
//  Serial_monitor.println("DATA : "+Data_str); 
  
    Serial_monitor.println(F("file not exists -> Writing to file:"));
    File file = SD.open(path, FILE_WRITE);
    if (file){
      Data_str.concat("\r\n");
      if (file.print(Data_str))
      {  
        Serial_monitor.println(F("Data write success.")); 
      }        
      else
      {
        Serial_monitor.println(F("Data write failed"));
        return 0;
      }
    }
    else
      Serial_monitor.println(F("Failed to open file for write."));
    Serial_monitor.println("File Size: " + String(file.size()));
    file.close();
    return 1;  
}
