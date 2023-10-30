// Copyright (c) 2023 Niklas Bettgen

#include <ArduinoJson.h>
#include <Bridge.h>
#include <Process.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <YunClient.h>

#include "./config.h"
#include "./src/state_machine.h"


YunClient               yun_client;
PubSubClient            mqtt_client(MQTT_BROKER, MQTT_PORT, yun_client);
MotorStateMachine       state_machine(SERVO_PIN, SERVO_POS_NEUTRAL_DEG, SERVO_POS_TOP_DEG, SERVO_POS_BOTTOM_DEG);
DynamicJsonDocument     json_buffer(MQTT_JSON_BUFFER);
MotorStateMachine::Position latest_cmd = MotorStateMachine::Position::NEUTRAL;


void setup() {
    Bridge.begin();
    Serial.begin(115200);
    Serial.println(F("Starting yun-switch sketch."));

    mqtt_client.setCallback(onMsgReceived);

    state_machine.setup();
}

void loop() {
    mqttReconnect();
    mqtt_client.loop();
    state_machine.loop();

    if (state_machine.hasPosChanged()) {
        onStateChanged();
    }
}

void onMsgReceived(char* topic, byte* payload, unsigned int length) {
    Serial.print(F("MQTT message with "));
    Serial.print(length);
    Serial.print(F(" bytes received on topic "));
    Serial.println(topic);

    if (strcasecmp(topic, MQTT_COMMAND_TOPIC) == 0) {
        deserializeJson(json_buffer, payload, length);
        const char* state = json_buffer["switch"];

        if (strcasecmp(state, "top") == 0) {
            if (state_machine.setPos(MotorStateMachine::Position::TOP)) {
                latest_cmd = MotorStateMachine::Position::TOP;
                Serial.println(F("Turning servo to position 'top'."));
            }
        }
        if (strcasecmp(state, "bottom") == 0) {
            if (state_machine.setPos(MotorStateMachine::Position::BOTTOM)) {
                latest_cmd = MotorStateMachine::Position::BOTTOM;
                Serial.println(F("Turning servo to position 'bottom'."));
            }
        }
    }
}

void onStateChanged() {
    json_buffer.clear();
    json_buffer["actual"] = "neutral";
    json_buffer["latest"] = "unknown";
    switch (state_machine.getPos()) {
        case MotorStateMachine::Position::TOP:
            json_buffer["actual"] = "top";
            break;
        case MotorStateMachine::Position::BOTTOM:
            json_buffer["actual"] = "bottom";
            break;
    }
    switch (latest_cmd) {
        case MotorStateMachine::Position::TOP:
            json_buffer["latest"] = "top";
            break;
        case MotorStateMachine::Position::BOTTOM:
            json_buffer["latest"] = "bottom";
            break;
    }

    char payload[MQTT_JSON_BUFFER];
    serializeJson(json_buffer, payload);
    mqtt_client.publish(MQTT_STATE_TOPIC, payload, true);
}

void onMqttConnected() {
    Serial.print(F("Connected to MQTT broker. Subscribing to topic "));
    Serial.println(MQTT_COMMAND_TOPIC);
    mqtt_client.subscribe(MQTT_COMMAND_TOPIC);

    // Update device availability
    json_buffer.clear();
    json_buffer["state"] = "online";
    json_buffer["version"] = SW_VERSION;

    char payload[MQTT_JSON_BUFFER];
    serializeJson(json_buffer, payload);
    mqtt_client.publish(MQTT_AVAIL_TOPIC, payload, true);
}

void mqttReconnect() {
    json_buffer.clear();
    json_buffer["state"] = "offline";
    char payload_avail[MQTT_JSON_BUFFER];
    serializeJson(json_buffer, payload_avail);

    while (!mqtt_client.connected()) {
        Serial.print(F("Attempting MQTT connection... "));
        if (mqtt_client.connect(
            MQTT_CLIENT_ID,
            MQTT_USER,
            MQTT_PASSWORD,
            MQTT_AVAIL_TOPIC,
            0,
            true,
            payload_avail)
        ) {
            Serial.println(F("connected!"));
            onMqttConnected();
        } else {
            Serial.print(F("failed! RC="));
            Serial.print(mqtt_client.state());
            Serial.println(F(" Trying again in 5 seconds."));
            delay(5000);
        }
    }
}
