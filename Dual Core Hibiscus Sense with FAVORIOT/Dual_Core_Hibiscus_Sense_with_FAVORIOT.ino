/*
This Code never been verified by anyone for the perfomance part... It seems the code working where both Task execute at one time
But the Stability has yet remain a question.
*/

#include <WiFi.h>
#include <HTTPClient.h>

#include <Adafruit_BME280.h>
Adafruit_BME280 bme;

long previousMillis = 0;

char ssid[] = "wIFI_NAME";
char pass[] = "WIFI_PASSWORD";
char device_developer_id[] = "DEVELOPER_ID";

TaskHandle_t Task1;
TaskHandle_t Task2;

void setup() {
  Serial.begin(115200); //Baud Rate set at 115200
  
  if (!bme.begin()){
    Serial.println("Failed to find Hibiscus Sense BME280 chip");
  }

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
  
  if(millis() - previousMillis > 10000){

    previousMillis = millis();
    String json = "{\"device_developer_id\":\"" + String(device_developer_id) + "\",\"data\":{";
    
    json += "\"altitude\":\"" + String(bme.readAltitude(1013.25)) + "\",";
    json += "\"barometer\":\"" + String(bme.readPressure()/100.00) + "\",";
    json += "\"humidity\":\"" + String(bme.readHumidity()) + "\",";
    json += "\"temperature\":\"" + String(bme.readTemperature()) + "\"";
    
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
  delay (1000);
  } 
 }
}

//CORE 1 for Sensor Reading Concentration Part (Without any Calibration)
void SensorRead( void * pvParameters ){
  Serial.print("Sensor Reading running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){

      Serial.print ("Altitude Reading : ");
      Serial.println (bme.readAltitude(1013.25));
      Serial.print ("Pressure Reading : ");
      Serial.println (bme.readPressure()/100.00);
      Serial.print ("Humidity Reading : ");
      Serial.println (bme.readHumidity());
      Serial.print ("Temperature Reading : ");
      Serial.println (bme.readTemperature());
      Serial.println();
      delay(1500);

  }
}

void loop() {
  
}
