// Copyright (c) 2023 Niklas Bettgen

#include <ArduinoJson.h>
#include <Bridge.h>
#include <Process.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <YunClient.h>

#include "./config.h"
#include "./src/state_machine.h"
#include "./src/lib/timer/timer.h"
#include "./src/lib/debouncer/debouncer.h"


// Git version fallback
#ifndef GIT_VERSION
#define GIT_VERSION __DATE__ " " __TIME__
#endif


YunClient               yun_client;
PubSubClient            mqtt_client(MQTT_BROKER, MQTT_PORT, yun_client);
DynamicJsonDocument     json_buffer(MQTT_JSON_BUFFER);
Timer                   mqtt_reconnect_timer(MQTT_RECONNECT_TIMEOUT);

MotorStateMachine       state_machine(SERVO_PIN, SERVO_POS_NEUTRAL_DEG, SERVO_POS_TOP_DEG, SERVO_POS_BOTTOM_DEG);
MotorStateMachine::Position latest_cmd = MotorStateMachine::Position::NEUTRAL;
DebouncedSwitch         button(BUTTON_PIN, BUTTON_DEBOUNCE_MS, BUTTON_USE_PULLUP);
bool                    latest_cmd_by_mqtt = false;


void setup() {
    Bridge.begin();
    Serial.begin(115200);
    Serial.println(F("Starting " PROJECT_NAME " (version: " GIT_VERSION ")."));

    pinMode(BUTTON_PIN, BUTTON_USE_PULLUP ? INPUT_PULLUP : INPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    state_machine.setup();
    mqtt_client.setCallback(onMsgReceived);
    mqttConnect();
}

void loop() {
    // Handle MQTT connection
    if (mqtt_client.connected()) {
        mqtt_client.loop();
    } else {
        // Indicate connection attempt by flashing LED with 2Hz
        mqtt_reconnect_timer.start();
        const Timer::ms freq = 500;
        digitalWrite(LED_PIN, ((mqtt_reconnect_timer.getElapsedTime() % freq) < freq / 2) ? HIGH : LOW);

        if (mqtt_reconnect_timer.checkAndRestart()) {
            if (mqttConnect()) {
                mqtt_reconnect_timer.reset();
                setLed();
            }
        }
    }

    // Handle servo state machine
    state_machine.loop();
    if (state_machine.hasPosChanged()) {
        onStateChanged();
    }

    // Handle actions triggered by button press
    button.debounce();
    if (button.hasChanged() && button.isClosed()) {
        Serial.println(F("Button was pressed."));
        latest_cmd_by_mqtt = false;
        if (latest_cmd == MotorStateMachine::Position::TOP) {
            setServoBottom();
        } else {
            setServoTop();
        }
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
            latest_cmd_by_mqtt = true;
            setServoTop();
        } else if (strcasecmp(state, "bottom") == 0) {
            latest_cmd_by_mqtt = true;
            setServoBottom();
        } else if (json_buffer.containsKey("pos")) {
            latest_cmd_by_mqtt = true;
            const int pos = json_buffer["pos"];
            setServoToPos(pos);
        }
    }
}

void onStateChanged() {
    // Send MQTT message
    if (mqtt_client.connected()) {
        json_buffer.clear();
        json_buffer["actual"] = "neutral";
        json_buffer["latest"] = "unknown";
        json_buffer["trigger"] = "unknown";
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
        if (latest_cmd_by_mqtt) {
            json_buffer["trigger"] = "mqtt";
        } else {
            json_buffer["trigger"] = "button";
        }

        char payload[MQTT_JSON_BUFFER];
        serializeJson(json_buffer, payload);
        mqtt_client.publish(MQTT_STATE_TOPIC, payload, true);
    }

    // Update LED
    setLed();
}

void onMqttConnected() {
    Serial.print(F("Connected to MQTT broker. Subscribing to topic "));
    Serial.println(MQTT_COMMAND_TOPIC);
    mqtt_client.subscribe(MQTT_COMMAND_TOPIC);

    // Update device availability
    json_buffer.clear();
    json_buffer["state"] = "online";
    json_buffer["version"] = GIT_VERSION;

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

void setServoTop() {
    if (state_machine.setPos(MotorStateMachine::Position::TOP)) {
        latest_cmd = MotorStateMachine::Position::TOP;
        Serial.println(F("Turning servo to position 'top'."));
    } else {
        Serial.println(F("State machine is busy! Not doing anything."));
    }
}

void setServoBottom() {
    if (state_machine.setPos(MotorStateMachine::Position::BOTTOM)) {
        latest_cmd = MotorStateMachine::Position::BOTTOM;
        Serial.println(F("Turning servo to position 'bottom'."));
    } else {
        Serial.println(F("State machine is busy! Not doing anything."));
    }
}

void setServoToPos(const int pos) {
    state_machine.setManualPos(pos);
    Serial.print(F("Turning servo to manual position: "));
    Serial.println(pos);
}

void setLed() {
    digitalWrite(LED_PIN, ((latest_cmd == MotorStateMachine::Position::BOTTOM) ^ LED_ON_TOP) ? HIGH : LOW);
}
