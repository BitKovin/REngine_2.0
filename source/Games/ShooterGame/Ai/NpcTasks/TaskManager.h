#pragma once
#include <memory>
#include <unordered_map>
#include <vector>
#include <type_traits>
#include <utility/hashed_string.hpp>

class Task;

class TaskManager {
private:
    static std::unordered_map<hashed_string, std::shared_ptr<Task>> tasks;

public:
    // Create task using template
    template<typename T, typename... Args>
    static std::shared_ptr<T> CreateTask(const hashed_string& taskId, Args&&... args);


    // Get task by ID - fastest possible lookup
    static std::shared_ptr<Task> GetTask(const hashed_string& taskId);
    static std::shared_ptr<Task> GetTask(const char* taskId);

    // Remove task by ID
    static bool RemoveTask(const hashed_string& taskId);
    static bool RemoveTask(const char* taskId);

    // Remove all tasks
    static void RemoveAllTasks();

    // Check if task exists
    static bool HasTask(const hashed_string& taskId);
    static bool HasTask(const char* taskId);

    // Get all task IDs (for iteration - use sparingly)
    static std::vector<hashed_string> GetAllTaskIds();

    // Update all tasks
    static void Update(float deltaTime);

    // Get number of active tasks
    static size_t GetTaskCount();
};