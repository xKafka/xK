//
// Created by xKafka on 07-Feb-22.
//

#include <async/task-queue.h>

namespace xgk::core::async {
    void TaskQueue::close()
    {
        // We close the queue then notify all waiting threads
        {
            Lock lock(m_mutex);
            m_isClosed = true;
        }
        m_conditionVariable.notify_all();
    }

    bool TaskQueue::pop(TaskHandler &taskHandler)
    {
        Lock lock(m_mutex);

        // While the queue is empty and running we wait for new tasks
        while (m_taskQueue.empty() && !m_isClosed)
            m_conditionVariable.wait(lock);

        // Queue is closed and empty
        if (m_taskQueue.empty())
            return false;

        taskHandler = std::move(m_taskQueue.front());
        m_taskQueue.pop_front();

        return true;
    }

    bool TaskQueue::tryPop(TaskHandler &taskHandler)
    {
        Lock lock(m_mutex, std::try_to_lock);

        // If we failed to lock or the queue is empty we return false
        if (!lock || m_taskQueue.empty())
            return false;

        taskHandler = std::move(m_taskQueue.front());
        m_taskQueue.pop_front();

        return true;
    }
}
