// Copyright (c) 2023 Niklas Bettgen

#define INIT_TIMEOUT        3000
#define SWITCH_TIMEOUT_MS   1500
#define ATTACH_TIMEOUT_MS   200

#include "./state_machine.h" //NOLINT

#include <Servo.h>


MotorStateMachine::MotorStateMachine(
    const int pin,
    const int neutral_deg,
    const int top_deg,
    const int bottom_deg) :
m_pin(pin),
m_neutral_deg(neutral_deg),
m_top_deg(top_deg),
m_bottom_deg(bottom_deg) {}

void MotorStateMachine::setup() {
    m_servo.attach(m_pin);
    m_servo.write(m_neutral_deg);
}

void MotorStateMachine::loop() {
    switch (m_state) {
        case INIT:
            // Wait for the servo to reach initial neutral position
            m_timer.start(INIT_TIMEOUT);
            if (m_timer.check()) {
                m_servo.detach();
                m_state = IDLE;
            }
            break;

        case IDLE:
            // Wait for new jobs
            if (m_target_pos != Position::NEUTRAL) {
                m_timer.reset();
                m_servo.attach(m_pin);
                m_state = ATTACHED;
            }
            break;

        case ATTACHED:
            // Give the motor some time to be properly attached
            m_timer.start(ATTACH_TIMEOUT_MS);
            if (m_timer.check()) {
                m_timer.reset();
                m_servo.write(
                    m_target_pos == Position::TOP ? m_top_deg : m_bottom_deg);

                m_current_pos = m_target_pos;
                m_pos_changed = true;
                m_state = SWITCH_ENGAGED;
            }
            break;

        case SWITCH_ENGAGED:
            // Wait for the motor to switch to top/bottom position
            m_timer.start(SWITCH_TIMEOUT_MS);
            if (m_timer.check()) {
                m_timer.reset();
                m_servo.write(m_neutral_deg);

                m_state = SWITCH_NEUTRAL;
            }
            break;

        case SWITCH_NEUTRAL:
            // Wait for the motor to switch back to neutral position
            m_timer.start(SWITCH_TIMEOUT_MS);
            if (m_timer.check()) {
                m_timer.reset();
                m_servo.detach();

                m_current_pos = Position::NEUTRAL;
                m_pos_changed = true;
                m_state = DETACHED;
            }
            break;

        case DETACHED:
            // Give the motor some time to be properly detached
            m_timer.start(ATTACH_TIMEOUT_MS);
            if (m_timer.check()) {
                m_timer.reset();
                m_target_pos = Position::NEUTRAL;
                m_state = IDLE;
            }
            break;
    }
}

bool MotorStateMachine::setPos(const Position pos) {
    if (m_state == IDLE) {
        m_target_pos = pos;
        return true;
    }
    return false;
}

bool MotorStateMachine::hasPosChanged() {
    if (m_pos_changed) {
        m_pos_changed = false;
        return true;
    }
    return false;
}
