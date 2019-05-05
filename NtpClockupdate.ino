// ========================================================================================
// Description:       Used to update the system time.
// Important:         http://arduino.cc/forum/index.php/topic,52018.0.html
//                    - about not bugging the NTP-server admins.
// Information:       http://playground.arduino.cc/Main/DS1307OfTheLogshieldByMeansOfNTP
// ========================================================================================

// ========================================================================================
// prepare deciaml value to binar code for the rtc modul
// ========================================================================================
// val = decimal value
byte DecimalToBinar(byte val){
  return( (val/10*16) + (val%10) );
}
// ========================================================================================
// Convert from rtc binar code to Decimal
// ========================================================================================
// val = decimal value
byte BinarToDecimal(byte val){
  return( (val/16*10) + (val%16) );
}

void ReadBinarDs1307(){
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_I2C_ADDRESS, 7);

  byte result[7];
  for(int index = 0; index < 7; index++) {
    result[index] = Wire.read();

    //Serial.print(result[index], DEC);
  }
  //Serial.println();

  mActualDateTime[0] = BinarToDecimal(result[0] & 0x7f);
  mActualDateTime[1] = BinarToDecimal(result[1]);
  mActualDateTime[2] = BinarToDecimal(result[2] & 0x3f);
  mActualDateTime[3] = BinarToDecimal(result[3]);
  mActualDateTime[4] = BinarToDecimal(result[4]);
  mActualDateTime[5] = BinarToDecimal(result[5]);
  mActualDateTime[6] = BinarToDecimal(result[6]);

  mActualDay = mActualDateTime[4];
  mActualMonth = mActualDateTime[5];
  mActualYear = mActualDateTime[6];
  mActualHour = mActualDateTime[2];
}

// ========================================================================================
// Setup the clock to the dataloggershield with rtc
// ========================================================================================
// minute       = set the actual minute
// hour         = set the actual hour
// dayOfWeek    = set the day of the week (begin with Sunday = 1)
// dayOfMonth   = set the actual day of the month
// month        = set the actual month
// year         = set the actual year
void SetupDS1307(byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year){
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0);
  Wire.write(0); // setzt die Sekunden
  Wire.write(DecimalToBinar(minute)); // setzt die Minuten
  Wire.write(DecimalToBinar(hour)); // setzt die Stunden
  Wire.write(DecimalToBinar(dayOfWeek)); // setzt den Wert für den Tag der Woche
  Wire.write(DecimalToBinar(dayOfMonth)); // setzt den Wert für den Tag im Monat
  Wire.write(DecimalToBinar(month)); // setzt den Monat
  Wire.write(DecimalToBinar(year)); // setzt das Jahr
  Wire.endTransmission();
}



// ========================================================================================
// get the time from the dataloggershield with rtc
// ========================================================================================
// secound      = get the second
// minute       = get the actual minute
// hour         = get the actual hour
// dayOfWeek    = get the day of the week (begin with Sunday = 1)
// dayOfMonth   = get the actual day of the month
// month        = get the actual month
// year         = get the actual year
void ReadDS1307(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year){

  ReadBinarDs1307();

  *second = mActualDateTime[0];
  *minute = mActualDateTime[1];
  *hour = mActualDateTime[2];
  *dayOfWeek = mActualDateTime[3];
  *dayOfMonth = mActualDateTime[4];
  *month = mActualDateTime[5];
  *year = mActualDateTime[6];
}
