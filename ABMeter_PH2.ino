#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <DHT.h>
#include <ESPAsyncTCP.h>
#include <AsyncElegantOTA.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>

WiFiClient wifiClient;


#define SensorPin A0

const char* WiFiSSID = "WiFiSSID";
const char* WiFiPassword = "WiFiPassword";

const char* ssidd = "Autobud_DataCollector";
const char* passwordd = "Grow4life";
bool wifisetup = false;
int localonly = 0;
//String ID = "03";
//const char* serverName = "http://192.168.0.204:1880/data";
unsigned long lastTime = 0;

//Fires every hour
unsigned long timerDelay = 1800000;
//unsigned long timerDelay = 30000;

//NEW PERAMS
const char* PARAM_COLFRQ = "colfrq";
const char* PARAM_POST = "PARAM_POST";
const char* PARAM_ABADDESS = "abaddress";
const char* PARAM_DEVID = "devid";


float sensorValue = 0.0;
float ph_act = 0.0;
float ph7 = 0;
float ph4 = 0;
float volt = 0;
int phval = 0;
float slope = 0; 
unsigned long int avgval; 
int buffer_arr[10],temp;

int TakeData = 0;
String mac = "";
String disPH = "";
String offsetdis = "";
String ph7voltage = "";
String ph4voltage = "";


AsyncWebServer server(80);

void localWifi ( void ){
    Serial.println("Local WiFi");
    WiFi.softAP(ssidd, passwordd) ;
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    server.begin();
    localonly = 1;
    wifisetup = true;
}

//NEW HTML
const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE HTML><html><head>
  <title>AutoBudMeter</title>

<style type="text/css">
            h2 {
                font-family: courier;
                font-size: 20pt;
                color: black;
                border-bottom: 2px solid blue;
            }
            p {
                font-family: arial, verdana, sans-serif;
                font-size: 12pt;
                color: grey;
            }
            .red_txt {
                color: red;
            }
        </style>
  
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <h2>ABdc_PH</h2>
  <p><i>ABdc_PH V1.11</i></p>
  <script>
    function submitMessage() {
      alert("Value Saved");
      setTimeout(function(){ document.location.reload(false); }, 500);   
    }
  </script></head><body>
    <p><b>Control</b></p>
    <button onclick="UpdateNow();">Update Now</button>
    </form><br>
  <form action="/get" target="hidden-form">
    Data Collection Freq: (Current: Every %colfrq% Minutes): <input type="number " name="colfrq">
    <input type="submit" value="Submit" onclick="submitMessage()">
  </form><br>
  <form action="/get" target="hidden-form">
    API Address: (Current: %abaddress%): <input type="number " name="abaddress">
    <input type="submit" value="Submit" onclick="submitMessage()">
  </form><br>
    </form><br>

  <p><b>Current Data</b></p>
   </form>
   Current PH: %ph_act% 
   </form><br>
   Device MAC: %mac%
   </form><br></p>
   DeviceID: %devid%
   </form><br></p>
   

   <p><b>Calibration</b></p>
   <button onclick="Pull7();">Calibrate PH7</button>
    </form><br>
    <button onclick="Pull4();">Calibrate PH4</button>
    </form><br>
   <form action="/get" target="hidden-form">
   Device ID: (Current: %devid%): <input type="text" name="devid">
   <input type="submit" value="Submit" onclick="submitMessage()">
   </form><br>
    
   </b></p>Wireless Configuration</b></p>
   <form action="/get" target="hidden-form">
    WiFiSSID (%WiFiSSID%): <input type="text" name="WiFiSSID">
    <input type="submit" value="Submit" onclick="submitMessage()">
   </form><br>
   <form action="/get" target="hidden-form">
    WiFiPassword (%WiFiPassword%): <input type="text" name="WiFiPassword">
    <input type="submit" value="Submit" onclick="submitMessage()">
   </form><br>
   
    
  </div>

