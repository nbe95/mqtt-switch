// Copyright (c) 2023 Niklas Bettgen

#ifndef SRC_TIMER_H_
#define SRC_TIMER_H_

#include "Arduino.h"


// Timer class for easy time handling and scheduling
class Timer {
 public:
    // Static constants and type definitions
    typedef uint32_t ms;
    static constexpr ms MaxValue = 0xFFFFFFFF;
    static constexpr ms InvalidDuration = 0;

    // Constructor
    explicit    Timer(const ms duration = InvalidDuration);

    // Timer setup and routines
    void        setDuration(const ms duration);
    void        reset();
    bool        start(const Timer::ms duration = InvalidDuration);
    bool        restart(const Timer::ms duration = InvalidDuration);

    // Timer properties and current status
    ms          getDuration() const;
    bool        check() const;
    bool        checkAndRestart();
    bool        isRunning() const;
    bool        isSet() const;
    ms          getStartTime() const;
    ms          getElapsedTime() const;
    float       getElapsedTimeRel() const;

    // Comparison operators
    bool operator==(const Timer& other) const { return getDuration() == other.getDuration(); }
    bool operator!=(const Timer& other) const { return getDuration() != other.getDuration(); }

    // Conversion operators
    explicit operator bool() const  { return isSet(); }
    operator uint32_t() const       { return (uint32_t)m_duration; }

 protected:
    virtual ms  getCurrentTime() const;

    bool        m_running = false;              // Flag indicating that the timer is running
    ms          m_start_time = 0;               // Start time captured when the timer was started
    ms          m_duration = InvalidDuration;   // Duration of the timer
};

#endif  // SRC_TIMER_H_
