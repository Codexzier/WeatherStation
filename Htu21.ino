// ========================================================================================
// Description:       read the HTU21D sensor.
// ========================================================================================

// ========================================================================================
// Read the measure of humidity and temperature. 
// ========================================================================================
// printOn = True to take on of detail output information.
void ReadHtu21Sensor(bool printOn) {

  if(printOn) {
    Serial.println("Read HTU21D sensor:");
  }
  
  int16_t rawTemperature = GetMeasurement(0xE3);
  int16_t rawHumidity = GetMeasurement(0xE5);

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
float GetTemperature(int16_t rawValue) {
  return -46.85f + (175.72f  / 65536.0 * (float)rawValue);
}

// ========================================================================================
// Calculate a raw temperature value to a readable value
// ========================================================================================
// rawValue = set the raw humidity value.
float GetHumidity(int16_t rawValue){
  return -6 + (125.0 / 65536.0 * (float)rawValue);
}

// ========================================================================================
// Get the specific measure from sensor by command value
// ========================================================================================
// command = command byte for specific measure
int16_t GetMeasurement(byte command) {
  
  Wire.beginTransmission(HTU21D_ADDRESS);
  Wire.write(command);
  Wire.endTransmission();

  delay(50);                                    // wait 50ms, need delay to wait finish
  
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

  int16_t dataResult1 = Wire.read();
  int16_t dataResult2 = Wire.read();
  //int16_t dataResult3 = Wire.read();         // Check value, I must read the datasheet again :D

  int16_t bitResult = (dataResult1 << 8) | dataResult2;
  
  bitResult &= 0xFFFC; // ~0x0003;             // found two diffrent operations

  return bitResult;
}
