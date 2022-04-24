//
// Created by xKafka on 08-Feb-22.
//

#include <async/task-scheduler.h>

namespace xk::core::async {
    TaskScheduler::TaskScheduler(ThreadId threadCount)
            : m_threadCount{threadCount}
            , m_taskQueues(threadCount)
    {
        for (ThreadId id = 0; id < m_threadCount; ++id)
            m_threads.emplace_back([&, id] { run(id); });
    }

    TaskScheduler::~TaskScheduler()
    {
        // We close the queues and wait for them to finish
        for (auto &taskQueue: m_taskQueues)
            taskQueue.close();
        for (auto &thread: m_threads)
            thread.join();
    }

    void TaskScheduler::run(ThreadId threadId)
    {
        // We process tasks until the queue is done (closed and empty)
        for (;;) {
            TaskHandler taskHandler;

            // We try to take a task from our queue, otherwise we try to steal one from others
            for (ThreadId offset = 0; offset < m_threadCount; ++offset) {
                const auto index = (threadId + offset) % m_threadCount;

                if (m_taskQueues[index].tryPop(taskHandler))
                    break;
            }

            // If we failed to steal a task from any queue we wait for our queue or quit
            if (!taskHandler && !m_taskQueues[threadId].pop(taskHandler))
                break;

            taskHandler();
        }
    }

    TaskScheduler &DefaultTaskScheduler()
    {
        static TaskScheduler taskScheduler;
        return taskScheduler;
    }
}
