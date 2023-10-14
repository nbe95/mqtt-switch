// Copyright (c) 2022 Niklas Bettgen

#ifndef SRC_STATE_MACHINE_H_
#define SRC_STATE_MACHINE_H_

#include <Servo.h>
#include "./timer.h"

class MotorStateMachine {
 public:
    enum Position { NEUTRAL, TOP, BOTTOM };

                MotorStateMachine(const int, const int, const int, const int);

    void        setPos(const Position);
    void        process();

 protected:
    enum State {
        INIT,
        IDLE,
        ATTACHED,
        SWITCH_ENGAGED,
        SWITCH_NEUTRAL,
        DETACHED
    }           m_state = State::INIT;
    Position    m_next_pos = Position::NEUTRAL;

    Servo       m_servo;
    const int   m_pin;
    const int   m_neutral_deg;
    const int   m_top_deg;
    const int   m_bottom_deg;

    Timer       m_timer;
};

#endif  // SRC_STATE_MACHINE_H_
