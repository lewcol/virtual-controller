#include "ControllerManager.h"
#include "MockController.h"
#include "messages.pb.h"

ControllerManager::ControllerManager(const std::string& impl)
{
    for (int i = 0; i < max_controllers_; i++) {
        if (impl == "mock") {
            controllers_.emplace_back(std::make_unique<MockController>(i));
        } else if (impl == "linux") {
            // Create LinuxController instances
        }
    }
}

void ControllerManager::startController(int id)
{
    if (validController(id)) {
        controllers_[id]->start();
    }
}

int ControllerManager::getController()
{
    for (auto&& controller : controllers_) {
        if (controller && !controller->isStarted()) {
            startController(controller->getPortNo());
            return controller->getPortNo();
        }
    }
    return -1; // No available controllers
}

void ControllerManager::stopController(int id)
{
    if (validController(id)) {
        controllers_[id]->stop();
    }
}

void ControllerManager::releaseController(int id)
{
    stopController(id);
}

void ControllerManager::updateController(int id, const ButtonUpdate& update)
{
    if (validController(id)) controllers_[id]->update(update);
}