</body>
<script>
  function UpdateNow() {
    var phr = new XMLHttpRequest();
    phr.open('GET', "/UpdateNow", true);
    phr.send();
  }
  
 function Pull4() {
    var phr = new XMLHttpRequest();
    phr.open('GET', "/Pull4", true);
    phr.send();
  }

 function CalibrateSensor() {
    var phr = new XMLHttpRequest();
    phr.open('GET', "/CalibrateSensor", true);
    phr.send();
  } 

function Pull7() {
    var phr = new XMLHttpRequest();
    phr.open('GET', "/Pull7", true);
    phr.send();
  } 
  
  
 </script> 
 </html>)rawliteral";


 //SET UP SPIFFS
 String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- empty file or failed to open file");
    return String();
  }
  Serial.println("- read from file:");
  String fileContent;
  while(file.available()){
    fileContent+=String((char)file.read());
  }
  Serial.println(fileContent);
  return fileContent;
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
}

String processor(const String& var){
  //Serial.println(var);
   if(var == "ph_act"){
    return String(disPH);
  }
   else if(var == "colfrq"){
    return readFile(SPIFFS, "/colfrq");
  }
 else if(var == "mac"){
    return String(mac);
  }
 else if(var == "WiFiSSID"){
    return readFile(SPIFFS, "/ssid.txt");
  }
  else if(var == "WiFiPassword"){
    return readFile(SPIFFS, "/wifipasswd.txt");
  }
  else if(var == "abaddress"){
    return readFile(SPIFFS, "/abaddress.txt");
  }
  else if(var == "devid"){
    return readFile(SPIFFS, "/id.txt");   
  }
  else if(var == "ph7offset"){
    return readFile(SPIFFS, "/ph7offset.txt");   
  }

  

  return String();
}


void setup() {
  Serial.begin(115200);

  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");  
    
    
    ESP.restart();
  }
  else {
    delay(500);
    Serial.println("SPIFFS mounted successfully");
  }


//New WiFi
//wifi
Serial.print(wifisetup);
//// pull saved value from SPIFFs
String savedssid = readFile(SPIFFS, "/ssid.txt");
String savedpass = readFile(SPIFFS, "/wifipasswd.txt");
int ssidlength = savedssid.length();
Serial.print("SSID Length:");
Serial.print(ssidlength);


//if the SSID is blank, then revert to local mode
if (ssidlength == 0) {
  wifisetup == true;
  localWifi();
}


 //If Wifi config was pressed, skip this, if not, run:
 if (wifisetup == false) {
  // String savedssid = readFile(SPIFFS, "/ssid.txt");
 //  String savedpass = readFile(SPIFFS, "/wifipasswd.txt");
   WiFi.mode(WIFI_STA);
   Serial.print("Using SSID ");
   Serial.println(savedssid);
   Serial.print("Using Password ");
   Serial.println(savedpass);
   //char hname[19];
   //snprintf(hname, 12, "ESP%d-LIGHT", 32);
   char hname[] = "ABDataCollector_ID11";
   WiFi.begin(savedssid.c_str(), savedpass.c_str());
   //WiFi.setHostname(hname);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    wifisetup == true;
    localWifi();
    //return;
  }

  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
 }

//Load HTML
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

//Server Request handeling
  server.on("/UpdateNow", HTTP_GET, [](AsyncWebServerRequest * request) {
   Serial.print("Pulling readings");
   TakeData = 1;
   request->send_P(200, "text/plain", "Post");
  });

  server.on("/Pull7", HTTP_GET, [](AsyncWebServerRequest * request) {
   Serial.print("Reading PH 7 Buffer..");
   calc7();
   request->send_P(200, "text/plain", "Post");
  });

  server.on("/Pull4", HTTP_GET, [](AsyncWebServerRequest * request) {
   Serial.print("Reading PH 4 Buffer..");
   calc4();
   request->send_P(200, "text/plain", "Post");
  });
AsyncElegantOTA.begin(&server); 
//server.onNotFound(notFound);

