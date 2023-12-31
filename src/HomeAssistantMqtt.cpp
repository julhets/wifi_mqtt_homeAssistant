/*
  HomeAssistantMqtt.h - Library to easily discover and register devices and entities on homeAssistant.
  Created by Julio Reis, October 29, 2023.
  Released into the public domain.
*/

#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "PubSubClient.h"
#include "HomeAssistantMqtt.h"

String deviceUniqueId;

HomeAssistantMqtt::HomeAssistantMqtt()
{
  client = PubSubClient(espClient);
}

void HomeAssistantMqtt::initWifiAndMqttClient(
    String wifiSsid,
    String wifiPassword,
    String mqttBroker,
    uint16_t mqttPort,
    String mqttUsername,
    String mqttPassword)
{
  initWifiAndMqttClient(wifiSsid, wifiPassword, mqttBroker, mqttPort, mqttUsername, mqttPassword, "");
}

void HomeAssistantMqtt::connectToWifi(String wifiSsid, String wifiPassword) {
    // connecting to a WiFi network
  WiFi.begin(wifiSsid, wifiPassword);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Connecting to WiFi..");
    delay(2000);
  }
  Serial.println("Connected to the WiFi network");
  // end
}

void HomeAssistantMqtt::initWifiAndMqttClient(
    String wifiSsid,
    String wifiPassword,
    String mqttBroker,
    uint16_t mqttPort,
    String mqttUsername,
    String mqttPassword,
    String deviceUniqueIdParam)
{
  connectToWifi(wifiSsid, wifiPassword);

  // if the user didn't set the deviceUniqueId, let's set the macAddress as it;
  if(deviceUniqueIdParam != "") {
    deviceUniqueId = deviceUniqueIdParam;
  } else {
    deviceUniqueId = WiFi.macAddress();
    deviceUniqueId.replace(":", "");
  }
  //end 

  // mqtt part
  client.setBufferSize(1024);
  client.setServer(mqttBroker.c_str(), mqttPort);

  while (!client.connected())
  {
    Serial.printf("The client %s connects to the public mqtt broker\n", deviceUniqueId.c_str());
    if (client.connect(deviceUniqueId.c_str(), mqttUsername.c_str(), mqttPassword.c_str()))
    {
      Serial.println("Public emqx mqtt broker connected");
    }
    else
    {
      Serial.println("failed with state ");
      Serial.println(client.state());
      delay(2000);
    }
  }
  //end
}

CallbackRegister callbacksRegisters[10];
int nextCallbackArrayAvailableIndex = 0;

static void customCallback(char *topic, byte *payload, unsigned int length)
{
  String payloadAsString;
  String topicAsString = String(topic);
  for (unsigned int i = 0; i < length; i++)
  {
    payloadAsString += (char)payload[i];
  }
  Serial.println("Payload received: " + payloadAsString + ", on topic: " + topicAsString);

  for (int i = 0; i < 10; i++)
  {
    if (callbacksRegisters[i].key.equals(topicAsString))
    {
      callbacksRegisters[i].callback(payloadAsString);
      return;
    }
  }
}

void HomeAssistantMqtt::addCallback(String topic, std::function<void(String payload)> callback)
{
  callbacksRegisters[nextCallbackArrayAvailableIndex] = {topic, callback};
  nextCallbackArrayAvailableIndex = nextCallbackArrayAvailableIndex + 1;
}

void HomeAssistantMqtt::registerEndpoint(String endpointName, String endpointCode, String deviceHAClass, String unitOfMeasurement, bool isOut, bool isIn, std::function<void(String payload)> callback)
{
  client.setCallback(customCallback);
  if (isIn)
  {
    String deviceEntityRegisterTopic = "homeassistant/sensor/in/" + deviceUniqueId + "_" + endpointCode + "/config";
    String stateTopic = "homeassistant/sensor/in/" + deviceUniqueId + "_" + endpointCode + "/state";
    String deviceEntityRegisterPayload = "{\"device_class\": \"" + deviceHAClass + "\", \"unit_of_measurement\": \"" + unitOfMeasurement + "\", \"value_template\": \"{{ value_json." + endpointCode + " }}\", \"state_topic\": \"" + stateTopic + "\", \"unique_id\": \"" + deviceUniqueId + "_" + endpointCode + "_in" + "\", \"name\": \"" + endpointName + "_IN" + "\", \"device\":{ \"identifiers\":[ \"" + deviceUniqueId + "\" ], \"name\":\"" + deviceUniqueId + "\" } }";
    client.publish(deviceEntityRegisterTopic.c_str(), deviceEntityRegisterPayload.c_str());
    client.subscribe(stateTopic.c_str());
    addCallback(stateTopic, callback);
  }
  if (isOut)
  {
    String stateTopic = "homeassistant/sensor/out/" + deviceUniqueId + "_" + endpointCode + "/state";
    String deviceEntityRegisterTopic = "homeassistant/sensor/out/" + deviceUniqueId + "_" + endpointCode + "/config";
    String deviceEntityRegisterPayload = "{\"device_class\": \"" + deviceHAClass + "\", \"unit_of_measurement\": \"" + unitOfMeasurement + "\", \"value_template\": \"{{ value_json." + endpointCode + " }}\", \"state_topic\": \"" + stateTopic + "\", \"unique_id\": \"" + deviceUniqueId + "_" + endpointCode + "_out" + "\", \"name\": \"" + endpointName + "_OUT" + "\", \"device\":{ \"identifiers\":[ \"" + deviceUniqueId + "\" ], \"name\":\"" + deviceUniqueId + "\" } }";
    client.publish(deviceEntityRegisterTopic.c_str(), deviceEntityRegisterPayload.c_str());
  }
}

void HomeAssistantMqtt::publishMessage(String endpointCode, String value)
{
  String stateTopic = "homeassistant/sensor/out/" + deviceUniqueId + "_" + endpointCode + "/state";
  String payload = "{\"" + endpointCode + "\": \"" + value + "\"}";
  client.publish(stateTopic.c_str(), payload.c_str());
}

void HomeAssistantMqtt::loop()
{
  client.loop();
}