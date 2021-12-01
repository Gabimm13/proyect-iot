
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "FirebaseESP8266.h"

// Configuration parameters for Access Point

#define FIREBASE_HOST "temp-drink-default-rtdb.firebaseio.com" //Sin incluir http:// or https:// 
#define FIREBASE_AUTH "SnxHQS0jDmj8ZpHM6EqiaNBqKSYnJEzKPB7IVo7L" //SECRETO DE LA BASE
#define WIFI_SSID "Gabys_room" //SSID DE SU RED
#define WIFI_PASSWORD "F82DC014EAE4" //CONTRASEÑA
#define host "192.168.0.11" //////////////////////////////AQUI VA LA IP DE SU COMPUTADORA//////////////////

// Set up the server object
ESP8266WebServer server(80);


//Define FirebaseESP8266 data object  
FirebaseData firebaseData;


FirebaseJson json;

int lcdColumns = 16;
int lcdRows = 2;
float sensorValue=0;

float temperatureC=0.0;
int led=D6;
int led1=D7;
int led2=D8;

// GPIO where the DS18B20 is connected to
const int oneWireBus = D5;     

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

String messageStatic = "TEMP-DRINK";
String messageToScroll = "La temperatura es de :";


void scrollText(int row, String message, int delayTime, int lcdColumns) {
  for (int i=0; i < lcdColumns; i++) {
    message = " " + message;  
  } 
  message = message + " "; 
  for (int pos = 0; pos < message.length(); pos++) {
    lcd.setCursor(0, row);
    lcd.print(message.substring(pos, pos + lcdColumns));
    delay(delayTime);
  }
}



void setup() {

Serial.begin(9600);
WiFi.mode(WIFI_AP);
WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
Serial.print("Connecting to Wi-Fi");
while (WiFi.status() != WL_CONNECTED)
{
Serial.print(".");
delay(300);
}
Serial.println();
Serial.print("Connected with IP: ");
Serial.println(WiFi.localIP());
Serial.println();

// Configure the server's routes
server.on("/",handleIndex); // use the top root path to report the last sensor valuer //S1
server.on("/update",handleUpdate); // use this route to update the sensor value //S2
server.begin();



  
  // Start the Serial Monitor
  Serial.begin(115200);
  // Start the DS18B20 sensor
  sensors.begin();
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);
  
  lcd.init();                     
  lcd.backlight();


  

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);


  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  firebaseData.setBSSLBufferSize(1024, 1024);


  //Set the size of HTTP response buffers in the case where we want to work with large data.
  firebaseData.setResponseSize(1024);


  //Set database read timeout to 1 minute (max 15 minutes)
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  //tiny, small, medium, large and unlimited.
  //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
  Firebase.setwriteSizeLimit(firebaseData, "tiny");


  Serial.println("------------------------------------");
  Serial.println("Updated temperature...");
}


void printResult(FirebaseData &data)
{
  Serial.println();
  FirebaseJson &json = data.jsonObject();
  //Print all object data
  Serial.println("Pretty printed JSON data:");
  String jsonStr;
  json.toString(jsonStr, true);
  Serial.println(jsonStr);
  Serial.println();
  Serial.println("Iterate JSON data:");
  //Serial.println();
}




void handleIndex(){ 
String Website = "<!DOCTYPE HTML>\r\n \r\n<html><head><title>Lectura de temperatura</title><meta http-equiv=\"refresh\" content=\"2\"></head><body ><center><h1>Temp-Drink: </h1> La temperatura de la bebida es de : "+ String(temperatureC)+"*C<br> </center></body></html>\n";

server.send(200,"text/html",Website); // we'll need to refresh the page for getting the lastest value //S1


}

void handleUpdate(){
// The value will be passed as a URL argument //S2
 temperatureC = server.arg("value").toFloat();
Serial.println( temperatureC);
server.send(200,"text/plain","updated");
}
 
void loop() {

////////////////////////CONEXION A MYSQL//////////////////////////////////////
    Serial.print("connecting to ");
    Serial.println(host);

    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
        Serial.println("connection failed");
        return;
    }



    // This will send the request to the server
 client.print(String("POST http://localhost/iot/connect.php?") +  //////////////AQUI EN LUGAR DE IOT, VA EL NOMBRE DONDE TIENES GUARDADO EL ARCHIVO DE CONEXION
                          ("&temperatura=") + temperatureC +               
                          " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 1000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
        }
    }

    // Read all the lines of the reply from server and print them to Serial
    while(client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
        
    }

    Serial.println();
    Serial.println("closing connection");








////////////////////////////////////////////////////////////////
  
 Serial.print("Connected with IP: ");
 Serial.println(WiFi.localIP());
 server.handleClient();

 

  sensors.requestTemperatures(); 
  temperatureC = sensors.getTempCByIndex(0);
  Serial.print(temperatureC);
  Serial.println("ºC");
  delay(3000);
  
  String path = "/Test";
  json.set("Gabriela Mejia" + String(1), temperatureC); /////AQUI VA SU NOMBRE PARA IDENTIFICAR SU CONEXION

  if(temperatureC >= 60){
    digitalWrite(led , HIGH);
  }else{
    digitalWrite(led , LOW);
  }

  if(temperatureC >=30 and temperatureC < 60){
    digitalWrite(led1 , HIGH);
  }else{
    digitalWrite(led1 , LOW);
  }

  if(temperatureC < 30){
    digitalWrite(led2 , HIGH);
  }else{
    digitalWrite(led2 , LOW);
  }
  
  lcd.setCursor(0, 0);
  lcd.print(messageStatic);
  scrollText(1, messageToScroll, 250, lcdColumns);
  lcd.print(temperatureC);

if (Firebase.updateNode(firebaseData, path + "/float", json))
  {
    //Serial.print("VALUE: ");
    printResult(firebaseData);
    Serial.println("------------------------------------");
    Serial.println();
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
  delay(1000);
  
}
