#include <coroutine>
#include <iostream>

struct Generator {

    struct promise_type {
        Generator get_return_object() {
            return Generator(std::coroutine_handle<promise_type>::from_promise(*this));
        }

        std::suspend_always initial_suspend() { return std::suspend_always{}; }
        std::suspend_always final_suspend() noexcept { return std::suspend_always{}; }
        void return_void() {}
        void unhandled_exception(){}

        std::suspend_always yield_value(int value) {
            current_value = value;
            return {};
        }

        int current_value;
    };

    Generator(std::coroutine_handle<promise_type> handle) :
        _handle(handle)
    {
    }

    bool next() {
        if (_handle.done()) {
            return false;
        }
        _handle.resume();
        return !_handle.done();
    }

    int get_current_value() {
        auto& promise = _handle.promise();
        return promise.current_value;
    }

    std::coroutine_handle<promise_type> _handle;
};

Generator createGenerator(int count) {
    for (int i=0; i<count; i++) {
        co_yield i;
    }
}

void run_generator() {
    auto generator = createGenerator(10);
    while (generator.next()){
        int value = generator.get_current_value();
        std::cout << value << "\n";
    }
    while (generator.next()){
        int value = generator.get_current_value();
        std::cout << value << "\n";
    }
}
