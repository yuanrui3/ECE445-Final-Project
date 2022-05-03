#include <SD.h>
#include <FS.h>
#include <ADS1299_esp32.h>
#include <Arduino.h>​
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include "time.h"
ADS1299 ADS;

//Arduino Uno - Pin Assignments; Need to use ICSP for later AVR boards
// SCK = 13
// MISO [DOUT] = 12
// MOSI [DIN] = 11
// CS = 10; 
// DRDY = 9;

// Replace with your network credentials
const char* ssid = "abc";
const char* password = "6032961203";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String dataMessage;

// NTP server to request epoch time
const char* ntpServer = "pool.ntp.org";

// Variable to save current epoch time
unsigned long epochTime; 

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}
//  0x## -> Arduino Hexadecimal Format
//  0b## -> Arduino Binary Format

boolean deviceIDReturned = false;
boolean startedLogging = false;
//int counter=0;
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("ADS1299-bridge has started!");
  initWiFi();
  delay(50);
  //pinMode(34,OUTPUT);
  //digitalWrite(34,HIGH);
  pinMode(8,OUTPUT);//RESET
  pinMode(10,OUTPUT);//START
  ADS.setup(7); // (DRDY pin, CS pin);
  delay(10);
  /*
  SPI.transfer(0x00);*/
  if(SD.begin(0)){
  Serial.println("Card Mount Succeeded!");
  }
  //delay(10);
  appendFile(SD, "/ads_data.html", "<br>");
  appendFile(SD, "/ads_data.html", "begin<br>");
  /*digitalWrite(0,LOW);
  appendFile(SD, "/ads_data.txt", "begin\n");*/
  //digitalWrite(0,HIGH);
  
  delay(10);  //delay to ensure connection
 
  //pinMode(5,OUTPUT);
  ADS.RESET();
  //long* channels;
  //digitalWrite(2,HIGH);
  //digitalWrite(4,HIGH);
  configTime(0, 3600, ntpServer);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SD, "/ads_data.html", "text/html");
  });

  server.serveStatic("/", SD, "/");

  server.begin();
}

void loop(){
  
  if(deviceIDReturned == false){
    
    ADS.getDeviceID(); //Funciton to return Device ID
    ADS.SDATAC(); 
    //prints dashed line to separate serial print sections
    Serial.println("----------------------------------------------");
    
    //Read ADS1299 Register at address 0x00 (see Datasheet pg. 35 for more info on SPI commands)
    ADS.RREG(0x00);
    Serial.println("----------------------------------------------");
    
    //PRINT ALL REGISTERS... Read 0x17 addresses starting from address 0x00 (these numbers can be replaced by binary or integer values)
    ADS.RREG(0x00, 0x17);
    Serial.println("----------------------------------------------");
    
    //Write register command (see Datasheet pg. 38 for more info about WREG)
    ADS.WREG(CONFIG1, 0x96);
    Serial.println("----------------------------------------------");
    ADS.WREG(CONFIG2, 0xD2);
    ADS.WREG(CONFIG3,0xE0);
    ADS.WREG(CH4SET,0x61);
    //ADS.WREG(CH5SET,0x80);
    //ADS.WREG(CH6SET,0x80);
    ADS.WREG(CH1SET,0x61);
    ADS.WREG(CH2SET,0x61);
    ADS.WREG(CH3SET,0x61);
    //Repeat PRINT ALL REGISTERS to verify that WREG changed the CONFIG1 register
    ADS.RREG(0x00, 0x17);
    Serial.println("----------------------------------------------");
    
    //Start data conversions command
    digitalWrite(8,HIGH);
    ADS.START(); //must start before reading data continuous
    digitalWrite(10,HIGH);
    delay(4);
    ADS.RDATAC();
    deviceIDReturned = true;
  }
  //print data to the serial console for only the 1st 10seconds of 
  /*while(millis()<10000){
    if(startedLogging == false){
      Serial.print("Millis: "); //this is to see at what time the data starts printing to check for timing accuracy (default sample rate is 250 sample/second)
      Serial.println(millis());
      startedLogging = true;
    }*/
    //counter++;
    //if (counter==500) digitalWrite(5,HIGH);
    //Print Read Data Continuous (RDATAC) to Ardiuno serial monitor... 
    //The timing of this method is not perfect yet. Some data is getting lost 
    //and I believe its due to the serial monitor taking too much time to print data and not being ready to recieve to packets
    //Serial.println(digitalRead(1));
    //Get epoch time
    epochTime = getTime();

    //Concatenate all info separated by commas
    if (digitalRead(7)==LOW) {
      dataMessage = String(epochTime)+ "<br>";
      Serial.print("Saving data: ");
      Serial.println(dataMessage);
  
      //Append the data to file
      appendFile(SD, "/ads_data.html", dataMessage.c_str());
      ADS.updateData_long(); 
    }
    
    /*
    Serial.print("Channel1");
    Serial.print(channels[1]);
    Serial.print(" ");
    Serial.print("Channel2");
    Serial.print(channels[2]);
    Serial.print(" ");
    Serial.print("Channel3");
    Serial.print(channels[3]);
    Serial.print(" ");
    Serial.print("Channels4");
    Serial.println(channels[4]);*/
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if(!root){
    Serial.println("Failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void createDir(fs::FS &fs, const char * path){
  Serial.printf("Creating Dir: %s\n", path);
  if(fs.mkdir(path)){
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

void removeDir(fs::FS &fs, const char * path){
  Serial.printf("Removing Dir: %s\n", path);
  if(fs.rmdir(path)){
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir failed");
  }
}

void readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while(file.available()){
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)){
      Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void deleteFile(fs::FS &fs, const char * path){
  Serial.printf("Deleting file: %s\n", path);
  if(fs.remove(path)){
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void testFileIO(fs::FS &fs, const char * path){
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if(file){
    len = file.size();
    size_t flen = len;
    start = millis();
    while(len){
      size_t toRead = len;
      if(toRead > 512){
        toRead = 512;
      }
      file.read(buf, toRead);
      len -= toRead;
    }
    end = millis() - start;
    Serial.printf("%u bytes read for %u ms\n", flen, end);
    file.close();
  } else {
    Serial.println("Failed to open file for reading");
  }


  file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }

  size_t i;
  start = millis();
  for(i=0; i<2048; i++){
    file.write(buf, 512);
  }
  end = millis() - start;
  Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
  file.close();
}

void SD_init() {
  if(!SD.begin(34)){
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  listDir(SD, "/", 0);
  createDir(SD, "/mydir");
  listDir(SD, "/", 0);
  removeDir(SD, "/mydir");
  listDir(SD, "/", 2);
  writeFile(SD, "/hello.txt", "Hello ");
  appendFile(SD, "/hello.txt", "World!\n");
  readFile(SD, "/hello.txt");
  deleteFile(SD, "/foo.txt");
  renameFile(SD, "/hello.txt", "/foo.txt");
  readFile(SD, "/foo.txt");
  testFileIO(SD, "/test.txt");
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
  //SD.end();
}
