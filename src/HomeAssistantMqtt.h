/*
  HomeAssistantMqtt.h - Library to easily discover and register devices and entities on homeAssistant.
  Created by Julio Reis, October 29, 2023.
  Released into the public domain.
*/
#ifndef HomeAssistantMqtt_h
#define HomeAssistantMqtt_h

#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "PubSubClient.h"

typedef struct
{
   String key;
   std::function<void(String payload)> callback;
}  CallbackRegister;

class HomeAssistantMqtt
{
  public:
    HomeAssistantMqtt();
    void initWifiAndMqttClient(String wifiSsid, String wifiPassword, String mqttBroker, uint16_t mqttPort, String mqttUsername, String mqttPassword);
    void initWifiAndMqttClient(String wifiSsid, String wifiPassword, String mqttBroker, uint16_t mqttPort, String mqttUsername, String mqttPassword, String deviceUniqueId);
    void registerEndpoint(String endpointName, String endpointCode, String deviceHAClass, String unitOfMeasurement, bool isOut, bool isIn, std::function<void(String payload)> callback);
    void publishMessage(String endpointCode, String message);
    void loop();
  private:
    WiFiClient espClient;
    PubSubClient client;
    void connectToWifi(String wifiSsid, String wifiPassword);
    void addCallback(String topic, std::function<void(String payload)> callback);
};

#endif