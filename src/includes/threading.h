#ifndef SRC_INCLUDES_THREADING_H_
#define SRC_INCLUDES_THREADING_H_

#include "shared.h"

class WorkerThread final : std::thread {
    private:
        std::string name;

        VkDevice logicalDevice = nullptr;
        VkCommandPool pool = nullptr;
        std::thread worker;
        
        bool active = false;    
    public:
        WorkerThread(const WorkerThread&) = delete;
        WorkerThread& operator=(const WorkerThread &) = delete;
        WorkerThread(WorkerThread &&) = delete;

        WorkerThread(const std::string name) : name(name) {};
        void start(std::function<VkCommandPool(const VkDevice &, const uint32_t)> poolCreation, const VkDevice & logicalDevice, const uint32_t queueIndex);
        void stop();
        bool isActive();
};


class ThreadPool final {
    private:
        static ThreadPool * instance;
        std::vector<std::unique_ptr<WorkerThread>> threads;
        
        ThreadPool();

    public:
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool &) = delete;
        ThreadPool(ThreadPool &&) = delete;

        static ThreadPool * INSTANCE();                
        VkCommandBuffer process(uint16_t frameIndex);
        void start(std::function<VkCommandPool(const VkDevice &, const uint32_t)> poolCreation, const VkDevice & logicalDevice, const uint32_t queueIndex = 0, const uint8_t numberOfThreads = 4);
        void stop();
        bool isActive();
};

#endif

