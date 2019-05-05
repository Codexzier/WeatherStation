// ========================================================================================
// Description:       read the HTU21D sensor.
// ========================================================================================

// ========================================================================================
// Settings

int mMeasurementOption = 0;                 // 0 = RH 12bit / 14Bit Temp
                                            // 1 = RH  8bit / 12Bit Temp
                                            // 2 = RH 10bit / 13Bit Temp
                                            // 3 = RH 11bit / 11Bit Temp
bool mOnChipHeater = false;                 // enable on-chip heater
bool mOtpReload = false;                    // enable OTP reload, well i inverted this option

// ========================================================================================
//  prepare sensor to start with default settings.
void Htu21PrepareSensor() {
  Htu21SoftReset();
  Htu21Setup();
}

// ========================================================================================
void Htu21SoftReset() {
  Serial.println("Softreset HTU21");
  Htu21WriteCommand(0xFE);                  // command for reset
  delay(20);                                // needed 15ms to reboot
}

// ========================================================================================
// read setting to show to display and write new setting.
void Htu21Setup() {
  Serial.println("Setup HTU21");

  Serial.println("Read setup: ");
  Htu21ReadUserRegister();
  Serial.println();

  Wire.beginTransmission(HTU21D_ADDRESS);
  Wire.write(0xE6);                                       // command for write register command
  Htu21WriteRegisterSetting(mMeasurementOption, mOnChipHeater, mOtpReload);
  Wire.endTransmission();

  Serial.println();
  Serial.println("Read setup after set:");
  Htu21ReadUserRegister();
}

void Htu21ReadUserRegister() {
  
  Htu21WriteCommand(0xE7);                                // command for read user register

  Wire.requestFrom(HTU21D_ADDRESS, 1);                    // wait for register content
  while(Wire.available() < 1) { }

  byte regCon = Wire.read();
  Serial.print("\tRegister Result: "); 
  Serial.println(regCon, BIN);                            // set decimal to bit and look to datasheet
}

// ========================================================================================
// set the configuration of htu21d for using other resolution.
// ========================================================================================
// measurement  = 0 = RH 12bit / 14Bit Temp
//                1 = RH  8bit / 12Bit Temp
//                2 = RH 10bit / 13Bit Temp
//                3 = RH 11bit / 11Bit Temp
// enableHeader = enable the on-chip header
// otpReload    = if set '0', the default setting loads after each time measurements
//                found not how many times going to set default.
void Htu21WriteRegisterSetting(int measurement, bool enableHeader, bool otpReload) {
  
  byte command = 0x00;

  if(measurement > 0) {
    command |= (byte)(measurement << 6);                  // set the bits for the measurement resultion
  }

  Serial.print("\tMeasurement resolution: ");
  switch(measurement) {
    case(1): { Serial.println("\tRH  8bit / 12Bit Temp"); break; }
    case(2): { Serial.println("\tRH 10bit / 13Bit Temp"); break; }
    case(3): { Serial.println("\tRH 11bit / 11Bit Temp"); break; }
    default: { Serial.println("\tRH 12bit / 14Bit Temp"); break; }
  }

  if(enableHeader) {
    command |= 0x02;                                      // set the bit for enable
  }
  Serial.print("\tOn-Chip header: \t\t"); Serial.println(enableHeader, BIN);

  if(!otpReload) {                                        // Invert enter, well from datasheet is disable if on
    command |= 0x01;                                      // set the bit for disable
  }
  Serial.print("\tOTP Reload: \t\t\t"); Serial.println(enableHeader, BIN);

  Wire.write(command);                                    // write the finish command
}

// ========================================================================================
// Read the measure of humidity and temperature. 
// ========================================================================================
// printOn = True to take on of detail output information.
void Htu21ReadSensor(bool printOn) {

  if(printOn) {
    Serial.println("Read HTU21D sensor:");
  }
  
  uint16_t rawTemperature = GetMeasurement(0xE3, 60);     // temperature max time for 14bit result
                                                          // need 58ms
  uint16_t rawHumidity = GetMeasurement(0xE5, 20);        // humidity max time for 12bit result
                                                          // need 18ms

  float temperatur = GetTemperature(rawTemperature);
  mHumidity = GetHumidity(rawHumidity);

  mTemperatures[1] = temperatur;

  if(printOn) {
    Serial.print("\tRaw Temperature: "); Serial.println(rawTemperature, DEC);
    Serial.print("\tRaw Humidity: "); Serial.println(rawHumidity, DEC);
    Serial.print("\tTemperature: "); Serial.println(temperatur, DEC);
    Serial.print("\tHumidity: "); Serial.println(mHumidity, DEC);
    Serial.println();
  }
}

// ========================================================================================
// Calculate a raw temperature value to a readable value
// ========================================================================================
// rawValue = set the raw temperature value.
float GetTemperature(uint16_t rawValue) {
  return -46.85f + (175.72f * (float)rawValue / 65536.0);
}

// ========================================================================================
// Calculate a raw temperature value to a readable value
// ========================================================================================
// rawValue = set the raw humidity value.
float GetHumidity(uint16_t rawValue){
  return -6 + (125.0 * (float)rawValue / 65536.0);
}

// ========================================================================================
// Get the specific measure from sensor by command value
// ========================================================================================
// command = command byte for specific measure
uint16_t GetMeasurement(byte command, int measureTime) {

  Htu21WriteCommand(command);

  delay(measureTime);                                     // wait 50ms, need delay to wait finish
  
  Wire.requestFrom(HTU21D_ADDRESS, 3);
  int timeOut = 0;
  while(Wire.available() < 3) {
    
    if(timeOut > 10) {                                    // break while if wait to long for request
      int res = Wire.available();                         // get moment available bytes
      Serial.print("result available: ");                 // it only for debug 
      Serial.println(res, DEC);
      break;
    }
    timeOut++;
    delay(10);
  }

  byte msb = Wire.read();
  byte lsb = Wire.read();
  byte checksum = Wire.read();                            // Checksum result, but now is not use 

  uint16_t bitResult = ((uint16_t)msb << 8) | (lsb);
  
  bitResult &= 0xFFFC;                                    // set the last two status bits to zero

  return bitResult;
}

// ========================================================================================
// basical write command to modul
void Htu21WriteCommand(byte command) {
  Wire.beginTransmission(HTU21D_ADDRESS);
  Wire.write(command);                        
  Wire.endTransmission();
}
