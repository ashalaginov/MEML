#include <UIPEthernet.h>
#include "PubSubClient.h"
#include <ArduinoJson.h>
#include <EEPROM.h>

#define INTERVAL        3000 // 3 sec delay between publishing
uint8_t mac[6] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
IPAddress ip(192, 168, 0, 100);

EthernetClient ethClient;
PubSubClient mqttClient;

long previousMillis;

bool receivedModel = 0;

const int InputNodes = 9;
const int HiddenNodes = 1;
const int OutputNodes = 1;
const int PatternCount = 2;

float HiddenWeights[InputNodes + 1][HiddenNodes];
float OutputWeights[HiddenNodes + 1][OutputNodes];
float Hidden[HiddenNodes];
float Output[OutputNodes];
float Accum;

//Normal & Attack traffic
const float Testing[PatternCount][InputNodes] = {
  {300, 13788, 0 , 1, 0, 0, 1, 0.02,  0}, // 0
  {0, 0, 0, 0, 0, 0, 0.05, 0, 0} //1
};

const int TestingClass[PatternCount] = {0, 1};


void callback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<208> doc;

  deserializeJson(doc, payload, length);
  JsonArray hidw = doc["HiddenWeights"];
  JsonArray outw = doc["OutputWeights"];
  /***** Debugging *****
    Serial.println(F("Message arrived ["));
    Serial.print(F("Length ["));
    Serial.println(length);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
  */
  Serial.print("Hidden Weights ");
  for (int i = 0; i <= InputNodes; i++)
  {
    HiddenWeights[i][0] = hidw[i][0];
    Serial.print(HiddenWeights[i][0]);
    Serial.print(" ");

  }
  Serial.println(" ");
  Serial.print("Output Weights ");

  for (int i = 0; i <= HiddenNodes; i++)
  {
    OutputWeights[i][0] = outw[i][0];
    Serial.print(OutputWeights[i][0]);
    Serial.print(" ");
  }
  Serial.println(" ");

  if (length > 0)
    receivedModel = 1;
}

void setup() {
  // setup serial communication
  Serial.begin(9600);
  // setup ethernet communication using DHCP
  Ethernet.begin(mac, ip);
  // Allow the hardware to sort itself out
  delay(1500);
  //if(Ethernet.begin(mac) == 0) {
  // Serial.println(F("Ethernet configuration using DHCP failed"));
  //for(;;);
  //  }
  // setup mqtt client
  mqttClient.setClient(ethClient);
  mqttClient.setServer("192.168.0.200", 1883); //for using local broker
  Serial.println(F("MQTT client configured"));
  mqttClient.setCallback(callback);

  //previousMillis = millis();
}

void loop() {
  // check interval
  //if (millis() - previousMillis > INTERVAL) {
  //sendData();
  reconnect();

  // previousMillis = millis();
  //}
  mqttClient.loop();
  if (receivedModel == 1)
    toTerminal();
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print(F("Attempting MQTT connection..."));
    // Attempt to connect
    if (mqttClient.connect("arduinoClient")) {
      Serial.println(F("connected"));
      // Once connected, publish an announcement...
      // ... and resubscribe

      mqttClient.subscribe("ANN");
    } else {
      Serial.print(F("failed, rc="));
      Serial.print(mqttClient.state());
      Serial.println(F(" try again in 5 seconds"));
      receivedModel = 0;
      // Wait 5 seconds before retrying
      delay(500);
    }
  }
}



void toTerminal()
{

  for (int p = 0 ; p < PatternCount ; p++ ) {

    Serial.println();
    Serial.print ("  Training Pattern: ");
    Serial.println (p);
    Serial.print ("  Input ");
    for (int i = 0 ; i < InputNodes ; i++ ) {
      Serial.print (Testing[p][i], 2);
      Serial.print ("  ");

    }
    /*
      Serial.print (" ");
      }
      Serial.print ("  Target ");
      for ( i = 0 ; i < OutputNodes ; i++ ) {
      Serial.print (Target[p][i], 2);
      Serial.print (" ");
      }
    */
    uint32_t ts1 = micros();
    /******************************************************************
      Compute hidden layer activations
    ******************************************************************/

    for (int i = 0 ; i < HiddenNodes ; i++ ) {
      Accum = HiddenWeights[InputNodes][i] ;
      for (int j = 0 ; j < InputNodes ; j++ ) {
        Accum += Testing[p][j] * HiddenWeights[j][i] ;
      }
      Hidden[i] = 1.0 / (1.0 + exp(-Accum)) ;
    }

    /******************************************************************
      Compute output layer activations and calculate errors
    ******************************************************************/

    for (int i = 0 ; i < OutputNodes ; i++ ) {
      Accum = OutputWeights[HiddenNodes][i] ;
      for (int j = 0 ; j < HiddenNodes ; j++ ) {
        Accum += Hidden[j] * OutputWeights[j][i] ;
      }
      Output[i] = 1.0 / (1.0 + exp(-Accum)) ;
    }
    uint32_t ts2 = micros();
    /*
        Serial.print ("  Output ");
        for (int i = 0 ; i < OutputNodes ; i++ ) {
          Serial.print (Output[i], 5);
          Serial.print (" ");
        }
    */
    Serial.println();
    Serial.print (F("Required output: "));
    Serial.println (Output[0], 5);

    Serial.print (F("Predicted output: "));
    Serial.println (TestingClass[p], 5);

    Serial.print ("-----Prediction time, microseconds:");
    Serial.print(ts2 - ts1);
    Serial.print ("-----");
  }

  delay(5000);

}
