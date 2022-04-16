//
//

#ifndef XGK_TASK_QUEUE_H
#define XGK_TASK_QUEUE_H

#include <deque>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace xgk::core::async {
    /** A task represents a procedure without return values or arguments */
    using TaskHandler = std::function<void()>;

    /** This class represents a queue for holding tasks which can be handled by a worker thread */
    class TaskQueue {
    public:
        using Lock = std::unique_lock<std::mutex>;

        /** Closes the queue so that it doesn't wait for further tasks */
        void close();

        /** Pops a task from the queue to be processed by a worker thread
         *    returns false if the queue is done (empty and closed) */
        bool pop(TaskHandler &taskHandler);

        /** Tries to pop a task from the queue to be processed by a worker thread
         *    returns false if obtaining the mutex fails or the queue is done (empty and closed)
         */
        bool tryPop(TaskHandler &taskHandler);

        /** Pushes a task onto the queue to be processed by a worker thread */
        template<typename Task>
        void push(Task &&task)
        {
            {
                Lock lock(m_mutex);
                m_taskQueue.emplace_back(std::forward<Task>(task));
            }

            m_conditionVariable.notify_one();
        }

        /** Tries to push a task onto the queue to be processed by a worker thread
         *    returns false if obtaining the mutex fails
         */
        template<typename Task>
        auto tryPush(Task &&task) -> bool
        {
            {
                Lock lock(m_mutex, std::try_to_lock);
                if (!lock)
                    return false;

                m_taskQueue.emplace_back(std::forward<Task>(task));
            }

            m_conditionVariable.notify_one();
            return true;
        }

    private:
        std::deque<TaskHandler> m_taskQueue;
        bool m_isClosed{false};
        std::mutex m_mutex;
        std::condition_variable m_conditionVariable;
    };
}

#endif //XGK_TASK_QUEUE_H
