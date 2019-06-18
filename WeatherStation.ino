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
// Open Tasks:    1. Connect to a central application or service
//                2. Show saved data on HTML side
//                3. to set the time
//                4. set time from ntp server
// ========================================================================================

#include <Time.h>
#include <TimeLib.h>

#include <SPI.h>
#include <SD.h>                             // library for the sd card

#include <ESP8266WiFi.h>
#include <Wire.h>                   
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>               // this library has the specific mode for 64x48 display
#include <ArduinoJson.h>                    // library to write and read json format data

#define BMP085_ADDRESS 0x77                 // standard address of BMP180 Sensor
#define HTU21D_ADDRESS 0x40                 // standard address of HTU21, HTU21D or SHT21
#define DS1307_I2C_ADDRESS 0x68             // standard address of datalogger shield


// ========================================================================================
// SD Card
Sd2Card mCard;
SdVolume mVolume;
SdFile mRoot;
const int mChipSelect = D8;

boolean mSdCardOk = false;                  // set true, if sd card success initialized.
int mPerSecond = 10;                        // save only by X secound
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
// for connecting to ntp

//RTC_DS1307 RTC;

unsigned int localPort = 8888;              // local port to listen for UDP packets

// find your local ntp server http://www.pool.ntp.org/zone/europe or 
// http://support.ntp.org/bin/view/Servers/StratumTwoTimeServers
// byte timeServer[] = {192, 43, 244, 18}; // time.nist.gov NTP server
byte timeServer[] = {193, 79, 237, 14};    // ntp1.nl.net NTP server  

const int NTP_PACKET_SIZE= 48;             // NTP time stamp is in the first 48 bytes of the message
byte pb[NTP_PACKET_SIZE];                  // buffer to hold incoming and outgoing packets 

// for displaying day
String mDaysOfWeek[8] = {"","Sonntag","Montag","Dienstag","Mittwoch","Donnerstag","Freitag","Samstag"};

byte mActualDateTime[7];                  // secound, minute, hour, day, day of month, month, year

// ========================================================================================
// measure data object
class MeasureData {

  public:
    int Day;
    int Month;
    int Year;
    int Hour;
    int Minute;
    float Temperature;
    float Humidity;
    float Pressure;

    // print all values 
    void Print() {
      Serial.print("Day "); Serial.print(Day, DEC); Serial.print(" ");
      Serial.print("Month "); Serial.print(Month, DEC); Serial.print(" ");
      Serial.print("Year "); Serial.print(Year, DEC); Serial.print(" ");

      Serial.print("Hour "); Serial.print(Hour, DEC); Serial.print(" ");
      Serial.print("Minute "); Serial.print(Minute, DEC); Serial.print(" ");

      Serial.print("Temperature "); Serial.print(Temperature, DEC); Serial.print(" ");
      Serial.print("Humidity "); Serial.print(Humidity, DEC); Serial.print(" ");
      Serial.print("Pressure "); Serial.print(Pressure, DEC); Serial.println(" ");
    }

    void ReadFromJson(JsonObject measure) {
      Day = measure["Day"];
      Month = measure["Month"];
      Year = measure["Year"];
      Hour = measure["Hour"];
      Minute = measure["Minute"];
      Temperature = measure["Temperature"];
      Humidity = measure["Humidity"];
      Pressure = measure["Pressure"];
    }
};

// ========================================================================================
// record average data 

//"Day;Month;Year;Hour;Minute;Temperature;Humidity;Pressure;"
// for save average data
int mActualDay = -1;
int mActualMonth = -1;
int mActualYear = -1;
int mActualHour = -1;
//int mActualMinute = -1;

float mAverageTemperature = -1;
float mAverageHumidity = -1;
float mAveragePressure = -1;

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
  Wire.setClock(100000);                    // set the standard clock is 100kHz, but set fast mode
                                            // not sure it is the real standard value of using 'wire'.

  OledSetup();                              // base OLED setup. Start, Clear, fontsize and set font type.
  
  StartWebserver();                         // Connect to WiFi network

  Bmp180Calibration();                      // get the calibration value for the BMP180 results for any calculations.  
  Htu21PrepareSensor();                     // make a softreset
  
  SetActualTemperatureToDiagrammArray();    // read temperature and write it to all array index

  SdCardInitSdCard();                       // check sd card exist and show some information.

  pinMode(mChipSelect, OUTPUT);             // pin select for sd card

  //SetupDS1307(01,14,1,7,4,19);              // Setup the actual date time (Method to check rtc time is planed)
}

// ========================================================================================
void loop() {

  ReadAnalogInput();                        // read sensor with an analog signal result

  Bmp180ReadSensor(false);                  // read bmp180 sensor, get temperatue and pressure, false = detail print off
  Htu21ReadSensor(false);                   // read htu21 sensor, get temperature and humidity, false = detail print off
  ReadBinarDs1307();       // read actual date time from rtc modul

  RecordTemperatureToArray();               // record temperature for diagram output
  
  mOled.clearDisplay();
  OledPrintTitleAndValue(0, "T: ", mTemperaturesArray[mIndex] + mOffsetTemperature);
  OledPrintTitleAndValue(1, "H: ", mHumidity + mOffsetHumidity);
  OledPrintTitleAndValue(2, "P: ", mPressure / 100.0);
  OledPrintTitleAndValue(3, "W:", mInputValue);
  
  OledPrintDiagramResults();                // Render the temperature results to a diagram 
  mOled.display(); 

  PrintOnWebsite(50, true);                 // post on website, if calling by webbrowser

  CountUp();                                // up counter
  SaveToSdCard();                           // save collected measure data
    
                                            // save data for webside
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
