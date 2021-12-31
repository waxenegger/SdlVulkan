#include "includes/threading.h"

void WorkerThread::start(std::function<VkCommandPool (const VkDevice &, const uint32_t)> poolCreation, const VkDevice & logicalDevice, const uint32_t queueIndex) {
    this->logicalDevice = logicalDevice;
    
    worker = std::thread([this, poolCreation, queueIndex]() {
        this->pool = poolCreation(this->logicalDevice, queueIndex);
        if (this->pool == nullptr) {
            return;
        }
        
        this->active = true;
        
        while(this->active) {
            logInfo("Thread " + this->name);
            std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(50));
        }
        
        vkDestroyCommandPool(this->logicalDevice, this->pool, nullptr);
    });
    
    worker.detach();
}

void WorkerThread::stop() {
    this->active = false;
}

bool WorkerThread::isActive() {
    return this->active;
}

void ThreadPool::start(std::function<VkCommandPool(const VkDevice &, const uint32_t)> poolCreation, const VkDevice & logicalDevice, const uint32_t queueIndex, const uint8_t numberOfThreads) {
    this->threads.resize(numberOfThreads);
    
    uint8_t i=0;
    while(i<numberOfThreads) {
        this->threads[i] = std::make_unique<WorkerThread>("Worker Thread " + std::to_string(i));
        this->threads[i]->start(poolCreation,logicalDevice, queueIndex);
        i++;
    }
}

bool ThreadPool::isActive() {
    bool ret = true;
    
    for (auto & t : this->threads) {
        if (!t->isActive()) return false;
    }    
    
    return ret;
}

void ThreadPool::stop() {   
    for (auto & t : this->threads) {
        t->stop();        
    }
    
    while (true) {
        logInfo("Waiting for Threads to end...");
        std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(50));

        bool allInactive = true;
        for (auto & t : this->threads) {
            if (t->isActive()) allInactive = false;
        }

        if (allInactive) break;
    }
}

ThreadPool * ThreadPool::INSTANCE() {
    if (ThreadPool::instance == nullptr) {
        ThreadPool::instance = new ThreadPool();
    }
    return ThreadPool::instance;
}

ThreadPool::ThreadPool() { }


ThreadPool * ThreadPool::instance = nullptr;
