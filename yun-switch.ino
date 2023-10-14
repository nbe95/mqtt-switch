#include <ArduinoJson.h>
#include <Bridge.h>
#include <Process.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <YunClient.h>

#include "./src/state_machine.h"


#define SW_VERSION              "yun-switch@" __TIMESTAMP__

#define MQTT_BROKER             IPAddress(192, 168, 1, 20)
#define MQTT_PORT               1883
#define MQTT_BASE_TOPIC         "yun-switch"
#define MQTT_STATUS_TOPIC       MQTT_BASE_TOPIC "/status"
#define MQTT_COMMAND_TOPIC      MQTT_BASE_TOPIC "/command"
#define MQTT_RESPONSE_TOPIC     MQTT_BASE_TOPIC "/response"

#define SERVO_PIN               6
#define SERVO_POS_TOP_DEG       45
#define SERVO_POS_NEUTRAL_DEG   90
#define SERVO_POS_BOTTOM_DEG    135


YunClient           yun_client;
PubSubClient        mqtt_client(MQTT_BROKER, MQTT_PORT, yun_client);
MotorStateMachine   state_machine(
    SERVO_PIN, SERVO_POS_NEUTRAL_DEG, SERVO_POS_TOP_DEG, SERVO_POS_TOP_DEG
);
DynamicJsonDocument json_buffer(100);


void setup() {
    Bridge.begin();
    Serial.begin(115200);
    Serial.println(F("Starting yun-switch sketch."));

    mqtt_client.setCallback(onMsgReceived);
}

void loop() {
    mqttReconnect();
    mqtt_client.loop();
    state_machine.process();
}

void onMsgReceived(char* topic, byte* payload, unsigned int length) {
    Serial.print(F("MQTT message with "));
    Serial.print(length);
    Serial.print(F(" bytes received on topic "));
    Serial.println(topic);

    if (strcasecmp(topic, MQTT_COMMAND_TOPIC) == 0) {
        deserializeJson(json_buffer, payload, length);
        const char* state = json_buffer["state"];

        Serial.println(state);

        if (strcasecmp(state, "on") == 0) {
            Serial.println(F("Turning servo to position 'top'."));
            state_machine.setPos(MotorStateMachine::Position::TOP);
        }
        if (strcasecmp(state, "off") == 0) {
            Serial.println(F("Turning servo to position 'bottom'."));
            state_machine.setPos(MotorStateMachine::Position::BOTTOM);
        }

        json_buffer.clear();
        json_buffer["state"] = "unknown";
        json_buffer["version"] = SW_VERSION;

        char payload[100];
        serializeJson(json_buffer, payload);
        mqtt_client.publish(MQTT_RESPONSE_TOPIC, payload);
    }
}

void onMqttConnected() {
    json_buffer.clear();
    json_buffer["info"] = F("connected");
    json_buffer["version"] = SW_VERSION;

    char payload[100];
    serializeJson(json_buffer, payload);

    mqtt_client.publish(MQTT_STATUS_TOPIC, payload);

    Serial.print(F("Connected to MQTT broker. Subscribing to topic "));
    Serial.println(MQTT_COMMAND_TOPIC);
    mqtt_client.subscribe(MQTT_COMMAND_TOPIC);
}

void mqttReconnect() {
    while (!mqtt_client.connected()) {
        Serial.print(F("Attempting MQTT connection... "));
        if (mqtt_client.connect("yun-switch")) {
            Serial.println(F("connected!"));
            onMqttConnected();
        } else {
            Serial.print(F("failed! rc="));
            Serial.print(mqtt_client.state());
            Serial.println(F(" Trying again in 5 seconds."));
            delay(5000);
        }
    }
}
