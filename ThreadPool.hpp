#ifndef SIMPLE_THREAD_POOL_H
#define SIMPLE_THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>

class ThreadPool {
public:
    using Task = std::function<void()>;

    ThreadPool(uint8_t num_threads) {
        threads.reserve(num_threads);
        for (uint8_t i = 0; i < num_threads; ++i) {
            threads.push_back(std::thread([this] { threadHandler(); }));
        }
    }

    void queue(const Task& task) {
        {
            std::lock_guard<std::mutex> taskQueueLock(taskQueueMutex);
            taskQueue.push(task);
        }
        stopPoolCV.notify_one();
    }

    void stopProcessing() {
        {
            std::lock_guard<std::mutex> taskQueueLock(taskQueueMutex);
            shouldStop = true;
        }
        stopPoolCV.notify_all();
    }

    void waitForTasks() {
        std::unique_lock<std::mutex> taskQueueLock(taskQueueMutex);
        tasksWaitCV.wait(taskQueueLock, [this] { return taskQueue.empty() && tasksInProgress == 0; });
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    ~ThreadPool() noexcept {
        stopProcessing();
        for (auto& thread: threads) {
            thread.join();
        }
    }
private:
    std::vector<std::thread> threads;

    std::queue<Task> taskQueue;
    std::mutex taskQueueMutex;

    std::condition_variable stopPoolCV;
    std::condition_variable tasksWaitCV;
    bool shouldStop = false;
    unsigned int tasksInProgress = 0;

    void threadHandler() {
        while (true) {
            std::unique_lock<std::mutex> taskQueueLock(taskQueueMutex);
            stopPoolCV.wait(taskQueueLock, [this] { return !taskQueue.empty() || shouldStop; });
            if (!taskQueue.empty()) {
                Task task = taskQueue.front();
                taskQueue.pop();
                ++tasksInProgress;
                taskQueueLock.unlock();

                task();

                taskQueueLock.lock();
                --tasksInProgress;
                tasksWaitCV.notify_one();
            } else if (shouldStop) {
                break;
            }
        }
    }
};


#endif //SIMPLE_THREAD_POOL_H