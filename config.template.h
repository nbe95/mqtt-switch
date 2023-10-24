// Copyright (c) 2023 Niklas Bettgen

#ifndef CONFIG_H_   // NOLINT
#define CONFIG_H_   // NOLINT


#define SW_VERSION              __DATE__ " " __TIME__

#define MQTT_BROKER             IPAddress(0, 0, 0, 0)
#define MQTT_PORT               1883
#define MQTT_CLIENT_ID          "yun-switch"
#define MQTT_USER               ""
#define MQTT_PASSWORD           ""

#define MQTT_BASE_TOPIC         "yun-switch"
#define MQTT_STATUS_TOPIC       MQTT_BASE_TOPIC "/status"
#define MQTT_COMMAND_TOPIC      MQTT_BASE_TOPIC "/command"
#define MQTT_UPDATE_TIME_MS     1000ul * 60 * 15

#define SERVO_PIN               6
#define SERVO_POS_TOP_DEG       135
#define SERVO_POS_NEUTRAL_DEG   90
#define SERVO_POS_BOTTOM_DEG    45


#endif  // CONFIG_H_  // NOLINT
