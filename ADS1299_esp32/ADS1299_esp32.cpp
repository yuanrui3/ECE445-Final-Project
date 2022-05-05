//
//  ADS1299.cpp
//  
//  Created by Conor Russomanno on 6/17/13.
//


#include "pins_arduino.h"
#include "ADS1299_esp32.h"

//SPIClass * spi_in_use = NULL;

void ADS1299::setup(int _DRDY){
    
    // **** ----- SPI Setup ----- **** //
    
    // Set direction register for SCK and MOSI pin.
    // MISO pin automatically overrides to INPUT.
    // When the SS pin is set as OUTPUT, it can be used as
    // a general purpose output port (it doesn't influence
    // SPI operations).
    
    
    // 1 MHz

    spi_in_use = new SPIClass(HSPI);
    spi_in_use->begin(12, 13, 11, 34);
    // initalize the  data ready and chip select pins:
    DRDY = _DRDY;
    //CS = 10;
    CS=34;
    pinMode(DRDY, INPUT);
    pinMode(spi_in_use->pinSS(), OUTPUT);

    spi_in_use->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE1));
    digitalWrite(CS,HIGH);
    
    tCLK = 0.000666; //666 ns (Datasheet, pg. 8)
    outputCount = 0;
}

//System Commands
void ADS1299::WAKEUP() {
    digitalWrite(CS, LOW); //Low to communicate
    transfer(_WAKEUP);
    digitalWrite(CS, HIGH); //High to end communication
    delay(4.0*tCLK);  //must way at least 4 tCLK cycles before sending another command (Datasheet, pg. 35)
}
void ADS1299::STANDBY() {
    digitalWrite(CS, LOW);
    transfer(_STANDBY);
    digitalWrite(CS, HIGH);
}
void ADS1299::RESET() {
    digitalWrite(CS, LOW);
    transfer(_RESET);
    //delay(10);
    delay(18.0*tCLK); //must wait 18 tCLK cycles to execute this command (Datasheet, pg. 35)
    digitalWrite(CS, HIGH);
}
void ADS1299::START() {
    digitalWrite(CS, LOW);
    transfer(_START);
    digitalWrite(CS, HIGH);
}
void ADS1299::STOP() {
    digitalWrite(CS, LOW);
    transfer(_STOP);
    digitalWrite(CS, HIGH);
}
//Data Read Commands
void ADS1299::RDATAC() {
    digitalWrite(CS, LOW);
    transfer(_RDATAC);
    digitalWrite(CS, HIGH);
}
void ADS1299::SDATAC() {
    digitalWrite(CS, LOW);
    transfer(_SDATAC);
    digitalWrite(CS, HIGH);
}
void ADS1299::RDATA() {
    digitalWrite(CS, LOW);
    transfer(_RDATA);
    digitalWrite(CS, HIGH);
}

//Register Read/Write Commands
void ADS1299::getDeviceID() {
    digitalWrite(CS, LOW); //Low to communicated
    transfer(_SDATAC); //SDATAC
    transfer(_RREG); //RREG
    transfer(0x00); //Asking for 1 byte
    byte data = transfer(0x00); // byte to read (hopefully 0b???11110)
    transfer(_RDATAC); //turn read data continuous back on
    digitalWrite(CS, HIGH); //Low to communicated
    Serial.println(data, BIN);
}

void ADS1299::RREG(byte _address) {
    byte opcode1 = _RREG + _address; //001rrrrr; _RREG = 00100000 and _address = rrrrr
    digitalWrite(CS, LOW); //Low to communicated
    transfer(_SDATAC); //SDATAC
    transfer(opcode1); //RREG
    transfer(0x00); //opcode2
    byte data = transfer(0x00); // returned byte should match default of register map unless edited manually (Datasheet, pg.39)
    printRegisterName(_address);
    Serial.print("0x");
    if(_address<16) Serial.print("0");
    Serial.print(_address, HEX);
    Serial.print(", ");
    Serial.print("0x");
    if(data<16) Serial.print("0");
    Serial.print(data, HEX);
    Serial.print(", ");
    for(byte j = 0; j<8; j++){
        Serial.print(bitRead(data, 7-j), BIN);
        if(j!=7) Serial.print(", ");
    }
    transfer(_RDATAC); //turn read data continuous back on
    digitalWrite(CS, HIGH); //High to end communication
    Serial.println();
}

