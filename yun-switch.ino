// Copyright (c) 2023 Niklas Bettgen

#include <ArduinoJson.h>
#include <Bridge.h>
#include <Process.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <YunClient.h>

#include "./config.h"
#include "./version.h"
#include "./src/state_machine.h"


YunClient               yun_client;
PubSubClient            mqtt_client(MQTT_BROKER, MQTT_PORT, yun_client);
DynamicJsonDocument     json_buffer(MQTT_JSON_BUFFER);
Timer                   mqtt_reconnect_timer(MQTT_RECONNECT_TIMEOUT);

MotorStateMachine       state_machine(SERVO_PIN, SERVO_POS_NEUTRAL_DEG, SERVO_POS_TOP_DEG, SERVO_POS_BOTTOM_DEG);
MotorStateMachine::Position latest_cmd = MotorStateMachine::Position::NEUTRAL;


void setup() {
    Bridge.begin();
    Serial.begin(115200);
    Serial.println(F("Starting " PROJECT_NAME " v" VERSION "."));

    state_machine.setup();

    mqtt_client.setCallback(onMsgReceived);
    mqttConnect();
}

void loop() {
    // Handle MQTT connection
    if (mqtt_client.connected()) {
        mqtt_client.loop();
    } else {
        mqtt_reconnect_timer.start();
        if (mqtt_reconnect_timer.checkAndRestart()) {
            if (mqttConnect()) {
                mqtt_reconnect_timer.reset();
            }
        }
    }

    // Handle servo state machine
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
        } else if (strcasecmp(state, "bottom") == 0) {
            if (state_machine.setPos(MotorStateMachine::Position::BOTTOM)) {
                latest_cmd = MotorStateMachine::Position::BOTTOM;
                Serial.println(F("Turning servo to position 'bottom'."));
            }
        } else if (json_buffer.containsKey("pos")) {
            const int pos = json_buffer["pos"];
            state_machine.setManualPos(pos);
            Serial.print(F("Turning servo to manual position: "));
            Serial.println(pos);
        }
    }
}

void onStateChanged() {
    if (!mqtt_client.connected()) {
        return;
    }

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

bool mqttConnect() {
    json_buffer.clear();
    json_buffer["state"] = "offline";
    char payload_avail[MQTT_JSON_BUFFER];
    serializeJson(json_buffer, payload_avail);

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
        return true;
    }

    Serial.print(F("failed! RC="));
    Serial.println(mqtt_client.state());
    return false;
}
