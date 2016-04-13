#include <ESP8266WiFi.h>
#include <DHT.h>

#define DHTPIN D4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

const char* ssid     = "ITCOL";
const char* password = "DaRa5803";

const char* zabbix = "192.168.0.103";

const char* host   = "WemosD1mini";
const char* item_temp = "temp";
const char* item_hum = "hum";
const char* item_heat = "heat";

// sleep for this many seconds
const int sleepSeconds = 30;

float humidity, temperature, heatIndex;
char str_humidity[10], str_temperature[10], str_heatIndex[10];


void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.println("\n\nWake up");

  // Connect D0 to RST to wake up
  pinMode(D0, WAKEUP_PULLUP);

  //connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("");
  Serial.println("Initialize DHT sensor");
  dht.begin();
  delay(2000);

}


void loop() {
  delay(2000);
  
  Serial.println();
  Serial.println("Read DHT sensor");
  
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  
  float heatIndex = dht.computeHeatIndex(temperature, humidity, false);

  // Convert the floats to strings and round to 2 decimal places
  dtostrf(humidity, 1, 2, str_humidity);
  dtostrf(temperature, 1, 2, str_temperature);
  dtostrf(heatIndex, 1, 2, str_heatIndex);

  Serial.printf("Humidity:    %s %%\nTemperature: %s *C\nHeat index:  %s *C\n", str_humidity, str_temperature, str_heatIndex);

  Serial.println();
  Serial.print("connecting to ");
  Serial.println(zabbix);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int port = 10051;
  if (!client.connect(zabbix, port)) {
    Serial.println("connection failed");
    return;
  }
  
    Serial.println("Sending data to zabbix");
    
      // We now create a JSON string for zabbix trapper
      String json = "{""\"request\":""\"sender data\""",""\"data\""":""[";

      //sending temperature
      json += "{""\"host\""":";
      json += (String("\"") + host + "\"" + ",""\"key\""":" + "\"" + item_temp + "\"" + ",""\"value\""":" + "\"" + str_temperature + "\"" + "}");
      json += ",";
      
      //sending humidity
      json += "{""\"host\""":";
      json += (String("\"") + host + "\"" + ",""\"key\""":" + "\"" + item_hum + "\"" + ",""\"value\""":" + "\"" + str_humidity + "\"" + "}");
      json += ",";

      //sending heatindex
      json += "{""\"host\""":";
      json += (String("\"") + host + "\"" + ",""\"key\""":" + "\"" + item_heat + "\"" + ",""\"value\""":" + "\"" + str_heatIndex + "\"" + "}");
      json += "]""}";

     
     Serial.print("Requesting JSON string: ");
     Serial.println(json);  

    //client.print will do all job
    client.print(json);
   
    Serial.println("Closing connection");
    client.stop();
  
  
  
  delay (3000);
  Serial.printf("Sleep for %d seconds\n\n", sleepSeconds);

  // convert to microseconds
  ESP.deepSleep(sleepSeconds * 1000000);

}
