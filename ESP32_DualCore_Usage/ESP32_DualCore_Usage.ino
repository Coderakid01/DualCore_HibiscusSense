/*
This Code never been verified by anyone for the perfomance part... It seems the code working where both Task execute at one time
But the Stability has yet remain a question.

This Demo use the following components, (Can refer to Fritzing Sketch for Circuit Installation)
2 Sensor : DHT22 and MQ135;
1 Controller : Hibiscus Sense ESP32 Based Architecture
*/

#include <WiFi.h>
#include <HTTPClient.h>

#include <MQUnifiedsensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include "DHT.h"

#define DHTPIN 32
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

char ssid[] = "wIFI_NAME";
char pass[] = "WIFI_PASSWORD";
char device_developer_id[] = "DEVELOPER_ID";

float MQ135, SoilMoisture, Humidity;

TaskHandle_t Task1;
TaskHandle_t Task2;

void setup() {
  Serial.begin(115200); //Baud Rate set at 115200


  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    DataStream,   /* Task function. */
                    "Data Streaming and WiFi Process",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    SensorRead,   /* Task function. */
                    "Sensor Reading Concentration (Wihtout Calibration)",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    delay(500); 
}

//CORE 0 for WiFi and Data Upload to Platform Concentration Part
void DataStream( void * pvParameters ){
  Serial.print("Data Streaming and WiFi running on core ");
  Serial.println(xPortGetCoreID());

  WiFi.begin(ssid, pass);

  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(250);
  } 

  for(;;){

  FAVORIOT(); //Call FAVORIOT Function to execute the Data Stream Command
  delay (1000);
  } 
}

//CORE 1 for Sensor Reading Concentration Part (Without any Calibration)
void SensorRead( void * pvParameters ){
  Serial.print("Sensor Reading running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){

    MQ135 = analogRead (34);
    Humidity = dht.readHumidity();

      Serial.print ("MQ-135 : ");
      Serial.println (MQ135);
      Serial.print ("Humidity : ");
      Serial.println (Humidity);
      Serial.println();
      delay(1500);

  }
}

void loop() {
  
}

// FAVORIOT Function to stream the data to The Platform
void FAVORIOT () {

    String json = "{\"device_developer_id\":\"" + String(device_developer_id) + "\",\"data\":{";
    
    json += "\"Air_Quality\":\"" + String(MQ135) + "\",";
    json += "\"Humidity\":\"" + String(Humidity) + "\"";
    
    json += "}}";
    HTTPClient http;

    http.begin("https://apiv2.favoriot.com/v2/streams");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("apikey", "YOUR_API_KEY");

    int httpCode = http.POST(json);

    if(httpCode > 0) {
      String payload = http.getString();
      Serial.println(payload);
    }
    else {
      Serial.println("HTTP Error!");
    }

    http.end();
  }
