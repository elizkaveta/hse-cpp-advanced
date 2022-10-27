#pragma once

#include <exception>
#include <stdexcept>
#include <cstring>
#include <functional>
#include <memory>

class TryBase {
public:
    void Throw() {
        if (!rand_excp_.empty()) {
            throw std::runtime_error(rand_excp_);
        } else {
            if (exception_) {
                std::rethrow_exception(exception_);
            }
        }
        throw std::logic_error("No exception");
    }

    bool IsFailed() const {
        return exception_ != nullptr || (!rand_excp_.empty());
    }

protected:
    TryBase() {
    }

    TryBase(const std::exception& what) : rand_excp_(std::string(what.what())) {
    }

    TryBase(const std::exception_ptr ex) : exception_(ex) {
    }
    std::string rand_excp_;
    std::exception_ptr exception_;
};
template <class T>
class Try : private TryBase {
public:
    using TryBase::IsFailed;
    using TryBase::Throw;
    using TryBase::TryBase;

    Try() : TryBase() {
    }

    Try(const T& r) : TryBase(), value_(new T(r)) {
    }

    Try(std::exception const& ex) : TryBase(ex) {
    }

    Try(std::exception_ptr ex) : TryBase(ex) {
    }

    const T& Value() const {
        if (!rand_excp_.empty()) {
            throw std::runtime_error(rand_excp_);
        }
        if (value_) {
            return *value_;
        } else {
            throw std::logic_error("Object is empty");
        }
    }

private:
    std::unique_ptr<T> value_;
};

template <>
class Try<void> : private TryBase {
public:
    using TryBase::IsFailed;
    using TryBase::Throw;
    using TryBase::TryBase;
    Try() : TryBase(std::exception()) {
    }

    Try(const Try&) : TryBase() {
    }

    Try(std::exception const& ex) : TryBase(ex) {
    }

    Try(std::exception_ptr ex) : TryBase(ex) {
    }
};

template <class T>
Try<T> TryRunBase(const std::function<Try<T>()>& f) {
    try {
        return f();
    } catch (const std::exception& err) {
        return Try<T>(std::current_exception());
    } catch (const char* err) {
        return Try<T>(std::runtime_error(err));
    } catch (int err) {
        return Try<T>(std::runtime_error(std::strerror(err)));
    } catch (...) {
        return Try<T>(std::logic_error("Unknown exception"));
    }
}

template <class Function, class... Args>
auto TryRun(Function func, Args... args)
    -> Try<typename std::enable_if<!std::is_same<void, decltype(func(args...))>::value,
                                   decltype(func(args...))>::type> {
    return TryRunBase<typeof(func(args...))>(
        [&func, &args...] { return Try<typeof(func(args...))>(func(args...)); });
}

template <class Function, class... Args>
auto TryRun(Function func, Args... args)
    -> Try<typename std::enable_if<std::is_same<void, decltype(func(args...))>::value>::type> {
    return TryRunBase<typeof(func(args...))>([&func, &args...] {
        func(args...);
        return Try<typeof(func(args...))>();
    });
}