void ADS1299::RREG(byte _address, byte _numRegistersMinusOne) {
    byte opcode1 = _RREG + _address; //001rrrrr; _RREG = 00100000 and _address = rrrrr
    digitalWrite(CS, LOW); //Low to communicated
    transfer(_SDATAC); //SDATAC
    transfer(opcode1); //RREG
    transfer(_numRegistersMinusOne); //opcode2
    for(byte i = 0; i <= _numRegistersMinusOne; i++){
        byte data = transfer(0x00); // returned byte should match default of register map unless previously edited manually (Datasheet, pg.39)
        printRegisterName(i);
        Serial.print("0x");
        if(i<16) Serial.print("0"); //lead with 0 if value is between 0x00-0x0F to ensure 2 digit format
        Serial.print(i, HEX);
        Serial.print(", ");
        Serial.print("0x");
        if(data<16) Serial.print("0"); //lead with 0 if value is between 0x00-0x0F to ensure 2 digit format
        Serial.print(data, HEX);
        Serial.print(", ");
        for(byte j = 0; j<8; j++){
            Serial.print(bitRead(data, 7-j), BIN);
            if(j!=7) Serial.print(", ");
        }
        Serial.println();
    }
    transfer(_RDATAC); //turn read data continuous back on
    digitalWrite(CS, HIGH); //High to end communication
}

void ADS1299::WREG(byte _address, byte _value) {
    byte opcode1 = _WREG + _address; //001rrrrr; _RREG = 00100000 and _address = rrrrr
    digitalWrite(CS, LOW); //Low to communicated
    transfer(_SDATAC); //SDATAC
    transfer(opcode1);
    transfer(0x00);
    transfer(_value);
    transfer(_RDATAC);
    digitalWrite(CS, HIGH); //Low to communicated
    //Serial.print("Register 0x");
    //Serial.print(_address, HEX);
    //Serial.println(" modified.");
}

void ADS1299::updateData_long(){
    //long output[5];
    //if(digitalRead(DRDY) == LOW){
    digitalWrite(CS, LOW);
//        long output[100][9];
    //long output[5];
    //char* towrite[5];
    long dataPacket;
    for(int i = 0; i<5; i++){
        for(int j = 0; j<3; j++){
            byte dataByte = transfer(0x00);
            dataPacket = (dataPacket<<8) | dataByte;
        }
        //int negative = (dataPacket & (1 << 23)) != 0;
        //Serial.print(dataPacket,HEX);
        //Serial.print(", ");
        int negative = dataPacket>>23;
        //Serial.print(negative);
        //Serial.print(", ");
        if (negative) {
            dataPacket-=(pow(2,24));
        }
        //output[outputCount][i] = dataPacket;
        output[i] = dataPacket;
        dataPacket = 0;
        //towrite[i] = (char*)output[i];
    }
    delay(4*tCLK);
    digitalWrite(CS, HIGH);
    //Serial.print(outputCount);
    //Serial.print(", ");
    /*
    char* toWrite;
    toWrite = new char[5];
    sprintf(toWrite,"%d",outputCount);
    appendFile(SD, "/ads_data.txt",toWrite);
    free(toWrite);
    appendFile(SD, "/ads_data.txt",", ");*/
    for (int i=0;i<5; i++) {
        //Serial.print(output[i], DEC);
        //Serial.print(output[i]*187.5/pow(2,23));
        //Serial.print(output[i]*0.00235);
       // appendFile(SD, "/ads_data.txt", towrite[i]);
        //if(i!=4) Serial.print(", ");
        
        char* toWrite;
        toWrite = new char[9];
        sprintf(toWrite,"%X",output[i]);
        //Serial.print(toWrite);
        //appendFile(SD, "/ads_data.html", toWrite);
        //if(i!=4) appendFile(SD, "/ads_data.html",", ");
        free(toWrite);
    }
    //Serial.println();
    //appendFile(SD, "/ads_data.html","<br>");
    
    //Serial.print("Channel1");
    
    Serial.print(output[1]);
    Serial.print(" ");
    //Serial.print("Channel2");
    Serial.print(output[2]);
    Serial.print(" ");
    //Serial.print("Channel3");
    Serial.print(output[3]);
    Serial.print(" ");
    //Serial.print("Channels4");
    Serial.println(output[4]);
    outputCount++;
    //appendFile(SD, "/ads_data.txt", towrite[0]);
    //appendFile(SD, "/ads_data.txt", "yuanrui3");
    /*
    delay(10);
    spi_in_use->endTransaction();
    digitalWrite(34,LOW);
    if(SD.begin(34)){
    Serial.println("Card Mount Succeeded");
    }
    //
    SD.end();
    digitalWrite(34,HIGH);
    spi_in_use->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE1));
    delay(5);*/
    //return output;
    //return output;
}
long* ADS1299::getOutput() {
    return output;
}

