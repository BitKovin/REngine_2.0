#pragma once
#include <string>

class Task {
protected:
    std::string taskId;
    bool isActive = true;

public:
    Task(const std::string& id) : taskId(id) {}
    virtual ~Task() = default;

    virtual void Update(float deltaTime) = 0;

    std::string GetTaskId() const { return taskId; }
    bool IsActive() const { return isActive; }

    void SetActive(bool active) { isActive = active; }
};