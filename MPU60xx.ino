// ========================================================================================
// Description:       read the MPU60xx sensor.
// ========================================================================================

// ========================================================================================
// setup the acceleation and gyroscope sensor with filter.
void SetupMpuAccelerationAndGyroscopeSensor() {

  // Powermanagement aufrufen
  // Sensor schlafen und Reset, Clock wird zunächst von Gyroskopeachse Z verwendet  
  Serial.println("Powermanagement aufrufen - Reset");
  SetConfiguration(0x6B, 0x80);
  
  // Kurz warten
  delay(1000);
  
  // Powermanagement aufrufen
  // Sleep beenden und Clock von Gyroskopeachse X verwenden
  Serial.println("Powermanagement aufrufen - Clock festlegen");
  SetConfiguration(0x6B, 0x03);
  
  delay(1000);
  
  // Konfigruation  aufrufen
  // Default => Acc=260Hz, Delay=0ms, Gyro=256Hz, Delay=0.98ms, Fs=8kHz
  Serial.println("Konfiguration aufrufen - Default Acc = 260Hz, Delay = 0ms");
  SetConfiguration(0x1A, 0x00);
  
  delay(1000);
  
  // Gyroskope Einstellung festelgen
  // Stellt den Gyrosope auf 250°/s ein
  Serial.println("Gyroskope Einstellung aufrufen - 250°/s");
  SetConfiguration(0x1B, 0xE7);
  
  delay(1000);
  
  // Beschleunigungssensor Einstellungen festlegen
  // Stellt den Acceleration auf 2g ein
  Serial.println("Beschleunigungs Einstellung aufrufen - 2g");
  SetConfiguration(0x1B, 0xE7);
  
  delay(1000);
  
  // Test Verbindung
  // Wer bin ich 
  Serial.println("Wer bin ich, aufrufen - Teste Verbindung");
  SetConfiguration(0x1B, 0x00);

  Serial.println("");
}

void SetConfiguration(byte reg, byte setting)
{
  Wire.beginTransmission(MPU60xx_ADDRESS);
  Wire.write(reg);
  Wire.write(setting);
  Wire.endTransmission();
  delay(100);
  Wire.requestFrom(MPU60xx_ADDRESS, 2);
  byte result = Wire.read();
   
  if(result != 0)
  {
    Serial.println("Status: OK");
  }
  else
  {
    Serial.println("Status: Failur");
  }
}

void ReadAccelerationAndGyroscope()
{
  byte result[14];
  
  // Anfangs Adresse von Beschleunigungssensorachse X
  result[0] = 0x3B;
  
   // Aufruf des MPU6050 Sensor
  Wire.beginTransmission(MPU60xx_ADDRESS);
  // Anfangsadresse verwenden.
  Wire.write(result[0]);
  Wire.endTransmission();
  // 14 Bytes kommen als Antwort
  Wire.requestFrom(MPU60xx_ADDRESS, 14);
  // Bytes im Array ablegen
  for(int i = 0; i < 14; i++)
  {
    result[i] = Wire.read();
  }
  
  // Zwei Bytes ergeben eine Achsen Wert und könenn per Bit Shifting zusammengelegt werden.
  
  // Beschleunigungssensor
  mAccX = (((unsigned int)result[0]) << 8) | result[1];
  mAccY = (((unsigned int)result[2]) << 8) | result[3];
  mAccZ = (((unsigned int)result[4]) << 8) | result[5];
  
  // Temperatur sensor
  unsigned int temp = (((unsigned int)result[6]) << 8) | result[7];
  
  // Gyroskopesensor
  mGyroX = (((unsigned int)result[8]) << 8) | result[9];
  mGyroY = (((unsigned int)result[10]) << 8) | result[11];
  mGyroZ = (((unsigned int)result[12]) << 8) | result[13];
}
