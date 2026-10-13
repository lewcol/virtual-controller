#pragma once
#include <cstdint>

#include "Controller.h"

class ButtonUpdate;

class MockController final : public Controller {
public:
    MockController(int port_no);
    void update(const ButtonUpdate& update) override;
private:
    void start_impl() override;
    void stop_impl() override;
    std::uint64_t left_analog_x;
    std::uint64_t left_analog_y;
    std::uint64_t right_analog_x;
    std::uint64_t right_analog_y;

    bool start_button_pressed;
    bool select_button_pressed;
    bool a_button_pressed;
    bool b_button_pressed;
    bool x_button_pressed;
    bool y_button_pressed;
    bool left_bumper_pressed;
    bool right_bumper_pressed;
    bool left_trigger_pressed;
    bool right_trigger_pressed;
};
