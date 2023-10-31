// Copyright (c) 2023 Niklas Bettgen

#ifndef CONFIG_H_   // NOLINT
#define CONFIG_H_   // NOLINT


#define PROJECT_NAME            "yun-switch"
#define LOCATION_TAG            ""
#define SW_VERSION              __DATE__ " " __TIME__

#define MQTT_BROKER             IPAddress(192, 168, 1, 20)
#define MQTT_PORT               1883
#define MQTT_CLIENT_ID          PROJECT_NAME "_" LOCATION_TAG
#define MQTT_USER               ""
#define MQTT_PASSWORD           ""

#define MQTT_BASE_TOPIC         PROJECT_NAME "/" LOCATION_TAG
#define MQTT_STATE_TOPIC        MQTT_BASE_TOPIC "/state"
#define MQTT_COMMAND_TOPIC      MQTT_BASE_TOPIC "/command"
#define MQTT_AVAIL_TOPIC        MQTT_BASE_TOPIC "/avail"
#define MQTT_RECONNECT_TIMEOUT  1000ul * 5
#define MQTT_JSON_BUFFER        100

#define SERVO_PIN               6
#define SERVO_POS_TOP_DEG       135
#define SERVO_POS_NEUTRAL_DEG   90
#define SERVO_POS_BOTTOM_DEG    49


#endif  // CONFIG_H_  // NOLINT
