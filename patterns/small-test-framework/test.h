#pragma once

#include <map>
#include <string>
#include <memory>
#include <functional>
#include <exception>
#include <vector>
#include <stdexcept>

class AbstractTest {
public:
    virtual void SetUp() = 0;
    virtual void TearDown() = 0;
    virtual void Run() = 0;
    virtual ~AbstractTest() {
    }
};

class TestBase {
public:
    TestBase() {
    }
    template <typename T>
    TestBase(T&& func) : ptr_func_(std::move(func)) {
    }

    std::unique_ptr<AbstractTest> GetTest() const {
        if (!test_ptr_) {
            test_ptr_ = ptr_func_();
        }
        return std::move(test_ptr_);
    }

private:
    mutable std::unique_ptr<AbstractTest> test_ptr_;
    std::function<std::unique_ptr<AbstractTest>()> ptr_func_;
};

class TestRegistry {
public:
    template <class TestClass>
    void RegisterClass(const std::string& class_name) {
        vec_tests_[class_name] = std::make_unique<TestBase>(
            TestBase([]() { return std::move(std::unique_ptr<TestClass>(new TestClass())); }));
    }

    std::unique_ptr<AbstractTest> CreateTest(const std::string& class_name) {
        auto it = vec_tests_.find(class_name);
        if (it == vec_tests_.end()) {
            throw std::out_of_range("");
        }
        return it->second->GetTest();
    }

    void RunTest(const std::string& test_name) {
        auto test = CreateTest(test_name);
        test->SetUp();
        try {
            test->Run();
            test->TearDown();
        } catch (...) {
            test->TearDown();
            throw;
        }
    }

    template <class Predicate>
    std::vector<std::string> ShowTests(Predicate callback) const {
        std::vector<std::string> res = ShowAllTests();
        std::vector<std::string> res2;
        std::copy_if(res.begin(), res.end(), std::back_inserter(res2), callback);
        return res2;
    }

    std::vector<std::string> ShowAllTests() const {
        std::vector<std::string> res;
        std::transform(vec_tests_.begin(), vec_tests_.end(), std::back_inserter(res),
                       [](const auto& pair) { return pair.first; });
        return res;
    }

    template <class Predicate>
    void RunTests(Predicate callback) {
        for (const auto& it : vec_tests_) {
            if (callback(it.first)) {
                RunTest(it.first);
            }
        }
    }

    void Clear() {
        vec_tests_.clear();
    }
    static TestRegistry& Instance() {
        if (!sing) {
            sing = std::unique_ptr<TestRegistry>(new TestRegistry);
        }
        return *sing.get();
    }
    static std::unique_ptr<TestRegistry> sing;

private:
    TestRegistry() {
    }
    std::map<std::string, std::unique_ptr<TestBase> > vec_tests_;
};

class FullMatch {
public:
    FullMatch(const std::string& str) : str_(str) {
    }

    bool operator()(const std::string& rhs) {
        return rhs == str_;
    }

private:
    const std::string str_;
};

class Substr {
public:
    Substr(const std::string& str) : str_(str) {
    }
    bool operator()(const std::string& str) const {
        return str.find(str_) != std::string::npos;
    }

private:
    const std::string str_;
};

std::unique_ptr<TestRegistry> TestRegistry::sing = nullptr;