void ADS1299::appendFile(fs::FS &fs, const char * path, const char * message){
  //Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for appending");
    return;
  }
  
  if (file.print(message)) {
      Serial.println("File appended");
  }

  file.close();
}
// String-Byte converters for RREG and WREG
void ADS1299::printRegisterName(byte _address) {
    if(_address == ID){
        Serial.print("ID, ");
    }
    else if(_address == CONFIG1){
        Serial.print("CONFIG1, ");
    }
    else if(_address == CONFIG2){
        Serial.print("CONFIG2, ");
    }
    else if(_address == CONFIG3){
        Serial.print("CONFIG3, ");
    }
    else if(_address == LOFF){
        Serial.print("LOFF, ");
    }
    else if(_address == CH1SET){
        Serial.print("CH1SET, ");
    }
    else if(_address == CH2SET){
        Serial.print("CH2SET, ");
    }
    else if(_address == CH3SET){
        Serial.print("CH3SET, ");
    }
    else if(_address == CH4SET){
        Serial.print("CH4SET, ");
    }
    else if(_address == CH5SET){
        Serial.print("CH5SET, ");
    }
    else if(_address == CH6SET){
        Serial.print("CH6SET, ");
    }
    else if(_address == CH7SET){
        Serial.print("CH7SET, ");
    }
    else if(_address == CH8SET){
        Serial.print("CH8SET, ");
    }
    else if(_address == BIAS_SENSP){
        Serial.print("BIAS_SENSP, ");
    }
    else if(_address == BIAS_SENSN){
        Serial.print("BIAS_SENSN, ");
    }
    else if(_address == LOFF_SENSP){
        Serial.print("LOFF_SENSP, ");
    }
    else if(_address == LOFF_SENSN){
        Serial.print("LOFF_SENSN, ");
    }
    else if(_address == LOFF_FLIP){
        Serial.print("LOFF_FLIP, ");
    }
    else if(_address == LOFF_STATP){
        Serial.print("LOFF_STATP, ");
    }
    else if(_address == LOFF_STATN){
        Serial.print("LOFF_STATN, ");
    }
    else if(_address == GPIO){
        Serial.print("GPIO, ");
    }
    else if(_address == MISC1){
        Serial.print("MISC1, ");
    }
    else if(_address == MISC2){
        Serial.print("MISC2, ");
    }
    else if(_address == CONFIG4){
        Serial.print("CONFIG4, ");
    }
}

//SPI communication methods

byte ADS1299::transfer(byte _data) {
    /*
    SPDR = _data;
    while (!(SPSR & _BV(SPIF)))
        ;
    return SPDR;*/
    //spi_in_use->beginTransaction(SPISettings(240000000,SPI_MSBFIRST,SPI_MODE0));
    return spi_in_use->transfer(_data);
    //spi_in_use->endTransaction();
    //eturn data;
}




