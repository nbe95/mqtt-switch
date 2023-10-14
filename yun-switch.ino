#include <ArduinoJson.h>
#include <Bridge.h>
#include <Process.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <YunClient.h>

#include "./src/state_machine.h"
#include "./src/timer.h"


#define SW_VERSION              "yun-switch@" __TIMESTAMP__

#define MQTT_BROKER             IPAddress(192, 168, 1, 20)
#define MQTT_PORT               1883
#define MQTT_BASE_TOPIC         "yun-switch"
#define MQTT_STATUS_TOPIC       MQTT_BASE_TOPIC "/status"
#define MQTT_COMMAND_TOPIC      MQTT_BASE_TOPIC "/command"
#define MQTT_UPDATE_TIME_MS     1000ul * 60 * 15

#define SERVO_PIN               6
#define SERVO_POS_TOP_DEG       135
#define SERVO_POS_NEUTRAL_DEG   90
#define SERVO_POS_BOTTOM_DEG    45


YunClient           yun_client;
PubSubClient        mqtt_client(MQTT_BROKER, MQTT_PORT, yun_client);
MotorStateMachine   state_machine(
    SERVO_PIN, SERVO_POS_NEUTRAL_DEG, SERVO_POS_TOP_DEG, SERVO_POS_BOTTOM_DEG
);
DynamicJsonDocument json_buffer(100);
Timer               update_timer(MQTT_UPDATE_TIME_MS);
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

    update_timer.start();
    if (state_machine.hasPosChanged() || update_timer.checkAndRestart()) {
        sendMqttUpdate();
        update_timer.restart();
    }

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

        if (strcasecmp(state, "top") == 0) {
            Serial.println(F("Turning servo to position 'top'."));
            state_machine.setPos(MotorStateMachine::Position::TOP);
            latest_cmd = MotorStateMachine::Position::TOP;
        }
        if (strcasecmp(state, "bottom") == 0) {
            Serial.println(F("Turning servo to position 'bottom'."));
            state_machine.setPos(MotorStateMachine::Position::BOTTOM);
            latest_cmd = MotorStateMachine::Position::BOTTOM;
        }
    }
}

void sendMqttUpdate() {
    json_buffer.clear();
    json_buffer["state"] = "neutral";
    json_buffer["latest_cmd"] = "unknown";
    switch (state_machine.getPos()) {
        case MotorStateMachine::Position::TOP:
            json_buffer["state"] = "top";
            break;
        case MotorStateMachine::Position::BOTTOM:
            json_buffer["state"] = "bottom";
            break;
    }
    switch (latest_cmd) {
        case MotorStateMachine::Position::TOP:
            json_buffer["latest_cmd"] = "top";
            break;
        case MotorStateMachine::Position::BOTTOM:
            json_buffer["latest_cmd"] = "bottom";
            break;
    }
    json_buffer["version"] = SW_VERSION;

    char payload[100];
    serializeJson(json_buffer, payload);
    mqtt_client.publish(MQTT_STATUS_TOPIC, payload);
}

void onMqttConnected() {
    Serial.print(F("Connected to MQTT broker. Subscribing to topic "));
    Serial.println(MQTT_COMMAND_TOPIC);
    mqtt_client.subscribe(MQTT_COMMAND_TOPIC);

    sendMqttUpdate();
    update_timer.restart();
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
