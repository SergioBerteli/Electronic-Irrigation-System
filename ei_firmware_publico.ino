#include <WiFi.h>
#include "DHT.h"
#include "PubSubClient.h"
#include "time.h"
#include <stdio.h>
#include <string.h>
#define WIFI_SSID "SSID"
#define WIFI_PASSWORD "password"
#define DHTPIN 23 // pino que estamos conectado
#define DHTTYPE DHT11 // DHT 11
#define MSPIN 34
#define RELAYPIN 25

// variable for storing the potentiometer value
int soilMoisture = 0;
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -10800;
const int   daylightOffset_sec = 3600;
const char horarios_regar[2][9] = {"20:00:00", "08:00:00"};
struct tm timeinfo;
char time_now[9];
// Conecte pino 1 do sensor (esquerda) ao +5V
// Conecte pino 2 do sensor ao pino de dados definido em seu Arduino
// Conecte pino 4 do sensor ao GND
// Conecte o resistor de 10K entre pin 2 (dados) 
// e ao pino 1 (VCC) do sensor
const char* mqtt_server = "broker.hivemq.com";
DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;

PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
char clientId[50];
int value = 0;
float h, t;
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
 
}

void mqttReconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(RELAYPIN, LOW);
  dht.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ");
  Serial.print(WIFI_SSID);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {

//  Serial.print(F("Humidity: "));
  //Serial.print(h);
  //Serial.print(F("%  Temperature: "));
  //Serial.print(t);

  long now = millis();

if (!client.connected()) {

mqttReconnect();

}
  if (now - lastMsg > 10000) {
    lastMsg = now;
    
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    t = dht.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }
    // Uncomment the next line to set temperature in Fahrenheit 
    // (and comment the previous temperature line)
    //temperature = 1.8 * bme.readTemperature() + 32; // Temperature in Fahrenheit
    
    // Convert the value to a char array
    char msgUART[256];
    int msgUARTlen;
    msgUARTlen = sprintf(msgUART, "%f", t);
    Serial.print("Temperature: ");
    Serial.println(msgUART);
    client.publish("mestrado/iot/aluno/sergio_filho/temperatura", msgUART);

    msgUARTlen = sprintf(msgUART, "%f", h);

    Serial.print("Humidity: ");
    Serial.println(msgUART);
    client.publish("mestrado/iot/aluno/sergio_filho/umidade", msgUART);
    soilMoisture = analogRead(MSPIN);
    Serial.println(soilMoisture);
    msgUARTlen = sprintf(msgUART, "%d", soilMoisture);
    client.publish("mestrado/iot/aluno/sergio_filho/umidade/solo", msgUART);

    
    Serial.println(time_now);
    
  }
    if(!getLocalTime(&timeinfo)){
      Serial.println("Failed to obtain time");
      return;
    }
    strftime(time_now,9, "%H:%M:%S", &timeinfo);

    uint8_t devo_regar = 0;
    for (int i=0; i<2;i++){
      if (strcmp(horarios_regar[i], time_now) == 0 && soilMoisture> 1360){
        devo_regar = 1;
      }
    }
    if (devo_regar){
      digitalWrite(RELAYPIN, HIGH);
      Serial.println("Regando o a plantação");
      delay(5000);
      digitalWrite(RELAYPIN, LOW);
    }
}
