// ========================================================================================
// Description:       read the ML8511 sensor.
// ========================================================================================

// ========================================================================================
// read the one pin for analog signal
void ReadAnalogInput(){
  
  mInputUvValue = analogRead(mPinInputUv);

  if(mInputUvValue > mInputUvValueMax) {
    mInputUvValueMax = mInputUvValue;
  }

  if(mInputUvValue < mInputUvValueMin) {
    mInputUvValueMin = mInputUvValue;
  }
}
