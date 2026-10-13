#include "MockController.h"
#include "messages.pb.h"

MockController::MockController(int port_no) : Controller(port_no)
{
    printf("MockController created on port %d\n", getPortNo());
}

void MockController::update(const ButtonUpdate& update)
{
    left_analog_x = update.left_analog_x();
    left_analog_y = update.left_analog_y();
    right_analog_x = update.right_analog_x();
    right_analog_y = update.right_analog_y();

    start_button_pressed = update.start_button_pressed();
    select_button_pressed = update.select_button_pressed();
    a_button_pressed = update.a_button_pressed();
    b_button_pressed = update.b_button_pressed();
    x_button_pressed = update.x_button_pressed();
    y_button_pressed = update.y_button_pressed();
    left_bumper_pressed = update.left_bumper_pressed();
    right_bumper_pressed = update.right_bumper_pressed();
    left_trigger_pressed = update.left_trigger_pressed();
    right_trigger_pressed = update.right_trigger_pressed();

    printf("MockController Update:\n");
    printf("Left Analog: (%llu, %llu)\n", left_analog_x,
              left_analog_y);
    printf("Right Analog: (%llu, %llu)\n", right_analog_x,
              right_analog_y);
    printf("Buttons: Start(%d), Select(%d), A(%d), B(%d), X(%d), Y(%d), LB(%d), RB(%d), LT(%d), RT(%d)\n",
              start_button_pressed, select_button_pressed,
              a_button_pressed, b_button_pressed,
              x_button_pressed, y_button_pressed,
              left_bumper_pressed, right_bumper_pressed,
              left_trigger_pressed, right_trigger_pressed);
}

void MockController::start_impl()
{
    printf("MockController started on port %d\n", getPortNo());
}

void MockController::stop_impl()
{
    printf("MockController stopped on port %d\n", getPortNo());
}