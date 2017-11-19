// ========================================================================================
// Description:       orgin source come from bmp085
//                    so a check later for must changes or any problems
// ========================================================================================

// ========================================================================================
// Calibration values
int16_t ac1, ac2, ac3, b1, b2, mb, mc, md;
uint16_t ac4, ac5, ac6;
long b5;

// ========================================================================================
// Read the measure of pressure and temperature. 
// The results used to calculation 
// ========================================================================================
// printOn = True to take on of detail output information.
void ReadBmp180Sensor(bool printOn) {

  if(printOn) {
    Serial.println("Read BMP180 sensor:");
  }
  
  uint16_t rawValueUt = bmp085ReadUT();                     // get the raw temperature value
                                                            // MUST be first
  float temperature = bmp085GetTemperature(rawValueUt);     // calculate to final temperature value
  uint16_t rawValueUp = bmp085ReadUP();                     // get the raw pressure value
  mPressure = bmp085GetPressure(rawValueUp);                // calculate to final pressure value
  mAtmosphere = mPressure / 101325;                          // calculate "standard atmosphere"
                                                            // 101325 Pa is an base value of atmosphere
  mAltitude = calcAltitude(mPressure);                       // calculate altitude in meters
  mTemperatures[0] = temperature;                           // set temperature result to global variable

  if(printOn) {
    Serial.print("\tTemperature: "); Serial.print(temperature, 1); Serial.println(" deg C");
    Serial.print("\tPressure: "); Serial.print(mPressure, 0); Serial.println(" Pa");
    Serial.print("\tStandard Atmosphere: "); Serial.println(mAtmosphere, 4);
    Serial.print("\tAltitude: "); Serial.print(mAltitude, 2); Serial.println(" M");
    Serial.println();
  }
}

// ========================================================================================
// read all needed calibration values
void bmp085Calibration() {
  ac1 = bmp085ReadInt(0xAA); 
  ac2 = bmp085ReadInt(0xAC);
  ac3 = bmp085ReadInt(0xAE); 
  ac4 = bmp085ReadInt(0xB0); 
  ac5 = bmp085ReadInt(0xB2); 
  ac6 = bmp085ReadInt(0xB4);
  b1 = bmp085ReadInt(0xB6); 
  b2 = bmp085ReadInt(0xB8); 
  mb = bmp085ReadInt(0xBA); 
  mc = bmp085ReadInt(0xBC); 
  md = bmp085ReadInt(0xBE); 
}

// ========================================================================================
// Calculate temperature in deg C
float bmp085GetTemperature(uint16_t ut){
  long x1 = (((long)ut - (long)ac6)*(long)ac5) >> 15;
  long x2 = ((long)mc << 11)/(x1 + md);
  b5 = x1 + x2;

  return (float)((b5 + 8)>>4) / 10.0;
}

// ========================================================================================
// calculate pressure from raw value
long bmp085GetPressure(unsigned long up){

  long b6 = b5 - 4000;
  // Calculate B3
  long x1 = (b2 * (b6 * b6)>>12)>>11;
  long x2 = (ac2 * b6)>>11;
  long x3 = x1 + x2;
  long b3 = (((((long)ac1)*4 + x3)<<OSS) + 2)>>2;

  // Calculate B4
  x1 = (ac3 * b6)>>13;
  x2 = (b1 * ((b6 * b6)>>12))>>16;
  x3 = ((x1 + x2) + 2)>>2;
  unsigned long b4 = (ac4 * (unsigned long)(x3 + 32768))>>15;
  
  unsigned long b7 = ((unsigned long)(up - b3) * (50000>>OSS));

  long p;
  if (b7 < 0x80000000)
    p = (b7<<1) / b4;
  else
    p = (b7 / b4)<<1;

  x1 = (p>>8) * (p>>8);
  x1 = (x1 * 3038)>>16;
  x2 = (-7357 * p)>>16;
  p += (x1 + x2 + 3791)>>4;

  return p;
}

// ========================================================================================
// Read 1 byte from the BMP085 at 'address'
char bmp085Read(byte address)
{
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();
  Wire.requestFrom(BMP085_ADDRESS, 1);
  while(!Wire.available()) {}
  return Wire.read();
}

// ========================================================================================
// read two byte 
int16_t bmp085ReadInt(byte address)
{
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();

  Wire.requestFrom(BMP085_ADDRESS, 2);

  while(Wire.available()<2) {
  }

  unsigned char msb = Wire.read();
  unsigned char lsb = Wire.read();
  return (int16_t) msb << 8 | lsb;
}

// ========================================================================================
// read temperature raw value
uint16_t bmp085ReadUT(){
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x2E);
  Wire.endTransmission();
  delay(5); // Wait at least 4.5ms
  
  return bmp085ReadInt(0xF6);
}

// ========================================================================================
// Read the uncompensated pressure value
unsigned long bmp085ReadUP(){
  // Write 0x34+(OSS<<6) into register 0xF4
  // Request a pressure reading w/ oversampling setting
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x34 + (OSS<<6));
  Wire.endTransmission();

  // Wait for conversion, delay time dependent on OSS
  delay(2 + (3<<OSS));

  // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
  byte msb = bmp085Read(0xF6);
  byte lsb = bmp085Read(0xF7);
  byte xlsb = bmp085Read(0xF8);
  unsigned long up = (((unsigned long) msb << 16) | ((unsigned long) lsb << 8) | (unsigned long) xlsb) >> (8-OSS);
  
  return up;
}

// ========================================================================================
// calculate altitude by pressure value
float calcAltitude(float pressure){
  float alti = pow((pressure/101325),(1/5.25588));
  return (1 - alti) /0.0000225577;
}
