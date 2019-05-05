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
    
    if(waitConnectingCount >= 5) {
      
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
void PrintOnWebsite(int timeout, bool getResourceFromInternet) {

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

  if(getResourceFromInternet) {
    printWebsideDashboard();
  }
  else {
    PrintHtmlHead();
    PrintHtmlBody();
  }
  
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

void printWebsideDashboard(){
    printWebsideDashboardHead();
    printWebsideDashboardBodyPart1();
    printWebsideDashboardBodyPart2();
}

void printWebsideDashboardHead(){

  //mWebclient.println("<!doctype html>");
  //mWebclient.println("<html lang=\"en\">");
  mWebclient.println("<head>");
  mWebclient.println("<meta charset=\"utf-8\">");
  mWebclient.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\">");
  mWebclient.println("<meta name=\"description\" content=\"Meine Welt in meinem Kopf - Wemos Weatherstation\">");
  mWebclient.println("<meta name=\"author\" content=\"Johannes Paul Langner\">");
  mWebclient.println("<title>Weatherstation</title>");

  mWebclient.println("<link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css\" integrity=\"sha384-ggOyR0iXCbMQv3Xipma34MD+dH/1fQ784/j6cY/iJTQUOhcWr7x9JvoRxT2MZw1T\" crossorigin=\"anonymous\">");
  mWebclient.println("<style>");
  mWebclient.println(".border-cell { margin: 1px 5px 1px 2px;}");
  mWebclient.println("</style>");
  mWebclient.println("</head>");
}

void printWebsideDashboardBodyPart1(){
  
  mWebclient.println("<body>");
  mWebclient.println("<div class=\"container-fluid\">");
  mWebclient.println("<div class=\"row\">");
  
  mWebclient.println("<main role=\"main\" class=\"col-lg-10\">");
  
  mWebclient.println("<div class=\"d-flex justify-content-between flex-wrap flex-md-nowrap align-items-center pt-3 pb-2 mb-3 border-bottom\">");
  mWebclient.println("<h1 class=\"h2\">Dashboard</h1>");
  mWebclient.println("<h5 id=\"countMeasurements\">Count of measrements: 123</h5>");
  mWebclient.println("</div>");
  
  mWebclient.println("<div class=\"container\">");
  mWebclient.println("<div class=\"card-deck mb-3 text-center\">");
  mWebclient.println("<div class=\"card mb-4 box-shadow\">");
  mWebclient.println("<div class=\"card-header\">");
  mWebclient.println("<h5 class=\"my-0 font-weight-normal\">last 24h</h5>");
  mWebclient.println("</div>");
  mWebclient.println("<div class=\"card-body\">");
  mWebclient.println("<canvas class=\"my-4 w-100\" id=\"myChart\" width=\"1000\" height=\"300\"></canvas>");
  mWebclient.println("</div>");
  mWebclient.println("</div>");
  mWebclient.println("</div>");
  mWebclient.println("<div class=\"card-deck mb-3 text-center\">");
  mWebclient.println("<div class=\"card mb-4 box-shadow\">");
  mWebclient.println("<div class=\"card-header\">");
  mWebclient.println("<h5 class=\"my-0 font-weight-normal\">last week</h5>");
  mWebclient.println("</div>");
  mWebclient.println("<div class=\"card-body\">");
  
  mWebclient.println("<canvas id=\"myChart2\" width=\"500\" height=\"160\"></canvas>");
  
  mWebclient.println("</div>");
  mWebclient.println("</div>");
  mWebclient.println("<div class=\"card mb-4 box-shadow\">");
  mWebclient.println("<div class=\"card-header\">");
  mWebclient.println("<h5 class=\"my-0 font-weight-normal\">actual measurement</h5>");
  mWebclient.println("</div>");
  mWebclient.println("<div class=\"card-body\">");

  mWebclient.println("<div class=\"d-flex\">");
  mWebclient.println("<div class=\"border-cell\">Date time</div>");
  mWebclient.println("<div class=\"border-cell\">");

  mWebclient.print(mActualDateTime[2]);               // hour
  mWebclient.print(":");
  mWebclient.print(mActualDateTime[1]);               // minute
  mWebclient.print(":");
  mWebclient.print(mActualDateTime[0]);               // second
  mWebclient.print(", ");
  mWebclient.print(mDaysOfWeek[mActualDateTime[3]]);  // day
  mWebclient.print(", ");
  mWebclient.print(mActualDateTime[4]);               // day of the month
  mWebclient.print(".");
  mWebclient.print(mActualDateTime[5]);               // month
  mWebclient.print(".");
  mWebclient.print(mActualDateTime[6], DEC);               // year
  
  mWebclient.println("");
  mWebclient.println("</div>");
  mWebclient.println("</div>");
  
  mWebclient.println("<div class=\"d-flex\">");
  mWebclient.println("<div class=\"border-cell\">Temperature</div>");
  mWebclient.println("<div class=\"border-cell\">");
  mWebclient.print(mTemperaturesArray[mIndex] + mOffsetTemperature);
  mWebclient.println(" &deg;C");
  mWebclient.println("</div>");
  mWebclient.println("</div>");
  
  mWebclient.println("<div class=\"d-flex\">");
  mWebclient.println("<div class=\"border-cell\">Humidty</div>");
  mWebclient.println("<div class=\"border-cell\">");
  mWebclient.print(mHumidity + mOffsetHumidity);
  mWebclient.println(" %");
  mWebclient.println("</div>");
  mWebclient.println("</div>");
  
  mWebclient.println("<div class=\"d-flex\">");
  mWebclient.println("<div class=\"border-cell\">Pressure </div>");
  mWebclient.println("<div class=\"border-cell\">");
  mWebclient.print(mPressure / 100.0);
  mWebclient.println(" hPa");
  mWebclient.println("</div>");
  mWebclient.println("</div>");

  mWebclient.println("<div class=\"d-flex\">");
  mWebclient.println("<div class=\"border-cell\">Altitude </div>");
  mWebclient.println("<div class=\"border-cell\">");
  mWebclient.print(mAltitude);
  mWebclient.println(" m");
  mWebclient.println("</div>");
  mWebclient.println("</div>");
  
  mWebclient.println("</div>");
  mWebclient.println("</div>");
  mWebclient.println("</div>");
  mWebclient.println("</div>");
  
  mWebclient.println("</main>");
  mWebclient.println("</div>");
  mWebclient.println("</div>");
}

void printWebsideDashboardBodyPart2() {
  mWebclient.println("<script src=\"https://cdnjs.cloudflare.com/ajax/libs/feather-icons/4.9.0/feather.min.js\"></script>");
  mWebclient.println("<script src=\"https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.7.3/Chart.min.js\"></script>");
  mWebclient.println("<script>");
  mWebclient.println("(function () { feather.replace()");
  // Graphs
  mWebclient.println("var ctx = document.getElementById('myChart')");
  // eslint-disable-next-line no-unused-vars
  mWebclient.println("var myChart = new Chart(ctx, {");
  mWebclient.println("type: 'line',");
  mWebclient.println("data: {");
  mWebclient.println("labels: [");
  
  int startHour = 0;
  for(int hourIndex = 0; hourIndex < 23; hourIndex++) {
    mWebclient.print("'");
    mWebclient.print(startHour, DEC);
    mWebclient.print("',");
    startHour++;
  }
  mWebclient.print("'");
  mWebclient.print(startHour, DEC);
  mWebclient.println("'");
  
  mWebclient.println("],");
  mWebclient.println("datasets: [{");
  mWebclient.println("data: [");
  
  int startValue = 0;
  for(int valueIndex = 0; valueIndex < 23; valueIndex++) {
    // TODO: hier wird der Wert eingetragen
    mWebclient.print(startValue, DEC);
    mWebclient.print(",");
    startValue++;
  }
  mWebclient.print(startValue, DEC);
  
  mWebclient.println("],");
  mWebclient.println("lineTension: 0,");
  mWebclient.println("backgroundColor: 'transparent',");
  mWebclient.println("borderColor: '#007bff',");
  mWebclient.println("borderWidth: 4,");
  mWebclient.println("pointBackgroundColor: '#007bff'");
  mWebclient.println("}]}, options: { scales: { yAxes: [{ ticks: { beginAtZero: false } }] }, legend: { display: false }}})");
  
  mWebclient.println("var ctx2 = document.getElementById('myChart2')");
  // eslint-disable-next-line no-unused-vars
  mWebclient.println("var myChart2 = new Chart(ctx2, {");
  mWebclient.println("type: 'line',");
  mWebclient.println("data: {");
  mWebclient.print("labels: [");
  
  int startDay = 0;
  for(int index = 0; index < 6; index++){
    SetDay(startDay);
    mWebclient.print(",");
    startDay++;
  }
  SetDay(startDay);
  
  mWebclient.print("], datasets: [{ data: [");
  
  int startDayValue = 0;
  for(int index = 0; index < 6; index++){
    mWebclient.print(startDayValue, DEC);
    mWebclient.print(",");
    startDayValue++;
  }
  mWebclient.print(startDayValue, DEC);
  
  mWebclient.println("], lineTension: 0, backgroundColor: 'transparent', borderColor: '#007bff', borderWidth: 4, pointBackgroundColor: '#007bff' }] }, ");
  mWebclient.println(" options: { scales: { yAxes: [{ ticks: { beginAtZero: false } }] }, legend: { display: false } } }) }())");
  mWebclient.println("</script>");
  mWebclient.println("</body>");
  //mWebclient.println("</html>");
}

void SetDay(int index) {

  if(index > 7) {
    index -= 7;
  }
  
  switch(index) {
    case 1:{ mWebclient.print("'Mo'"); break; }
    case 2:{ mWebclient.print("'Di'"); break; }
    case 3:{ mWebclient.print("'Mi'"); break; }
    case 4:{ mWebclient.print("'Do'"); break; }
    case 5:{ mWebclient.print("'Fr'"); break; }
    case 6:{ mWebclient.print("'Sa'"); break; }
    case 7:{ mWebclient.print("'Su'"); break; }
    default: { break; }
  }
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
