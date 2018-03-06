#include <ESP8266SSDP.h>
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library
#include <PubSubClient.h>         //MQTT
 
////**********START CUSTOM PARAMS******************//
 
////Define parameters for the http firmware update // which we aren't using 
const char* host = "Garage1ESP";
const char* ssid = "YOURSSID";
const char* password = "YOURPW";
 
//Define the pins
#define RELAY_PIN D1
#define DOOR_PIN 15 //just another name for D8 pin
 
//Define your own MQTT 
#define mqtt_server "192.168.1.71"  //This is what you set up in HA. 
#define door_topic "sensor/garage/state1" //you can change this name, but make sure you "replace all"
#define button_topic "sensor/garage/action1" //you can change this name, but make sure you "replace all"
const char* mqtt_user = "YOURMQTTUSER"; //This is what you set up in HA. 
const char* mqtt_pass = "YOURMQTTPW"; //This is what you set up in HA. 
 
//************END CUSTOM PARAMS********************//
//This can be used to output the date the code was compiled
const char compile_date[] = __DATE__ " " __TIME__;
 
WiFiClient espCgarage1;
 
//Initialize MQTT
PubSubClient client(espCgarage1);
 
//Setup Variables
String switch1;
String strTopic;
String strPayload;
char* door_state = "UNDEFINED";
char* last_state = "";
 
void setup() {
  //Set Relay(output) and Door(input) pins
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  pinMode(DOOR_PIN, INPUT);
 
  Serial.begin(115200);

  setup_wifi();


  client.setServer(mqtt_server, 1883); //1883 is the port number you have forwared for mqtt messages. You will need to change this if you've used a different port 
  client.setCallback(callback); //callback is the function that gets called for a topic sub
}


void loop() {
  //If MQTT client can't connect to broker, then reconnect
  if (!client.connected()) {
    reconnect();
  }
  checkDoorState();
  client.loop(); //the mqtt function that processes MQTT messages
  
}
 
void callback(char* topic, byte* payload, unsigned int length) {
  //if the 'garage/button' topic has a payload "OPEN", then 'click' the relay
  payload[length] = '\0';
  strTopic = String((char*)topic);
  if (strTopic == button_topic)
  {
    switch1 = String((char*)payload);
    Serial.println(switch1);
    if (switch1 == "OPEN")
    {
      //'click' the relay
      Serial.println("ON");
      digitalWrite(RELAY_PIN, LOW);
      delay(600);
      digitalWrite(RELAY_PIN, HIGH);
    }
  }
}



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

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}



void checkDoorState() {
  //Checks if the door state has changed, and MQTT pub the change
  last_state = door_state; //get previous state of door
  if (digitalRead(DOOR_PIN) == 0) // get new state of door
    door_state = "closed";
  else if (digitalRead(DOOR_PIN) == 1)
    door_state = "open";  
  if (last_state != door_state) { // if the state has changed then publish the change
    client.publish(door_topic, door_state, true);
    Serial.println(door_state);
  }
}
 
void reconnect() {
  //Reconnect to Wifi and to MQTT. If Wifi is already connected, then autoconnect doesn't do anything.
  Serial.print("Attempting MQTT connection...");
  if (client.connect(host, mqtt_user, mqtt_pass)) {
    Serial.println("connected");
    client.subscribe("sensor/garage/#");
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
    // Wait 5 seconds before retrying
    delay(5000);
  }
}
