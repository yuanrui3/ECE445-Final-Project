//Developed by Conor Russomanno (June 2013)
//This example uses the ADS1299 Arduino Library, a software bridge between the ADS1299 TI chip and 
//Arduino. See http://www.ti.com/product/ads1299 for more information about the device and the reference
//folder in the ADS1299 directory for more information about the library.

//Note: this Library and example file are still in early stages of development.
/*Currently, you are able to:
  - execute all System Commands (Datasheet, pg. 35-36)
  - Enable/disable read data continuous mode (Datasheet, pg. 35-38)
  - Read and write registers of the ADS1299 using RREG and WREG with HEX or BINARY values to edit the appropriate bits (Datasheet, pg. 39-47)
  
  Future Methods:
  - Commands/functions to edit ADS1299 settings without having to rewrite entire register bytes (ex. "setNumChannels(4);" would automatically power-down channels 5-8 by accessing those bits behind the scenes)
  - Fix timing issue with RDATAC - right now some data is getting lost (from test below, you'll see the serial monitor consistantly prints ~1511 samples from 225ms to 10000ms, but that # should be ~2443 based on default 250samples/second setting)
  - add SD-card functionality to log data packets to an SD
  - add txt file creation/writing for data storage when operating through a computer
*/

#include <ADS1299.h>

ADS1299 ADS;

//Arduino Uno - Pin Assignments; Need to use ICSP for later AVR boards
// SCK = 13
// MISO [DOUT] = 12
// MOSI [DIN] = 11
// CS = 10; 
// DRDY = 9;

//  0x## -> Arduino Hexadecimal Format
//  0b## -> Arduino Binary Format

boolean deviceIDReturned = false;
boolean startedLogging = false;

void setup() {

  Serial.begin(230400);
  Serial.println();
  Serial.println("ADS1299-bridge has started!");
  
  ADS.setup(9, 10); // (DRDY pin, CS pin);
  pinMode(4,OUTPUT);//START
  pinMode(2,OUTPUT);//RESET
  delay(10);  //delay to ensure connection
  
  ADS.RESET();
  //digitalWrite(2,HIGH);
  //digitalWrite(4,HIGH);
}

void loop(){
  
  if(deviceIDReturned == false){
    
    //ADS.getDeviceID(); //Funciton to return Device ID
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
    digitalWrite(2,HIGH);
    ADS.START(); //must start before reading data continuous
    //digitalWrite(4,HIGH);
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
    
    //Print Read Data Continuous (RDATAC) to Ardiuno serial monitor... 
    //The timing of this method is not perfect yet. Some data is getting lost 
    //and I believe its due to the serial monitor taking too much time to print data and not being ready to recieve to packets
    ADS.updateData_long();  

  
  
}
