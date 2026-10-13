#pragma once

#include <vector>
#include <memory>
#include <string>
#include "Controller.h"
/*
public:
    constructor()
    getController()
    releaseController(id)
private:
    startController(id)
    stopController(id)
    vector<Controller> controllers
    num_of_controllers
*/
class ButtonUpdate;

class ControllerManager {
public:
    explicit ControllerManager(const std::string& impl);
    int getController();
    void releaseController(int id);
    void updateController(int id, const ButtonUpdate& update);

private:
    void startController(int id);
    void stopController(int id);
    inline bool validController(int id) const {
        return id >= 0 && id < max_controllers_ && controllers_[id] != nullptr;
    }

    const int max_controllers_ = 4;
    std::vector<std::unique_ptr<Controller>> controllers_;
};