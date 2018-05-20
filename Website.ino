// ========================================================================================
// Description:       Used connect to wlan to get information on webbrowser
// ========================================================================================

// ========================================================================================
// client received

WiFiClient mWebclient;
boolean mIsConnected = false;

// ========================================================================================
// start webserver and print information on serial
// ========================================================================================
void StartWebserver() {

  Serial.print(F("Setting static ip to : "));
  Serial.println(mIp);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(mSsid);
  IPAddress subnet(255, 255, 255, 0);                   // set subnet mask to match your network
  WiFi.config(mIp, mGateway,subnet); 
  WiFi.begin(mSsid, mPassword);

  int waitConnectingCount = 0;
  mOled.clearDisplay();
  OledPrintTitle(0, "Connecting to WLAN");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);

    mOled.clearDisplay();
    OledPrintTitle(0, "Connecting");
    OledPrintTitle(1, "to WLAN");
    OledPrintTitleAndValue(2, "wait: ", waitConnectingCount);
    mOled.display(); 
    Serial.print(".");
    
    if(waitConnectingCount >= 10) {
      
      mOled.clearDisplay();
      OledPrintTitle(0, "Connecting Fail");
      mOled.display(); 
      delay(500);
      break;
    }
    
    waitConnectingCount++;
  }

  if(waitConnectingCount >= 10) {
    
    mOled.clearDisplay();
    OledPrintTitle(0, "No WiFi");
    mOled.display(); 
    delay(1000);
    return;
  }

  mIsConnected = true;

  mOled.clearDisplay();
  OledPrintTitle(0, "WiFi connected");
  mOled.display(); 
  delay(500);
    
  Serial.println();
  Serial.println("WiFi connected");
 
  mServer.begin();
  Serial.println("Server started");
 
  Serial.print("Use this URL : http://");               // Print the IP address
  Serial.print(WiFi.localIP());
  Serial.println("/");
}

// ========================================================================================
// Wait for some send data for show data on a website.
// ========================================================================================
// timeout = set max time to wait for incoming data.
void PrintOnWebsite(int timeout) {

  if(!mIsConnected) {
    return;
  }
  
  mWebclient = mServer.available();
  if (!mWebclient) {
    return;
  }
  if (!WaitClientSendsData(timeout)) {
    return;
  }

  mOled.clearDisplay();
  OledPrintTitle(0, "Running");
  OledPrintTitle(1, "web access");
  mOled.display(); 
  
  mWebclient.println("HTTP/1.1 200 OK");
  mWebclient.println("Content-Type: text/html");
  mWebclient.println("");                               //  do not forget this one
  mWebclient.println("<!DOCTYPE HTML>");
  mWebclient.println("<html>");

  PrintHtmlHead();
  PrintHtmlBody();
 
  mWebclient.println("</html>");
}

void PrintHtmlHead() {

  mWebclient.println("<head>");
  
  mWebclient.println("<title>");
  mWebclient.println("Weatherstation with wemos");
  mWebclient.println("</title>");
  
  mWebclient.println("<style>");
  mWebclient.println("h1 { color:green; font-size:30px; }");
  mWebclient.println(".divTable{ display: table;width: 100%;}");
  mWebclient.println(".divTableRow {display: table-row;}");
  mWebclient.println(".divTableHeading { background-color: #EEE;display: table-header-group; }");
  mWebclient.println(".divTableCell, .divTableHead { border: 1px solid #999999;display: table-cell;padding: 3px 10px;}");
  mWebclient.println(".divTableCellLeft {width: 150px;border: 1px solid #999999;display: table-cell;padding: 3px 10px; }");
  mWebclient.println(".divTableHeading {background-color: #EEE; display: table-header-group;font-weight: bold; }");
  mWebclient.println(".divTableFoot {background-color: #EEE;display: table-footer-group;font-weight: bold; }");
  mWebclient.println(".divTableBody {display: table-row-group; }");
  mWebclient.println("</style>");
  
  mWebclient.println("</head>");
}

void PrintHtmlBody() {
  
  mWebclient.println("<body>");
  
  mWebclient.println(" <h1>Weatherstation</h1>");
  
  mWebclient.println("  <div class=\"divTable\" style=\"border: 1px solid rgb(95, 95, 95);\" >");
  mWebclient.println("   <div class=\"divTableBody\">");
  
  mWebclient.println("    <div class=\"divTableRow\">");
  mWebclient.println("     <div class=\"divTableCellLeft\">Temperatur</div>");
  mWebclient.println("     <div class=\"divTableCell\">");
  mWebclient.print(mTemperaturesArray[mIndex] + mOffsetTemperature);
  mWebclient.println(" &deg;C");
  mWebclient.println("     </div>");
  mWebclient.println("    </div>");

  mWebclient.println("    <div class=\"divTableRow\">");
  mWebclient.println("     <div class=\"divTableCellLeft\">Humidity</div>");
  mWebclient.println("     <div class=\"divTableCell\">");
  mWebclient.print(mHumidity + mOffsetHumidity);
  mWebclient.println(" %");
  mWebclient.println("     </div>");
  mWebclient.println("    </div>");

  mWebclient.println("    <div class=\"divTableRow\">");
  mWebclient.println("     <div class=\"divTableCellLeft\">Pressure</div>");
  mWebclient.println("     <div class=\"divTableCell\">");
  mWebclient.print(mPressure / 100.0);
  mWebclient.println(" hPa");
  mWebclient.println("     </div>");
  mWebclient.println("    </div>");

  mWebclient.println("    <div class=\"divTableRow\">");
  mWebclient.println("     <div class=\"divTableCellLeft\">Ground humidity</div>");
  mWebclient.println("     <div class=\"divTableCell\">");
  mWebclient.print(mInputValue);
  mWebclient.print(", Min: ");
  mWebclient.print(mInputValueMin);
  mWebclient.print(", Max: ");
  mWebclient.print(mInputValueMax);
  mWebclient.println(" ");
  mWebclient.println("     </div>");
  mWebclient.println("    </div>");
  
  mWebclient.println("   </div>");
  mWebclient.println("  </div>");
  
  mWebclient.println("</body>");
}

// ========================================================================================
// Waiting for sending data from client.
// ========================================================================================
// timeout = set max time to wait for incoming data.
boolean WaitClientSendsData(int timeout) {

  Serial.print(" wait.. ");
  
  int timeOutCount = 0;
  while(!mWebclient.available()){

    timeOutCount++;
    
    if(timeOutCount >= timeout) {
      Serial.println("false");
      return false;
    }
    delay(1);
  }

  Serial.println("true");
  return true;
}

