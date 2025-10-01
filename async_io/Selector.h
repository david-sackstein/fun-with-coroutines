#pragma once

#include "Fd.h"
#include "FdSet.h"

#include <vector>
#include <functional>
#include <chrono>
#include <mutex>
#include <memory>
#include <sys/time.h>

class Selector {
public:

    using FdVector = std::vector<std::reference_wrapper<Fd>>;

    class Work {
    public:
        explicit Work(Selector &s) : s(s) { s.add_work(); }
        ~Work() { s.remove_work(); }
    private:
        Selector &s;
    };

    Selector(FdVector &fds);

    [[nodiscard]] FdVector wait_for_fds();

    std::size_t get_outstanding_work() const { return outstanding_work; }

private:

    FdVector get_ready_fds(const FdSet& fdSet);
    void wait_once(FdSet& fdSet);

    void add_work();
    void remove_work();
    void interrupt_wait();

    FdVector _fds;
    std::unique_ptr<Fd> _wakeup_read;
    std::unique_ptr<Fd> _wakeup_write;
    std::size_t outstanding_work = 0;
    mutable std::mutex mtx;
};