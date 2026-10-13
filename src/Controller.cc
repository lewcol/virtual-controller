#include "Controller.h"

void Controller::start()
{
    if (!started_) {
        start_impl();
        started_ = true;
    }
}

void Controller::stop()
{
    if (started_) {
        stop_impl();
        started_ = false;
    }
}
