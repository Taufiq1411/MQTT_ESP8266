#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

const char* ssid = "MyWifi";
const char* password = "cempedak2";
const char* mqtt_server = "test.mosquitto.org"; 

WiFiClient espClient;
PubSubClient client(espClient);
DHT dhtd(13, DHT11);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg1[MSG_BUFFER_SIZE];
char msg2[MSG_BUFFER_SIZE];
int value1 = 0, value2 = 0;
const int relay = 15;

void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  
  if ((char)payload[0] == '1') {
    digitalWrite(relay, HIGH);
  } else {
    digitalWrite(relay, LOW);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.publish("SmartHome/temp", "temp is ready");
      client.publish("SmartHome/humid", "humid is ready");
      client.subscribe("SmartHome/relay");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(relay, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  dhtd.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    value1 = dhtd.readTemperature();
    value2 = dhtd.readHumidity();
    snprintf (msg1, MSG_BUFFER_SIZE, "%ld", value1);
    snprintf (msg2, MSG_BUFFER_SIZE, "%ld", value2);
    Serial.print("Publish message: ");
    Serial.println(msg1);
    Serial.print("  ");
    Serial.println(msg2);
    client.publish("SmartHome/temp", msg1);
    client.publish("SmartHome/humid", msg2);
  }
}
