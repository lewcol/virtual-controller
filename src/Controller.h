/*
class controller
public:
    constructor(id)
    virtual updateState()
private:
    button_state
    port_no
*/
#pragma once

class ButtonUpdate;

class Controller {
public:
    Controller(int port_no) : port_no_(port_no) {}
    void start();
    void stop();
    virtual void update(const ButtonUpdate& update) = 0;
    inline int getPortNo() const { return port_no_; }
    inline bool isStarted() const { return started_; }
private:
    virtual void start_impl() = 0;
    virtual void stop_impl() = 0;
    int port_no_;
    bool started_ = false;
};