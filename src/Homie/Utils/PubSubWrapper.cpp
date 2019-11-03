#include "PubSubWrapper.hpp"

PubSubWrapper::PubSubWrapper() 
    : _pubsub(_client)
    , _connected(false)
    , _host(nullptr)
#if MQTT_SSL
    , _secure(false)
#endif
    , _port(0)
    , _cleanSession(true)
    , _clientId(nullptr)
    , _username(nullptr)
    , _password(nullptr)
    , _willTopic(nullptr)
    , _willPayload(nullptr)
    , _willPayloadLength(0)
    , _willQos(0)
    , _willRetain(false)
{

}


PubSubWrapper& PubSubWrapper::setClientId(const char* clientId) {
  _clientId = clientId;
  return *this;
}

PubSubWrapper& PubSubWrapper::setCleanSession(bool cleanSession) {
  _cleanSession = cleanSession;
  return *this;
}

PubSubWrapper& PubSubWrapper::setMaxTopicLength(uint16_t) {
    return *this;
}

PubSubWrapper& PubSubWrapper::setCredentials(const char* username, const char* password) {
  _username = username;
  _password = password;
  return *this;
}

PubSubWrapper& PubSubWrapper::setWill(const char* topic, uint8_t qos, bool retain, const char* payload, size_t length) {
  _willTopic = topic;
  _willQos = qos;
  _willRetain = retain;
  _willPayload = payload;
  _willPayloadLength = length;
  return *this;
}

PubSubWrapper& PubSubWrapper::setServer(const char* host, uint16_t port) {
  _host = host;
  _port = port;
  return *this;
}

#if ASYNC_TCP_SSL_ENABLED
PubSubWrapper& PubSubWrapper::setSecure(bool secure) {
  _secure = secure;
  return *this;
}

PubSubWrapper& PubSubWrapper::addServerFingerprint(const uint8_t* fingerprint) {
//   std::array<uint8_t, SHA1_SIZE> newFingerprint;
//   memcpy(newFingerprint.data(), fingerprint, SHA1_SIZE);
//   _secureServerFingerprints.push_back(newFingerprint);
  _client.setFingerprint(fingerprint);
  return *this;
}
#endif

PubSubWrapper& PubSubWrapper::onConnect(OnConnectUserCallback callback) {
  _onConnectUserCallbacks.push_back(callback);
  return *this;
}

PubSubWrapper& PubSubWrapper::onDisconnect(OnDisconnectUserCallback callback) {
  _onDisconnectUserCallbacks.push_back(callback);
  return *this;
}

PubSubWrapper& PubSubWrapper::onMessage(OnMessageUserCallback callback) {
  _onMessageUserCallbacks.push_back(callback);
  return *this;
}

PubSubWrapper& PubSubWrapper::onPublish(OnPublishUserCallback callback) {
  _onPublishUserCallbacks.push_back(callback);
  return *this;
}

void PubSubWrapper::connect() {
    _pubsub.setServer(_host, _port);
    _pubsub.setCallback(std::bind(&PubSubWrapper::_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    if (_pubsub.connect(_clientId, _username, _password, _willTopic,_willQos, _willRetain, _willPayload, _cleanSession)) {
         _connected = true;
        for (auto callback : _onConnectUserCallbacks) callback(true);       
    } else {
        for (auto callback : _onDisconnectUserCallbacks) callback(static_cast<AsyncMqttClientDisconnectReason>(_pubsub.state()));
    }
}

void PubSubWrapper::disconnect(bool force) {
    _pubsub.disconnect();
    for (auto callback : _onDisconnectUserCallbacks) callback(static_cast<AsyncMqttClientDisconnectReason>(_pubsub.state()));
}
uint16_t PubSubWrapper::subscribe(const char* topic, uint8_t qos) {
    return _pubsub.subscribe(topic, qos);
}
uint16_t PubSubWrapper::unsubscribe(const char* topic) {
    return _pubsub.unsubscribe(topic);
}
uint16_t PubSubWrapper::publish(const char* topic, uint8_t qos, bool retain, const char* payload, size_t length, bool dup, uint16_t message_id) {
    boolean rc = _pubsub.publish(topic, payload, retain);
    if (!rc) {
        Serial.printf("Could not publish to topic %s\n", topic);
    }
    return rc;
}

void PubSubWrapper::_callback(const char* topic, byte* payload, unsigned int length) {
    for (auto callback : _onMessageUserCallbacks) callback((char*)topic, (char*)payload, nullptr, length, 0, length);
}