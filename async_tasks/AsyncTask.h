#pragma once

#include "EventLoop.h"

#include <iostream>
#include <thread>

using namespace std::chrono_literals;

struct AsyncTask {
    EventLoop &loop;

    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> h) {
        loop.post([h] {
            std::thread t([h] {
                std::this_thread::sleep_for(1s);
                std::cout << "Resumed in thread " << std::this_thread::get_id() << "\n";
                h.resume();
            });
            t.detach();
        });
    }
    void await_resume() {}
};