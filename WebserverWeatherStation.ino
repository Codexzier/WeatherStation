// ========================================================================================
//      Meine Welt in meinem Kopf
// ========================================================================================
// Projekt:       Weatherstation
// Author:        Johannes P. Langner
// Controller:    WEMOS D1 Mini
// Sensors:       HTU21, BMP180
// Actor:         
// Description:   Ein kleines Projekt einer Wetterstation mit zwei Sensor Modulen.
// ========================================================================================
// Open Tasks:    1. OLED Output
//                2. Webserver
//                3. HTML and JavaScript
//                4. Connect to an central system
// ========================================================================================

#include <Wire.h>                   
#define BMP085_ADDRESS 0x77         // standard address of BMP180 Sensor
#define HTU21D_ADDRESS 0x40         // standard address of HTU21, HTU21D or SHT21

// ========================================================================================
// Setup

const byte OSS = 0;                 // BMP180, Oversampling Setting

// ========================================================================================
// Result variables

float mTemperatures[2];             // HTU21 and BMP180 have a temperature sensor part.
float mPressure;                    // BMP180, pressure result
float mAtmosphere;                  // BMP180, a function atmosphere result of pressure and temperature
float mAltitude;                    // BMP180, a function altitude result of pressure and temperature
float mHumidity;                    // HTU21, humidity result

// ========================================================================================
void setup() {
  Serial.begin(115200);
  
  Wire.begin();                     // Start the I²C 
  Wire.setClock(100000);            // set the standard clock to 100kHz, 
                                    // not sure it is the real standard value of using 'wire'.

  bmp085Calibration();              // get the calibration value for the BMP180 results for any calculations.  
}


// ========================================================================================
void loop() {

  ReadBmp180Sensor(false);          // read bmp180 sensor, get temperatue and pressure, false = detail print off
  ReadHtu21Sensor(false);           // read htu21 sensor, get temperature and humidity, false = detail print off

  PrintAllResults();                // print all finsihed results
  delay(2000);
}

// ========================================================================================
// all readed sensor and calculation results write out to serial
void PrintAllResults() {
  Serial.println("-----------------------");
  Serial.print("\tHumidity: "); 
  Serial.println(mHumidity, 2);
  
  // Get the average result from two sensors
  float averageTemperature = (mTemperatures[0] + mTemperatures[1]) / 2.0;
  Serial.print("\tTemperature: "); 
  Serial.println(averageTemperature, 2);
  Serial.print("\tPressure: "); 
  Serial.print(mPressure / 100.0, 2); Serial.println(" hPa");
  Serial.print("\tStandard Atmosphere: "); 
  Serial.println(mAtmosphere, 4);
  Serial.print("\tAltitude: "); 
  Serial.print(mAltitude, 2); Serial.println(" M");

  Serial.println("-----------------------");
  Serial.println();
}

