//
//  ADS1299.h
//  
//  Created by Conor Russomanno on 6/17/13.
//

#ifndef ____ADS1299__
#define ____ADS1299__

#include <stdio.h>
#include <Arduino.h>
//#include <avr/pgmspace.h>
#include "Definitions.h"
#include "SPI.h"
#include <string.h>
#include <math.h>
#include <SD.h>
#include <FS.h>
using namespace std;
class ADS1299 {
public:
    
    void setup(int _DRDY);
    String toBinary(long n);
    //ADS1299 SPI Command Definitions (Datasheet, Pg. 35)
    //System Commands
    void WAKEUP();
    void STANDBY();
    void RESET();
    void START();
    void STOP();
    
    //Data Read Commands
    void RDATAC();
    void SDATAC();
    void RDATA();
    
    //Register Read/Write Commands
    void getDeviceID();
    void RREG(byte _address);
    void RREG(byte _address, byte _numRegistersMinusOne); //to read multiple consecutive registers (Datasheet, pg. 38)
    
    void printRegisterName(byte _address);
    
    void WREG(byte _address, byte _value); //
    void WREG(byte _address, byte _value, byte _numRegistersMinusOne); //
    void appendFile(fs::FS &fs, const char * path, const char * message);
    
    void updateData();
    void updateData_long();
    //SPI Arduino Library Stuff
    byte transfer(byte _data);
    long* getOutput();
    //------------------------//
    void attachInterrupt();
    void detachInterrupt(); // Default
    void begin(); // Default
    void end();
    void setBitOrder(uint8_t);
    void setDataMode(uint8_t);
    void setClockDivider(uint8_t);
    //------------------------//
    
    float tCLK;
    static const int spiClk=1000000;
    int DRDY, CS; //pin numbers for "Data Ready" (DRDY) and "Chip Select" CS (Datasheet, pg. 26)
    SPIClass *spi_in_use;
    int outputCount;
    long output[5];
//    vector<String> registers;
    
};

#endif