// ========================================================================================
// Description:       read the HMC5883l sensor.
// ========================================================================================

// ========================================================================================
//
void SetupHmc5883l()
{
  Serial.println("Init HMC5883i Sensor");
  
  // Operating Mode (0x02):
  // Continuous-Measurement Mode (0x00)
  // Single-Measurement Mode (0x01)
  Hmc5883lSetConfiguration(0x02, 0x00);
  
  // Standard Skalierung wie aus dem Datenblatt
  Hmc5883lScale(1);
  
  Serial.print("Sacle by ");
  Serial.println(mMagScale, DEC);
}

// Legt die Konfiguration fest, 
// mit dem Einstellungs Byte und
// dem Byte für die Einstellungsbereich.
void Hmc5883lSetConfiguration(byte reg, byte setting)
{
  Wire.beginTransmission(HMC5883l_ADDRESS);
  Wire.write(reg);
  Wire.write(setting);
  Wire.endTransmission();
  Wire.requestFrom(HMC5883l_ADDRESS, 1);
  byte result = Wire.read();
  
  Serial.print("Status: ");
  if(result != 0)
  {
    Serial.println("OK");
  }
  else
  {
    Serial.println("Failur");
  }
}

// Skalierungseinstellung des Sensors
void Hmc5883lScale(int option)
{
  byte setupReg = 0x00;
  switch(option)
  {
    case(0):
    {
      // +- 0.88 Ga, 1370 Gain(LSb/Gauss)
      setupReg = 0x00;
      mMagScale = 0.73;
      break;
    }
    case(1):
    {
      // +- 1.3 Ga, 1090 Gain(LSb/Gauss)
      setupReg = 0x20;
      mMagScale = 0.92;
      break;
    }
    case(2):
    {
      // +- 1.9 Ga, 820 Gain(LSb/Gauss)
      setupReg = 0x40;
      mMagScale = 1.22;
      break;
    }
    case(3):
    {
      // +- 2.5 Ga, 660 Gain(LSb/Gauss)
      setupReg = 0x60;
      mMagScale = 1.52;
      break;
    }
    case(4):
    {
      // +- 4.0 Ga, 440 Gain(LSb/Gauss)
      setupReg = 0x80;
      mMagScale = 2.27;
      break;
    }
    case(5):
    {
      // +- 4.7 Ga, 390 Gain(LSb/Gauss)
      setupReg = 0xA0;
      mMagScale = 2.56;
      break;
    }
    case(6):
    {
      // +- 5.6 Ga, 330 Gain(LSb/Gauss)
      setupReg = 0xC0;
      mMagScale = 3.03;
      break;
    }
    case(7):
    {
      // +- 8.1 Ga, 230 Gain(LSb/Gauss)
      setupReg = 0xC0;
      mMagScale = 4.35;
      break;
    }
    default:{ break; }
  }
  
  // Konfiguration festlegen
  SetConfiguration(0x01, setupReg);
}

// Liest die Messwerte aus dem Magnet Sensor
void Hmc5883lReadCompass()
{
  byte data[6];
  Hmc5883lGetReadData(0x03, 6, &data[0]);
  
  mMagX = (data[0] << 8) | data[1];
  mMagY = (data[2] << 8) | data[3];
  mMagZ = (data[4] << 8) | data[5];
}

void Hmc5883lGetReadData(byte reg, int length, byte data[])
{
  Wire.beginTransmission(HMC5883l_ADDRESS);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(HMC5883l_ADDRESS, length);

  for(int i = 0; i < length; i++)
  {
    data[i] = Wire.read();
  }
}
