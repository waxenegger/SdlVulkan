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
            
            // wait
            std::unique_lock<std::mutex> lock(this->waitLock);
            this->threadPool->getConditionVariable().wait(lock);
            
            if (!this->active) break;
                         
            // fetch and execute task
            std::unique_ptr<std::function<void(WorkerThread *)>> task = this->threadPool->fetchTask();
            (*task.get())(this);
            this->threadPool->decrementNumberOfTasksPending();
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

VkCommandPool WorkerThread::getCommandPool() {
    return this->pool;
}

void WorkerThread::recycle(VkCommandBuffer & commandBuffer) {
    this->trash.push_back(commandBuffer);
}

void WorkerThread::emptyRecycling() {
    vkFreeCommandBuffers(this->logicalDevice, this->pool, this->trash.size(), this->trash.data());
}

void ThreadPool::start(std::function<VkCommandPool(const VkDevice &, const uint32_t)> poolCreation, const VkDevice & logicalDevice, const uint32_t queueIndex, const uint8_t numberOfThreads) {
    this->threads.resize(numberOfThreads);
    
    uint8_t i=0;
    while(i<numberOfThreads) {
        this->threads[i] = std::make_unique<WorkerThread>(this, "Worker Thread " + std::to_string(i));
        this->threads[i]->start(poolCreation,logicalDevice, queueIndex);
        i++;
    }
}

bool ThreadPool::isActive() {
    if (this->threads.empty()) return false;
    
    bool ret = true;
    
    for (auto & t : this->threads) {
        if (!t->isActive()) return false;
    }    
    
    return ret;
}

void ThreadPool::stop() {   
    if (this->threads.empty()) return;

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

void ThreadPool::addTask(std::unique_ptr<std::function<void(WorkerThread *)>> & task) {
    this->numberOfTasksPending++;
    
    std::lock_guard<std::mutex>lock(this->mutex);

    this->taskQueue.push(std::move(task));
    
    this->tasksAvailable.notify_one();
}

std::unique_ptr<std::function<void(WorkerThread *)>> ThreadPool::fetchTask() {
    std::lock_guard<std::mutex>lock(this->mutex);

    // nothing pending, return dummy
    if (taskQueue.empty()) {
         return std::make_unique<std::function<void(WorkerThread *)>>([](WorkerThread *){});
    }

    // fetch next pending task
    auto ret = std::move(this->taskQueue.front());
    this->taskQueue.pop();

    return ret;
}

std::condition_variable & ThreadPool::getConditionVariable() {
    return this->tasksAvailable;
}

void ThreadPool::decrementNumberOfTasksPending() {
    this->numberOfTasksPending--;
}

void ThreadPool::emptyRecycling() {
    for (auto & t : this->threads) {
        t->emptyRecycling();
    }
}

void ThreadPool::waitForIdle() {
     std::chrono::high_resolution_clock::time_point frameStart = std::chrono::high_resolution_clock::now();
    
    while (true) {
        if (this->numberOfTasksPending == 0) break;
    }
    
    std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> time_span = now -frameStart;
    logInfo("WAIT " + std::to_string(time_span.count()));

}

uint8_t ThreadPool::getNumberOfThreads() {
    return this->threads.size();
}

ThreadPool * ThreadPool::instance = nullptr;
