
#include "FS.h"
#include "SPIFFS.h"

/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */
#define FORMAT_SPIFFS_IF_FAILED true

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial_monitor.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial_monitor.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial_monitor.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial_monitor.print("  DIR : ");
            Serial_monitor.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial_monitor.print("  FILE: ");
            Serial_monitor.print(file.name());
            Serial_monitor.print("\tSIZE: ");
            Serial_monitor.println(file.size());
        }
        file = root.openNextFile();
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial_monitor.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial_monitor.println("- failed to open file for reading");
        return;
    }

    Serial_monitor.println("- read from file:");
    while(file.available()){
        Serial_monitor.write(file.read());
    }
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial_monitor.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial_monitor.println("- failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial_monitor.println("- file written");
    } else {
        Serial_monitor.println("- frite failed");
    }
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial_monitor.printf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial_monitor.println("- failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial_monitor.println("- message appended");
    } else {
        Serial_monitor.println("- append failed");
    }
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial_monitor.printf("Renaming file %s to %s\r\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial_monitor.println("- file renamed");
    } else {
        Serial_monitor.println("- rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    Serial_monitor.printf("Deleting file: %s\r\n", path);
    if(fs.remove(path)){
        Serial_monitor.println("- file deleted");
    } else {
        Serial_monitor.println("- delete failed");
    }
}

void testFileIO(fs::FS &fs, const char * path){
    Serial_monitor.printf("Testing file I/O with %s\r\n", path);

    static uint8_t buf[512];
    size_t len = 0;
    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial_monitor.println("- failed to open file for writing");
        return;
    }

    size_t i;
    Serial_monitor.print("- writing" );
    uint32_t start = millis();
    for(i=0; i<2048; i++){
        if ((i & 0x001F) == 0x001F){
          Serial_monitor.print(".");
        }
        file.write(buf, 512);
    }
    Serial_monitor.println("");
    uint32_t end = millis() - start;
    Serial_monitor.printf(" - %u bytes written in %u ms\r\n", 2048 * 512, end);
    file.close();

    file = fs.open(path);
    start = millis();
    end = start;
    i = 0;
    if(file && !file.isDirectory()){
        len = file.size();
        size_t flen = len;
        start = millis();
        Serial_monitor.print("- reading" );
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            if ((i++ & 0x001F) == 0x001F){
              Serial_monitor.print(".");
            }
            len -= toRead;
        }
        Serial_monitor.println("");
        end = millis() - start;
        Serial_monitor.printf("- %u bytes read in %u ms\r\n", flen, end);
        file.close();
    } else {
        Serial_monitor.println("- failed to open file for reading");
    }
}

void SPIFFS_setup(){
    Serial_monitor.begin(115200);
    if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        Serial_monitor.println("SPIFFS Mount Failed");
        return;
    }
}

//{    
//    listDir(SPIFFS, "/", 0);
//    writeFile(SPIFFS, "/hello.txt", "Hello ");
//    appendFile(SPIFFS, "/hello.txt", "World!\r\n");
//    readFile(SPIFFS, "/hello.txt");
//    renameFile(SPIFFS, "/hello.txt", "/foo.txt");
//    readFile(SPIFFS, "/foo.txt");
//    deleteFile(SPIFFS, "/foo.txt");
//    testFileIO(SPIFFS, "/test.txt");
//    deleteFile(SPIFFS, "/test.txt");
//    Serial_monitor.println( "Test complete" );
//}
