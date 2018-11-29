#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <mbusparser.h>

static const int LED_PIN = 2;
static const char* ssid = "YourWifi";
static const char* password = "YourPassword";
static const char* mqtt_server = "192.168.10.58";

WiFiClient espClient;
PubSubClient client(espClient);
char msg[60];

// Power meter serial com
std::vector<uint8_t> receiveBuffer;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial1.println();
  Serial1.print("Connecting to ");
  Serial1.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial1.print(".");
  }

  randomSeed(micros());

  Serial1.println("");
  Serial1.println("WiFi connected");
  Serial1.println("IP address: ");
  Serial1.println(WiFi.localIP());
}

void blink(int count) {
  for (int i = 0; i < count; i++) {
    digitalWrite(LED_PIN, LOW);
    delay(50);
    digitalWrite(LED_PIN, HIGH);
    delay(50);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial1.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "Kamstrup-ESP8266-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), "mqttclient", "YourPassword")) {
      Serial1.println("connected");
      // Once connected, publish an announcement...
      client.publish("house/electricity/total/status", "Initializing");
    } else {
      Serial1.print("failed, rc=");
      Serial1.print(client.state());
      Serial1.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void parseData() {
  // Parse serial data here, and send to MQTT broker
  std::vector<VectorView> frames = getFrames(receiveBuffer);
  if (frames.size() > 0) {
    for (VectorView vv : frames) {
      MeterData md = parseMbusFrame(vv);

      if (md.activePowerPlusValid) {
	snprintf(msg, sizeof(msg), "%ld", md.activePowerPlus);
	client.publish("house/electricity/power/activePowerPlus", msg);
      }
      if (md.activePowerMinusValid) {
	snprintf(msg, sizeof(msg), "%ld", md.activePowerMinus);
	client.publish("house/electricity/power/activePowerMinus", msg);
      }
      if (md.reactivePowerPlusValid) {
	snprintf(msg, sizeof(msg), "%ld", md.reactivePowerPlus);
	client.publish("house/electricity/power/reactivePowerPlus", msg);
      }
      if (md.reactivePowerMinusValid) {
	snprintf(msg, sizeof(msg), "%ld", md.reactivePowerMinus);
	client.publish("house/electricity/power/reactivePowerMinus", msg);
      }

      if (md.voltageL1Valid) {
	snprintf(msg, sizeof(msg), "%ld", md.voltageL1);
	client.publish("house/electricity/voltage/l1", msg);
      }
      if (md.voltageL2Valid) {
	snprintf(msg, sizeof(msg), "%ld", md.voltageL2);
	client.publish("house/electricity/voltage/l2", msg);
      }
      if (md.voltageL3Valid) {
	snprintf(msg, sizeof(msg), "%ld", md.voltageL3);
	client.publish("house/electricity/voltage/l3", msg);
      }
    }
    receiveBuffer.clear();
  }
}

void setup() {
  receiveBuffer.reserve(500);
  pinMode(LED_PIN, OUTPUT);     // Initialize the LED_PIN pin as an output
  Serial.begin(2400, SERIAL_8N1);
  Serial.swap();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  client.publish("house/electricity/status", "Initializing");  
}

int bytesReceived = 0;
int loopCounter = 0;

// the loop function runs over and over again forever
void loop() {
  blink(10);
  for (int i = 0; i < 10; ++i) {
    delay(1000);
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
    while (Serial.available() > 0) {
      receiveBuffer.push_back(Serial.read());
      bytesReceived++;
      if (receiveBuffer.back() == 0x7E && receiveBuffer.size() > 5) {
        parseData();
      }
      if (receiveBuffer.size() >= 500) {
        // Make sure we don't fill up the memory
        receiveBuffer.clear();
      }
    }
  }
  ++loopCounter;

  // Debug:
  snprintf(msg, sizeof(msg), "Looping. Loops=%ld", loopCounter);
  client.publish("house/electricity/status", msg);
  snprintf(msg, sizeof(msg), "BufSize=%ld", receiveBuffer.size());
  client.publish("house/electricity/status", msg);
  snprintf(msg, sizeof(msg), "Received bytes: %ld", bytesReceived);
  client.publish("house/electricity/status", msg);
}
