// Copyright (c) 2023 Niklas Bettgen

#include "./timer.h" //NOLINT

// Constructor for a timer with or without specific duration
Timer::Timer(const Timer::ms duration) {
    setDuration(duration);
}

// Set the timer duration
void Timer::setDuration(const Timer::ms duration) {
    // Only overwrite if necessary (may be called each cycle!)
    if (duration != m_duration)
        m_duration = duration;
}

// Reset the timer
void Timer::reset() {
    m_start_time = 0;
    m_running = false;
}

// Start the timer if not done yet
bool Timer::start(const Timer::ms duration) {
    if (duration > InvalidDuration)
        setDuration(duration);

    if (isRunning())
        return false;
    m_start_time = getCurrentTime();
    m_running = true;
    return true;
}

// Reset and, if provided, restart the timer with a new duration
bool Timer::restart(const Timer::ms duration) {
    reset();
    return start(duration);
}

// Get the configured timer duration
Timer::ms Timer::getDuration() const {
    return m_duration;
}

// Check wheter the specified duration is already expired
bool Timer::check() const {
    return isRunning() ? getElapsedTime() > getDuration() : false;
}

// Perform a check and immediately restarts the timer - useful for cyclic tasks
bool Timer::checkAndRestart() {
    if (check()) {
        restart();
        return true;
    }
    return false;
}

// Check if the timer is currently running
bool Timer::isRunning() const {
    return m_running;
}

// Check if a valid duration has been set
bool Timer::isSet() const {
    return getDuration() != InvalidDuration;
}

// Return the time when the timer was started
Timer::ms Timer::getStartTime() const {
    return isRunning() ? m_start_time : 0;
}

// Return the amount of elapsed milliseconds
Timer::ms Timer::getElapsedTime() const {
    return isRunning() ? getCurrentTime() - getStartTime() : 0;
}

// Return the amount of relative elapsed time as a float value between 0 and 1
float Timer::getElapsedTimeRel() const {
    if (!getDuration())
        return 0;
    float p = (float)getElapsedTime() / getDuration();
    return max((float)0, min(p, (float)1));   //NOLINT
}

// Internal function to retreive the current µC time
Timer::ms Timer::getCurrentTime() const {
    return millis();
}
