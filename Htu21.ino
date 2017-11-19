// ========================================================================================
// Description:       read the HTU21D sensor.
// ========================================================================================

// ========================================================================================
// data result from reading
byte htu21DataResult[3] = { 0x00, 0x00, 0x00 };

// ========================================================================================
// Read the measure of humidity and temperature. 
// ========================================================================================
// printOn = True to take on of detail output information.
void ReadHtu21Sensor(bool printOn) {

  if(printOn) {
    Serial.println("Read HTU21D sensor:");
  }
  
  int32_t rawTemperature = GetMeasurement(0xE3);
  int32_t rawHumidity = GetMeasurement(0xE5);

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
float GetTemperature(int32_t rawValue) {
  float f = (float)rawValue / 65536.0;
  return -46.85f + (175.72f * f);
}

// ========================================================================================
// Calculate a raw temperature value to a readable value
// ========================================================================================
// rawValue = set the raw humidity value.
float GetHumidity(int32_t rawValue){
  float f = (float)rawValue / 65536.0;
  return -6 + (125.0 * f);
}

// ========================================================================================
// Get the specific measure from sensor by command value
// ========================================================================================
// command = command byte for specific measure
int32_t GetMeasurement(byte command) {
  
  Wire.beginTransmission(HTU21D_ADDRESS);
  Wire.write(command);
  Wire.endTransmission();

  delay(50);                                // wait 50ms, need delay to wait finish
  
  Wire.requestFrom(HTU21D_ADDRESS, 3);
  int timeOut = 0;
  while(Wire.available() < 3) {
    
    if(timeOut > 10) {
      int res = Wire.available();
      Serial.print("result available: ");
      Serial.println(res, DEC);
      break;
    }
    timeOut++;
    delay(10);
  }

  htu21DataResult[0] = Wire.read();
  htu21DataResult[1] = Wire.read();
  //htu21DataResult[2] = Wire.read();         // Check value, I must read the datasheet again :D

  int32_t bitResult = ((int32_t)htu21DataResult[0]) << 8 | htu21DataResult[1];
  
  bitResult &= ~0x0003; // 0xFFFC;

  return bitResult;
}
