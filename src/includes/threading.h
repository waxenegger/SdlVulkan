#ifndef SRC_INCLUDES_THREADING_H_
#define SRC_INCLUDES_THREADING_H_

#include "shared.h"

class ThreadPool;

class WorkerThread final : std::thread {
    private:
        ThreadPool * threadPool = nullptr;
        std::string name;

        VkDevice logicalDevice = nullptr;
        VkCommandPool pool = nullptr;
        std::vector<VkCommandBuffer> trash;
        
        std::thread worker;
        std::mutex waitLock;
        
        bool active = false;    
    public:
        WorkerThread(const WorkerThread&) = delete;
        WorkerThread& operator=(const WorkerThread &) = delete;
        WorkerThread(WorkerThread &&) = delete;

        WorkerThread(ThreadPool * threadPool, const std::string name) : threadPool(threadPool),name(name) {};
        void start(std::function<VkCommandPool(const VkDevice &, const uint32_t)> poolCreation, const VkDevice & logicalDevice, const uint32_t queueIndex);
        void stop();
        bool isActive();
        
        VkCommandPool getCommandPool();
        void recycle(VkCommandBuffer & commandBuffer);
        void emptyRecycling();
};


class ThreadPool final {
    private:
        static ThreadPool * instance;
        std::vector<std::unique_ptr<WorkerThread>> threads;

        std::queue<std::unique_ptr<std::function<void(WorkerThread *)>>> taskQueue;
        std::mutex mutex;
        std::condition_variable tasksAvailable;
        std::atomic<uint8_t> numberOfTasksPending;
        
        ThreadPool();

    public:
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool &) = delete;
        ThreadPool(ThreadPool &&) = delete;

        static ThreadPool * INSTANCE();                
        VkCommandBuffer process(uint16_t frameIndex);
        
        void start(std::function<VkCommandPool(const VkDevice &, const uint32_t)> poolCreation, const VkDevice & logicalDevice, const uint32_t queueIndex = 0, const uint8_t numberOfThreads = 2);
        void stop();
        bool isActive();
        
        void emptyRecycling();
        
        void addTask(std::unique_ptr<std::function<void(WorkerThread *)>> & task);
        std::condition_variable & getConditionVariable();
        std::unique_ptr<std::function<void(WorkerThread *)>> fetchTask();

        uint8_t getNumberOfThreads();
        void decrementNumberOfTasksPending();
        void waitForIdle();
};

#endif
