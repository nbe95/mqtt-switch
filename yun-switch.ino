#include <Servo.h>

Servo motor;

const int SERVO_PIN = 6;

const int SERVO_POS_TOP_DEG = 70;
const int SERVO_POS_NEUTRAL_DEG = 90;
const int SERVO_POS_BOTTOM_DEG = 110;

void setup() {
    motor.attach(SERVO_PIN);
    motor.write(SERVO_POS_NEUTRAL_DEG);
    delay(5000);
}

void loop() {
    motor.write(SERVO_POS_TOP_DEG);
    delay(1000);

    motor.write(SERVO_POS_NEUTRAL_DEG);
    delay(1000);

    motor.write(SERVO_POS_BOTTOM_DEG);
    delay(1000);
}
