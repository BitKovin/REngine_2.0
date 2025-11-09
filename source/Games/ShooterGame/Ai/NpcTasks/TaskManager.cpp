#include "TaskManager.h"
#include "Task.h"

// Define the static member
std::unordered_map<hashed_string, std::shared_ptr<Task>> TaskManager::tasks;

// Template implementations
template<typename T, typename... Args>
std::shared_ptr<T> TaskManager::CreateTask(const hashed_string& taskId, Args&&... args) {
    static_assert(std::is_base_of<Task, T>::value, "T must derive from Task");

    auto task = std::make_shared<T>(taskId, std::forward<Args>(args)...);
    tasks[taskId] = task;
    return task;
}

std::shared_ptr<Task> TaskManager::GetTask(const hashed_string& taskId) {
    auto it = tasks.find(taskId);
    return it != tasks.end() ? it->second : nullptr;
}

std::shared_ptr<Task> TaskManager::GetTask(const char* taskId) {
    return GetTask(hashed_string(taskId));
}

bool TaskManager::RemoveTask(const hashed_string& taskId) {
    return tasks.erase(taskId) > 0;
}

bool TaskManager::RemoveTask(const char* taskId) {
    return RemoveTask(hashed_string(taskId));
}

void TaskManager::RemoveAllTasks() {
    tasks.clear();
}

bool TaskManager::HasTask(const hashed_string& taskId) {
    return tasks.find(taskId) != tasks.end();
}

bool TaskManager::HasTask(const char* taskId) {
    return HasTask(hashed_string(taskId));
}

std::vector<hashed_string> TaskManager::GetAllTaskIds() {
    std::vector<hashed_string> ids;
    ids.reserve(tasks.size());
    for (const auto& pair : tasks) {
        ids.push_back(pair.first);
    }
    return ids;
}

void TaskManager::Update(float deltaTime) {
    for (auto& pair : tasks) {
        pair.second->Update(deltaTime);
    }
}

size_t TaskManager::GetTaskCount() {
    return tasks.size();
}