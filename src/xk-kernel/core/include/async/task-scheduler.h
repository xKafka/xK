//
//

#ifndef XK_TASK_SCHEDULER_H
#define XK_TASK_SCHEDULER_H

#include <vector>
#include <thread>
#include <atomic>
#include <type_traits>
#include <future>
#include "task-queue.h"

namespace xk::core::async {
    /** This class represents a scheduler for tasks
     *    it has a thread pool and every thread has it's own queue for tasks to minimize contention
     *    the threads implement task stealing, so if any thread has an empty queue it tries to
     *      obtain a task from other threads */
    class TaskScheduler {
    public:
        /** A type for holding the number of threads and their indexing */
        using ThreadId = uint_fast32_t;
        /** Number of cycles to try to push onto the queue before forcing the task onto the active thread
         *    this should be tuned and set to an appropriate value */
        static constexpr ThreadId ScheduleTryCycles = 1;

        explicit TaskScheduler(ThreadId threadCount = std::thread::hardware_concurrency());
        ~TaskScheduler();

        /** Schedules a task to be executed on the thread pool */
        template<typename Task>
        void schedule(Task &&task)
        {
            // We push the task onto the next queue in the pool
            // We post-increment activeThreadId
            // Relaxed order because we are not obtaining any resources and only keep incrementing
            auto threadId = m_activeThreadId.fetch_add(1, std::memory_order_relaxed);

            // We try to push onto our or any available queue
            for (ThreadId offset = 0; offset < m_threadCount * ScheduleTryCycles; ++offset) {
                const auto index = (threadId + offset) % m_threadCount;

                if (m_taskQueues[index].tryPush(std::forward<Task>(task)))
                    return;
            }

            m_taskQueues[threadId].push(task);
        }

        /** Schedules a task with arguments and returns a future of the return type or an exception */
        template<typename Function, typename ... Args>
        auto async(Function &&function, Args &&...args)
        {
            using Result = decltype(function(std::forward<Args>(args) ...));

            // We bind the arguments to the function and then package it
            auto packagedTask = new std::packaged_task<Result()>(
                    std::bind(std::forward<Function>(function), std::forward<Args>(args) ...)
            );

            auto future = packagedTask->get_future();
            schedule([packagedTask] {
                (*packagedTask)();
                delete packagedTask;
            });
            return future;
        }

    private:
        /** Thread handler for thread threadId */
        void run(ThreadId threadId);

        const ThreadId m_threadCount;
        std::vector<std::thread> m_threads;
        std::vector<TaskQueue> m_taskQueues{m_threadCount};
        std::atomic<ThreadId> m_activeThreadId{0};
    };

    TaskScheduler &DefaultTaskScheduler();
}

#endif //XK_TASK_SCHEDULER_H
