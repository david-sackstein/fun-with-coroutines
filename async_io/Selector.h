#pragma once

#include "Fd.h"
#include "FdSet.h"
#include "NotifySignal.h"

#include <vector>
#include <functional>
#include <chrono>
#include <mutex>
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

    std::size_t get_outstanding_work() const { return _outstanding_work; }

private:

    FdVector get_ready_fds(const FdSet& fdSet);
    void wait_for_fds(FdSet& fdSet);
    FdVector with_wakeup_fds() const;

    void add_work();
    void remove_work();

    FdVector _fds;
    NotifySignal _notify;
    std::size_t _outstanding_work = 0;
    mutable std::mutex _mtx;
};