server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    if (request->hasParam(PARAM_COLFRQ)) {
      inputMessage = request->getParam(PARAM_COLFRQ)->value();
      writeFile(SPIFFS, "/colfrq", inputMessage.c_str());
    }
    else if (request->hasParam(WiFiSSID)) {
      inputMessage = request->getParam(WiFiSSID)->value();
      writeFile(SPIFFS, "/ssid.txt", inputMessage.c_str());
    }
    else if (request->hasParam(WiFiPassword)) {
      inputMessage = request->getParam(WiFiPassword)->value();
      writeFile(SPIFFS, "/wifipasswd.txt", inputMessage.c_str());
    }
    else if (request->hasParam(PARAM_ABADDESS)) {
      inputMessage = request->getParam(PARAM_ABADDESS)->value();
      writeFile(SPIFFS, "/abaddress.txt", inputMessage.c_str());
    }
    else if (request->hasParam(PARAM_POST)) {
      inputMessage = request->getParam(PARAM_POST)->value();
      //post2ab();
    }
    else if (request->hasParam(PARAM_DEVID)) {
      inputMessage = request->getParam(PARAM_DEVID)->value();
      writeFile(SPIFFS, "/id.txt", inputMessage.c_str());
    }
    
    
    Serial.println(inputMessage);
    request->send(200, "text/text", inputMessage);
  });
server.begin(); 
mac = (WiFi.macAddress());
}

void loop() {

  //int freq = readFile(SPIFFS, "/colfrq").toInt();
  //timerDelay = (freq * 60000); 
  
  //Send an HTTP POST request every X minutes
  if (((millis() - lastTime) > timerDelay) or (TakeData == 1)) {
    Serial.println("Taking Measurements and Posting Data");
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      //HTTPClient http;

PullPH();
POSTDATA();
//post2ab();

    }
    else {
      Serial.println("WiFi Disconnected");
    }
    //Ceck for updated collection frequencey
    //If null then exit

    int freq = readFile(SPIFFS, "/colfrq").toInt();
    timerDelay = (freq * 60000); 
    
    lastTime = millis();
    TakeData = 0;
  }
}


// new global vars
void POSTDATA( void )
{
  HTTPClient http;
  String ABAddress = readFile(SPIFFS, "/abaddress.txt");
  String IDx = readFile(SPIFFS, "/id.txt");
       
      // Your Domain name with URL path or IP address with path
      http.begin(wifiClient, ABAddress);

      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      // Data to send with HTTP POST
      String httpRequestData = "Id=" + IDx + "&msg=" + disPH;          
      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);

      Serial.println(httpRequestData);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
        
      // Free resources
      http.end();
      TakeData = 0;
}


void PullPH( void )
{

 for(int i=0;i<10;i++) 
 { 
 buffer_arr[i]=analogRead(A0);
 delay(30);
 }
 for(int i=0;i<9;i++)
 {
 for(int j=i+1;j<10;j++)
 {
 if(buffer_arr[i]>buffer_arr[j])
 {
 temp=buffer_arr[i];
 buffer_arr[i]=buffer_arr[j];
 buffer_arr[j]=temp;
 }
 }
 }
 avgval=0;
 for(int i=2;i<8;i++)
 avgval+=buffer_arr[i];
 volt=(float)avgval*5.0/1024/6;
 Serial.print("Raw Volatage =:");
 Serial.print(volt);
 //float ph_act = -5.70 * volt + calibration_value;
 //New Logic
float ph7 = readFile(SPIFFS, "/ph7.txt").toFloat();
float ph4 = readFile(SPIFFS, "/ph4.txt").toFloat();
float slope = ((ph7 - ph4) / (7 - 4));
slope = (slope * -1.0);
float ph_act = (7 + ((ph7 - volt) / slope));

 disPH = String(ph_act);
 Serial.print("PH =:");
 Serial.print(disPH);
 delay(1000);
}

void calc7 ( void )
{
  PullPH();
  ph7voltage = String(volt);
  Serial.print("PH7 Calibration Voltage =:");
  Serial.print(ph7voltage);
  writeFile(SPIFFS, "/ph7.txt", ph7voltage.c_str());
}

void calc4 ( void )
{
  PullPH();
  ph4voltage = String(volt);
  Serial.print("PH4 Calibration Voltage =:");
  Serial.print(ph4voltage);
  writeFile(SPIFFS, "/ph4.txt", ph4voltage.c_str());
}
