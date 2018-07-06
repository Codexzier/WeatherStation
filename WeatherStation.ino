// ========================================================================================
//      Meine Welt in meinem Kopf
// ========================================================================================
// Projekt:       Weatherstation
// Author:        Johannes P. Langner
// Controller:    WEMOS D1 Mini
// Sensors:       HTU21, BMP180
// Actor:         OLED SSD1306 64x48
// Description:   Ein kleines Projekt einer Wetterstation mit zwei Sensor Modulen.
// ========================================================================================
// Open Tasks:    1. HTML and JavaScript
//                2. Connect to a central application or service
// ========================================================================================

#include <Time.h>
#include <TimeLib.h>

#include <SPI.h>
#include <SD.h>                             // library for the sd card

#include <ESP8266WiFi.h>
#include <Wire.h>                   
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>               // this library has the specific mode for 64x48 display

#define BMP085_ADDRESS 0x77                 // standard address of BMP180 Sensor
#define HTU21D_ADDRESS 0x40                 // standard address of HTU21, HTU21D or SHT21

// ========================================================================================
// SD Card
Sd2Card mCard;
SdVolume mVolume;
SdFile mRoot;
const int mChipSelect = D8;

boolean mSdCardOk = false;                  // set true, if sd card success initialized.
int mPerSecond = 10;                       // save only by any secound
int mLastSecond = 0;
int mCountSecond = 0;

int32_t mMeasureCount = 0;
String mStringMeasurement = "";
// ========================================================================================
// WLAN
const char* mSsid = "wlanName";
const char* mPassword = "wlanPassword";

WiFiServer mServer(80);
IPAddress mIp(192, 168, 20, 99);             // where xx is the desired IP Address
IPAddress mGateway(192, 168, 20, 1);         // set gateway to match your network

// ========================================================================================
// Setup display and offset for sensors

const byte OSS = 0;                         // BMP180, Oversampling Setting
Adafruit_SSD1306 mOled(0);                  // set pins for D1 and D2 for the I²C connection pins

float mOffsetTemperature = 22.4 - 25.9;     // set offset the temperature result from a reference sensor
float mOffsetHumidity = 41.0 - 30.0;        // set offset the humidity result from reference sensor

#if (SSD1306_LCDHEIGHT != 48)
#error("You musst changed in Adafruit_SSD1306.h!"); // Check the right setting display resulotion.
#endif

// ========================================================================================
// Result variables

float mTemperatures[2];                     // HTU21 and BMP180 have a temperature sensor part.
float mPressure;                            // BMP180, pressure result
float mAtmosphere;                          // BMP180, a function atmosphere result of pressure and temperature
float mAltitude;                            // BMP180, a function altitude result of pressure and temperature
float mHumidity;                            // HTU21, humidity result

// ========================================================================================
// array of tracked variables

int mIndex = 0;                             // index of temperature result array
float mTemperaturesArray[64];               // an array of 64 result
                                            // it used for the bottom diagram

// ========================================================================================
// ground humidity status by analog sensor

int mPinInputSignal = A0;                   // pin for read the analog signal
int mInputValue = 0;                        // hold the actual analog measurement
int mInputValueMax = 0;                     // hold the max value of measurement
int mInputValueMin = 1024;                  // hold the min value of measuerment
                                            // Min and max value can be usful to find out 
                                            // the value for a other target to do.

// ========================================================================================
void setup() {
  Serial.begin(115200);
  Serial.println();                         // only one row. 
                                            // sometime the controller shows other unencoded chars
  
  Wire.begin();                             // Start the I²C 
  Wire.setClock(400000);                    // set the standard clock is 100kHz, but set fast mode
                                            // not sure it is the real standard value of using 'wire'.

  OledSetup();                              // base OLED setup. Start, Clear, fontsize and set font type.
  
  StartWebserver();                         // Connect to WiFi network

  Bmp180Calibration();                      // get the calibration value for the BMP180 results for any calculations.  
  Htu21PrepareSensor();                     // make a softreset
  
  SetActualTemperatureToDiagrammArray();    // read temperature and write it to all array index

  SdCardInitSdCard();                             // check sd card exist and show some information.
}

// ========================================================================================
void loop() {

  ReadAnalogInput();                        // read sensor with an analog signal result

  Bmp180ReadSensor(false);                  // read bmp180 sensor, get temperatue and pressure, false = detail print off
  Htu21ReadSensor(false);                   // read htu21 sensor, get temperature and humidity, false = detail print off

  RecordTemperatureToArray();               // record temperature for diagram output
  
  mOled.clearDisplay();
  OledPrintTitleAndValue(0, "T: ", mTemperaturesArray[mIndex] + mOffsetTemperature);
  OledPrintTitleAndValue(1, "H: ", mHumidity + mOffsetHumidity);
  OledPrintTitleAndValue(2, "P: ", mPressure / 100.0);
  OledPrintTitleAndValue(3, "W:", mInputValue);
  
  OledPrintDiagramResults();                // Render the temperature results to a diagram 
  mOled.display(); 

  PrintOnWebsite(50);                       // post on website, if calling by webbrowser

  CountUp();                                // up counter
  SaveToSdCard();                           // save collected measure data
  
  //PrintAllResults();                      // print all finsihed results
  //delay(50);
}

// ========================================================================================
// Set count one up, if it next second
void CountUp() {
  
  unsigned long nowSec = second();
  if(nowSec != mLastSecond) {
    mCountSecond++;
  }
  mLastSecond = second();
}

// ========================================================================================
// If sd card initialize success, the readed data can save
void SaveToSdCard() {
  
  if(mSdCardOk && mCountSecond > mPerSecond) {

    Serial.println("Save data to sd card");
    SdCardSave();
    mCountSecond = 0;
    mStringMeasurement = "";
  }
}

// ========================================================================================
// read the one pin for analog signal
void ReadAnalogInput(){
  
  mInputValue = analogRead(mPinInputSignal);

  if(mInputValue > mInputValueMax) {
    mInputValueMax = mInputValue;
  }

  if(mInputValue < mInputValueMin) {
    mInputValueMin = mInputValue;
  }
}

// ========================================================================================
// set the actual average measurement of temperatures.
void RecordTemperatureToArray() {
  
  if(mIndex < 63) { mIndex++; }
  else { mIndex = 0; }
  
  mTemperaturesArray[mIndex] = (mTemperatures[0] + mTemperatures[1]) / 2.0;
}

// ========================================================================================
// need the actual temperature to display non initial zero value.
// it only a optical start optimzing
void SetActualTemperatureToDiagrammArray() {
  
  // set first measure to array
  Bmp180ReadSensor(false);                  // read bmp180 sensor, get temperatue and pressure, false = detail print off
  Htu21ReadSensor(false);                   // read htu21 sensor, get temperature and humidity, false = detail print off
  
  for(int i = 0; i < 64; i++) {
    mTemperaturesArray[i] = (mTemperatures[0] + mTemperatures[1]) / 2.0;
  }
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

