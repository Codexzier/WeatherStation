// ========================================================================================
// Description:       Write data to a sd card
// ========================================================================================

// ========================================================================================
// filenames
String mWeather = String("weather.csv");
String mWeatherAverageData = String("weatherAverage.csv");
String mWeatherLastWeek = String("weatherAverageLastWeek");

// ========================================================================================
// Initialized and check the sd card type. 
// If success, after then can save the measurement data
void SdCardInitSdCard() {

  SdCardCreateFileAndTableHeader();
  SdCardCreateFileForWebside();

  Serial.println();
  Serial.println("Check SD Card");
//  if(!SD.begin(mChipSelect)){                             // start sd card for writing and reading
//    Serial.println("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
//    Serial.println("\t\tERROR: can not init sd card");
//    Serial.println("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
//    return;
//  }
//
//  File file = SD.open("weather.csv", FILE_WRITE);
//
//  if(!file) {
//    Serial.println("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
//    Serial.println("\t\tERROR: Could not open card");
//    Serial.println("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
//    
//    return;
//  }

  if(!mCard.init(SPI_HALF_SPEED, mChipSelect)) {          // initialize sd card and check insert sd card
    Serial.println("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    Serial.println("\t\tERROR: can not init sd card");
    Serial.println("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    return;
  }

  if(!mVolume.init(mCard)) {                              // initialize sd card
    Serial.println("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    Serial.println("\t\tERROR: Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    Serial.println("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    return;
  }

  Serial.println("SD Card: Exist and partion was found");
  SdCardShowSDCardType();                                 // Show sd card information
  SdCardShowVolumeSize();                                 // Show sd card storage size informations

  mSdCardOk = true;                                       // set true for after save collected measured data
}

// ========================================================================================
// Read only the type and print to serial
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

  volumesize = mVolume.blocksPerCluster();            // clusters are collections of blocks
  volumesize *= mVolume.clusterCount();               // we'll have a lot of clusters
  volumesize *= 512;                                  // SD card blocks are always 512 bytes
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

  mRoot.ls(LS_R | LS_DATE | LS_SIZE);                 // list all files in the card with date and size
}

// ========================================================================================
// Main method to save data to sd card.
// The method check exist file and create the csv file, if it not exist.
// the result measure data it will going to attached exist measure data.
void SdCardSave() {

  //SdCardCreateFileAndTableHeader();                   // create new file and header informations
  
  File dataFile = SD.open(mWeather, FILE_WRITE);

  if(!dataFile) {                                     // if file not exist
    OledPrintTitle(3, "SD ERROR");
    Serial.println("Could not open file");
    mSdCardOk = false;                                // set next try to save off.
    return;
  }

  String data =  String(mMeasureCount) + ";";         // show the count number after turn on
  mMeasureCount++;
  
  data += String(mTemperaturesArray[mIndex] + mOffsetTemperature) + ";";
  data += String(mHumidity + mOffsetHumidity) + ";";
  data += String(mPressure / 100.0) + ";";
  data += String(mInputUvValue) + ";";
  mStringMeasurement += data;
  
  dataFile.println(mStringMeasurement);
  dataFile.close();

  Serial.println(mStringMeasurement);

  //SdCardShowVolumeSize();                           // Only for debug purpose for look how many space is set
}

// ========================================================================================
// save data for webside
void SdCardSaveWebsiteContent() {
  //SdCardCreateFileForWebside();                       // create new file and header informations
    
    
  // TODO: Not finish. Need to write data to the file and 
  //       need a reading and prepare data for the webside

  File dataFile = SD.open(mWeatherAverageData, FILE_WRITE);

  if(!dataFile) {                                     // if file not exist

    Serial.println("Could not open file");
    mSdCardOk = false;                                // set next try to save off.
    return;
  }

// "Day;Month;Year;Hour;Minute;Temperature;Humidity;Pressure;"
  String data =  String(mActualDateTime[4]) + ";";   // day
  data += String(mActualDateTime[5]) + ";"; // month
  data += String(mActualDateTime[6]) + ";"; // year
  data += String(mActualDateTime[2]) + ";"; // hour
  data += String(mActualDateTime[1]) + ";"; // minute
  data += String(mAverageTemperature) + ";"; // temperature
  data += String(mAverageHumidity) + ";"; // humidity
  data += String(mAveragePressure); // presssure
}

// ========================================================================================
// If the target file not exist, then it create new csv file with csv header information.
void SdCardCreateFileAndTableHeader() {

  if(SD.exists(mWeather)){
    
    Serial.println("File weather.csv exist...");
    return;    
  }

  mMeasureCount = 0;                                  // return counting to zero

  Serial.println("Create weather.csv file...");
  
  File dataFile = SD.open(mWeather, FILE_WRITE);
  dataFile.println("Nr;Temperature;Humidity;Pressure;Input;");
  dataFile.close();
}

// ========================================================================================
// this file will contain average values for the webside call.
void SdCardCreateFileForWebside(){

  if(!SD.exists(mWeatherAverageData)) {
    
    Serial.println("Create weaterAverage.csv file...");

    File dataFile = SD.open(mWeatherAverageData, FILE_WRITE);
    dataFile.println("Day;Month;Year;Hour;Minute;Temperature;Humidity;Pressure;");
    dataFile.close(); 
  }

  Serial.println("File weaterAverage.csv exist...");

  if(SD.exists(mWeatherLastWeek)) {
    Serial.println("File weaterAverageLastWeek.csv exist...");
    return;    
  }

  Serial.println("Create weaterAverageLastWeek.csv file...");

  File dataFile = SD.open(mWeatherLastWeek, FILE_WRITE);
  dataFile.println("Day;Month;Year;Hour;Minute;Temperature;Humidity;Pressure;");
  dataFile.close(); 
}
