#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <exception>

class TestFramework {
public:
    struct TestCase {
        std::string name;
        std::function<void()> test;
    };

    static TestFramework& instance() {
        static TestFramework instance;
        return instance;
    }

    void addTest(const std::string& name, std::function<void()> test) {
        tests.push_back({name, test});
    }

    int runAll() {
        int passed = 0;
        int failed = 0;

        std::cout << "Running " << tests.size() << " tests...\n\n";

        for (const auto& test : tests) {
            std::cout << "Running: " << test.name << " ... ";
            try {
                test.test();
                std::cout << "PASSED\n";
                passed++;
            } catch (const std::exception& e) {
                std::cout << "FAILED: " << e.what() << "\n";
                failed++;
            } catch (...) {
                std::cout << "FAILED: Unknown exception\n";
                failed++;
            }
        }

        std::cout << "\nResults: " << passed << " passed, " << failed << " failed\n";
        return failed;
    }

private:
    std::vector<TestCase> tests;
};

// Test macros
#define TEST_CASE(name) \
    static void test_##name(); \
    static int dummy_##name = (TestFramework::instance().addTest(#name, test_##name), 0); \
    static void test_##name()

#define REQUIRE(expr) \
    if (!(expr)) { \
        throw std::runtime_error("Assertion failed: " #expr " at " __FILE__ ":" + std::to_string(__LINE__)); \
    }

#define REQUIRE_EQ(a, b) \
    if ((a) != (b)) { \
        throw std::runtime_error("Assertion failed: " #a " == " #b " at " __FILE__ ":" + std::to_string(__LINE__)); \
    }

#define REQUIRE_NE(a, b) \
    if ((a) == (b)) { \
        throw std::runtime_error("Assertion failed: " #a " != " #b " at " __FILE__ ":" + std::to_string(__LINE__)); \
    }

#endif // TEST_FRAMEWORK_H
