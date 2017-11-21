// ========================================================================================
// Description:       read the HTU21D sensor.
// ========================================================================================

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
  Htu21WriteCommand(0xE7);                  // command for read user register

  Wire.requestFrom(HTU21D_ADDRESS, 1);      // wait for register content
  while(Wire.available() < 1) { }

  byte regCon = Wire.read();
  Serial.print("Register Content: "); 
  Serial.println(regCon, DEC);              // set decimal to bit and look to datasheet

  Wire.beginTransmission(HTU21D_ADDRESS);
  Wire.write(0xE6);                         // command for write register command
  Htu21RegisterSetting(1);                  // max resolution and on-chip heater
  Wire.endTransmission();
}

// ========================================================================================
// TODO: is not the right way, to configure the setting, but it quick to do that.
// ========================================================================================
// option = 0 is default, 1 default with on-chip header
void Htu21RegisterSetting(int option) {

  switch(option) {
    case 1: {
        Wire.write(0x01);                   // register content to write default setup
                                            // measurement:           RH 12Bit / Temp 14Bit
                                            // Status End Battery:    VDD > 2.25V
                                            // Enable on-chip heater: OFF
                                            // Disable OTP reload:    ON
      break;
    }
    case 2: {
        Wire.write(0x03);                   // register content to write default setup
                                            // measurement:           RH 12Bit / Temp 14Bit
                                            // Status End Battery:    VDD > 2.25V
                                            // Enable on-chip heater: ON
                                            // Disable OTP reload:    ON
      break;
    }
  }
  

}


// ========================================================================================
// Read the measure of humidity and temperature. 
// ========================================================================================
// printOn = True to take on of detail output information.
void Htu21ReadSensor(bool printOn) {

  if(printOn) {
    Serial.println("Read HTU21D sensor:");
  }
  
  uint16_t rawTemperature = GetMeasurement(0xE3, 60);    // temperature max time for 14bit result
                                                        // need 58ms
  uint16_t rawHumidity = GetMeasurement(0xE5, 20);       // humidity max time for 12bit result
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

  delay(measureTime);                           // wait 50ms, need delay to wait finish
  
  Wire.requestFrom(HTU21D_ADDRESS, 3);
  int timeOut = 0;
  while(Wire.available() < 3) {
    
    if(timeOut > 10) {                          // break while if wait to long for request
      int res = Wire.available();               // get moment available bytes
      Serial.print("result available: ");       // it only for debug 
      Serial.println(res, DEC);
      break;
    }
    timeOut++;
    delay(10);
  }

  byte msb = Wire.read();
  byte lsb = Wire.read();
  byte checksum = Wire.read();                  // Checksum result, but now is not use 

  uint16_t bitResult = ((uint16_t)msb << 8) | (lsb);
  
  bitResult &= 0xFFFC;                          // set the last two status bits to zero

  return bitResult;
}

// ========================================================================================
// basical write command to modul
void Htu21WriteCommand(byte command) {
  Wire.beginTransmission(HTU21D_ADDRESS);
  Wire.write(command);                        
  Wire.endTransmission();
}
