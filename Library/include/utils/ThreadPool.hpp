/*
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  ThreadPool.h
//  Stonefish
//
//  Created by Patryk Cieslak on 14/07/26.
//  Copyright (c) 2026 Patryk Cieslak. All rights reserved.
//

#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <memory>
#include <type_traits>
#include <atomic>

namespace sf
{
    //! A utility class implementing a thread pool.
    class ThreadPool
    {
    public:
        //! A constructor
        explicit ThreadPool(size_t num_threads) : stop_(false), activeTasks_(0)
        {
            for (size_t i = 0; i < num_threads; ++i)
            {
                workers_.emplace_back([this] 
                {
                    while (true) 
                    {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(this->queueMutex_);
                            this->condition_.wait(lock, [this] 
                                {
                                return this->stop_ || !this->tasks_.empty();
                            });
                            
                            if (this->stop_ && this->tasks_.empty()) 
                            {
                                return;
                            }
                            
                            task = std::move(this->tasks_.front());
                            this->tasks_.pop();
                        }
                        
                        // Execute the task
                        task(); 

                        // Decrement the task counter and notify if we are completely done
                        if (--activeTasks_ == 0) 
                        {
                            std::unique_lock<std::mutex> lock(this->queueMutex_);
                            this->waitCondition_.notify_all();
                        }
                    } 
                });
            }
        }

        //! A method used to add tasks to the queue.
        template <class F, class... Args>
        auto enqueue(F &&f, Args &&...args)
            -> std::future<std::invoke_result_t<F, Args...>>
        {

            using return_type = std::invoke_result_t<F, Args...>;

            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...));

            std::future<return_type> res = task->get_future();
            {
                std::unique_lock<std::mutex> lock(queueMutex_);
                if (stop_)
                {
                    throw std::runtime_error("Enqueue on stopped ThreadPool");
                }

                // Increment BEFORE pushing to avoid a race condition where
                // is_idle() returns true before the worker thread can even register the task.
                activeTasks_++;
                tasks_.emplace([task]() { (*task)(); });
            }
            condition_.notify_one();
            return res;
        }

        //! A non-blocking method that returns true if no tasks are queued or running.
        bool isIdle() const
        {
            return activeTasks_ == 0;
        }

        //! A blocking method that halts the caller thread until all current tasks are finished.
        void waitAll()
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            waitCondition_.wait(lock, [this]{ return activeTasks_ == 0; });
        }

        ~ThreadPool()
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            stop_ = true;
            
            condition_.notify_all();
            for (std::thread &worker : workers_)
            {
                if (worker.joinable())
                    worker.detach();
            }
        }

    private:
        std::vector<std::thread> workers_;
        std::queue<std::function<void()>> tasks_;

        std::mutex queueMutex_;
        std::condition_variable condition_;      // For workers waiting for tasks
        std::condition_variable waitCondition_; // For external threads waiting for completion

        bool stop_;
        std::atomic<size_t> activeTasks_; // Tracks pending + currently running tasks
    };

}