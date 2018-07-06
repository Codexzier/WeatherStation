// ========================================================================================
// Description:       Write data to a sd card
// ========================================================================================

// ========================================================================================
// Initialized and check the sd card type. 
// If success, after then can save the measurement data
void SdCardInitSdCard() {

  Serial.println();
  Serial.println("Check SD Card");
  SD.begin();

  if(!mCard.init(SPI_HALF_SPEED, mChipSelect)) {
    Serial.println("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    Serial.println("\t\tERROR: can not init sd card");
    Serial.println("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    return;
  }

  Serial.println("SD Card: Exist");
  
  SdCardShowSDCardType();

  if(!mVolume.init(mCard)) {
    Serial.println("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    Serial.println("\t\tERROR: Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    Serial.println("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    return;
  }

  SdCardShowVolumeSize();

  mSdCardOk = true;
  //SD.begin();
}

// ========================================================================================
void SdCardShowSDCardType() {
  
  Serial.print("\nCard type: ");
  
  switch (mCard.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }
}

// ========================================================================================
// print the type and size of the first FAT-type volume
void SdCardShowVolumeSize() {

  uint32_t volumesize;
  Serial.print("\nVolume type is FAT");
  Serial.println(mVolume.fatType(), DEC);
  Serial.println();

  volumesize = mVolume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= mVolume.clusterCount();       // we'll have a lot of clusters
  volumesize *= 512;                            // SD card blocks are always 512 bytes
  Serial.print("Volume size (bytes): ");
  Serial.println(volumesize);
  Serial.print("Volume size (Kbytes): ");
  volumesize /= 1024;
  Serial.println(volumesize);
  Serial.print("Volume size (Mbytes): ");
  volumesize /= 1024;
  Serial.println(volumesize);

  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  mRoot.openRoot(mVolume);

  // list all files in the card with date and size
  mRoot.ls(LS_R | LS_DATE | LS_SIZE);
}

void SdCardSave() {

  SdCardCreateFileAndTableHeader();
  
  File dataFile = SD.open("weather.csv", FILE_WRITE);

  if(!dataFile) {
    OledPrintTitle(3, "SD ERROR");
    Serial.println("Could not open file");
    return;
  }

  String data =  String(mMeasureCount) + ";";
  mMeasureCount++;
  
  data += String(mTemperaturesArray[mIndex] + mOffsetTemperature) + ";";
  data += String(mHumidity + mOffsetHumidity) + ";";
  data += String(mPressure / 100.0) + ";";
  data += String(mInputValue) + ";";
  mStringMeasurement += data;
  
  dataFile.println(mStringMeasurement);
  dataFile.close();

  Serial.println(mStringMeasurement);

  SdCardShowVolumeSize();
}

void SdCardCreateFileAndTableHeader() {

  if(SD.exists("weather.csv")){
    Serial.println("File exist...");

//    File dataRead = SD.open("weather.csv");
//    Serial.write(dataRead.read());
//    dataRead.close();
    return;    
  }

  mMeasureCount = 0;

  Serial.println("Create weather.csv file...");
  
  File dataFile = SD.open("weather.csv", FILE_WRITE);
  dataFile.println("Nr;Temperature;Humidity;Pressure;Input;");
  dataFile.close();
}

