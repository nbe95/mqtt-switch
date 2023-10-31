// Copyright (c) 2023 Niklas Bettgen

#ifndef SRC_STATE_MACHINE_H_
#define SRC_STATE_MACHINE_H_

#include <Servo.h>
#include "./timer.h"

class MotorStateMachine {
 public:
    enum Position { NEUTRAL, TOP, BOTTOM };

    MotorStateMachine(const int, const int, const int, const int);

    void        setup();
    void        loop();

    bool        setPos(const Position);
    Position    getPos() const { return m_current_pos; }
    bool        hasPosChanged();

    bool        setManualPos(const int);

 protected:
    enum State {
        INIT,
        IDLE,
        ATTACHED,
        SWITCH_ENGAGED,
        SWITCH_NEUTRAL,
        DETACHED,
        MANUAL
    }           m_state = State::INIT;
    Position    m_current_pos = Position::NEUTRAL;
    Position    m_target_pos = Position::NEUTRAL;
    bool        m_pos_changed = false;
    int         m_manual_deg = -1;

    Servo       m_servo;
    const int   m_pin;
    const int   m_neutral_deg;
    const int   m_top_deg;
    const int   m_bottom_deg;

    Timer       m_timer;
};

#endif  // SRC_STATE_MACHINE_H_
