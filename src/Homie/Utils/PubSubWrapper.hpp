#pragma once

#include <functional>
#include <vector>

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define SHA1_SIZE 20

typedef int AsyncMqttClientDisconnectReason;
typedef void* AsyncMqttClientMessageProperties;

typedef std::function<void(bool sessionPresent)> OnConnectUserCallback;
typedef std::function<void(AsyncMqttClientDisconnectReason reason)> OnDisconnectUserCallback;
typedef std::function<void(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)> OnMessageUserCallback;
typedef std::function<void(uint16_t packetId)> OnPublishUserCallback;


class PubSubWrapper {
 public:
  PubSubWrapper();
  // ~PubSubWrapper();

  PubSubWrapper& setKeepAlive(uint16_t keepAlive);
  PubSubWrapper& setClientId(const char* clientId);
  PubSubWrapper& setCleanSession(bool cleanSession);
  PubSubWrapper& setMaxTopicLength(uint16_t maxTopicLength);
  PubSubWrapper& setCredentials(const char* username, const char* password = nullptr);
  PubSubWrapper& setWill(const char* topic, uint8_t qos, bool retain, const char* payload = nullptr, size_t length = 0);
  PubSubWrapper& setServer(IPAddress ip, uint16_t port);
  PubSubWrapper& setServer(const char* host, uint16_t port);
#if MQTT_SSL
  PubSubWrapper& setSecure(bool secure);
  PubSubWrapper& addServerFingerprint(const uint8_t* fingerprint);
#endif

  PubSubWrapper& onConnect(OnConnectUserCallback callback);
  PubSubWrapper& onDisconnect(OnDisconnectUserCallback callback);
  PubSubWrapper& onMessage(OnMessageUserCallback callback);
  PubSubWrapper& onPublish(OnPublishUserCallback callback);

  inline bool connected() { return _pubsub.connected(); }
  void connect();
  void disconnect(bool force = false);
  uint16_t subscribe(const char* topic, uint8_t qos);
  uint16_t unsubscribe(const char* topic);
  uint16_t publish(const char* topic, uint8_t qos, bool retain, const char* payload = nullptr, size_t length = 0, bool dup = false, uint16_t message_id = 0);
  inline void loop() { _pubsub.loop(); }
 private:
 #if MQTT_SSL
  WiFiClientSecure _client;
 #else
  WiFiClient _client;
 #endif  

  PubSubClient _pubsub;

  bool _connected;

  const char* _host;
#if MQTT_SSL
  bool _secure;
#endif
  uint16_t _port;
  uint16_t _keepAlive;
  bool _cleanSession;
  const char* _clientId;
  const char* _username;
  const char* _password;
  const char* _willTopic;
  const char* _willPayload;
  uint16_t _willPayloadLength;
  uint8_t _willQos;
  bool _willRetain;

  void _callback(const char* topic, byte* payload, unsigned int length);

#if MQTT_SSL
  std::vector<std::array<uint8_t, SHA1_SIZE>> _secureServerFingerprints;
#endif

  std::vector<OnConnectUserCallback> _onConnectUserCallbacks;
  std::vector<OnDisconnectUserCallback> _onDisconnectUserCallbacks;
  std::vector<OnMessageUserCallback> _onMessageUserCallbacks;
  std::vector<OnPublishUserCallback> _onPublishUserCallbacks;
};
