// ========================================================================================
// Description:       Used a 64x48 Display OLED to show the measure results.
// ========================================================================================

// ========================================================================================
// Setup the base configuration
void OledSetup() {
  mOled.begin(SSD1306_SWITCHCAPVCC, 0x3C);      // set address
  mOled.clearDisplay();                         // Clear the display's memory (gets rid of artifacts)
  mOled.setTextSize(1);                         // set font size
  mOled.setTextColor(WHITE);                    // set color
  mOled.display();                              
}

// ========================================================================================
// Write the parameters to the oled display
// ========================================================================================
// row          = write the text to the target row.
// columnValue  = set the werite position
// text         = write a text, but only place for two chars
// value        = write the value to right side
void OledPrintTitleAndValue(int row, String text, float value) {

  mOled.setCursor(0, row * 10);                       // set position for text info
  mOled.print(text);                            

  mOled.setCursor(GetCursorPosition(value), row * 10);             // set position for value
  mOled.print(value, 3);
}

int GetCursorPosition(float value) {
  if(value >= 1000) {
    return 13;
  }
  if(value >= 100) {
    return 19;
  }
  if(value >= 10) {
    return 25;
  }

  return 31;
}

// ========================================================================================
// render result of temperature results to a diagram view
void OledPrintDiagramResults() {

  int maxHightPixels = 16;                            // max height range for diagram show
  
  float temperatureMax = 0.0;                         // set the max measure of temperature
  float temperatureMin = 100.0;                       // set the min measure of temperature
  
  for (int index = 0; index < 64; index++) {
    if( mTemperaturesArray[index] > temperatureMax) { // set max value
      temperatureMax = mTemperaturesArray[index];
    }
    if(mTemperaturesArray[index] < temperatureMin) {  // set min value
      temperatureMin = mTemperaturesArray[index];
    }
  }

  float diff = temperatureMax - temperatureMin;       // get range
  float scale = (float)maxHightPixels / diff;         // get scale

  for (int index = 0; index < 64; index++) {
    mOled.drawPixel(index, 48 - maxHightPixels - 4, WHITE);
    
    if(index == mIndex) {                             // write a arrow to show actual position
      mOled.drawPixel(index, 48 - maxHightPixels - 1, WHITE);
      mOled.drawPixel(index, 48 - maxHightPixels - 2, WHITE);
      mOled.drawPixel(index, 48 - maxHightPixels - 3, WHITE);
    }
    
    float startHight = scale * (mTemperaturesArray[index] - temperatureMin);
                                                      // scale measurement to pixel height
                                                      
    for(int h = (int)startHight; h < maxHightPixels; h++) {
      mOled.drawPixel(index, 48 - maxHightPixels + h, WHITE);
                                                      // draw pixels
    }
  }
}

