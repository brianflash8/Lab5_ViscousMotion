#pragma once

// Мінімальний фреймворк модульних тестів (без зовнішніх залежностей).
// TEST(Назва) { ... }  - реєструє тест; CHECK / CHECK_EQ / CHECK_THROWS - перевірки.

#include <cmath>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace testfw
{

struct TestCase
{
    std::string name;
    std::function<void()> body;
};

inline std::vector<TestCase>& registry()
{
    static std::vector<TestCase> tests;
    return tests;
}

struct Registrar
{
    Registrar(std::string name, std::function<void()> body)
    {
        registry().push_back({std::move(name), std::move(body)});
    }
};

struct CheckFailure : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

template <class A, class B>
std::string describeMismatch(const char* exprA, const char* exprB, const A& a, const B& b,
                             const char* file, int line)
{
    std::ostringstream out;
    out << file << ':' << line << ": " << exprA << " == " << exprB << " не виконується (отримано «" << a
        << "», очікувалось «" << b << "»)";
    return out.str();
}

} // namespace testfw

#define TEST(name)                                         \
    static void name();                                    \
    static testfw::Registrar registrar_##name(#name, name); \
    static void name()

#define CHECK(cond)                                                                           \
    do                                                                                        \
    {                                                                                         \
        if (!(cond))                                                                          \
        {                                                                                     \
            throw testfw::CheckFailure(std::string(__FILE__) + ":" + std::to_string(__LINE__) + \
                                       ": CHECK(" #cond ") не виконується");                  \
        }                                                                                     \
    } while (0)

#define CHECK_EQ(a, b)                                                                          \
    do                                                                                          \
    {                                                                                           \
        const auto va_ = (a); /* копія, а не посилання: вираз може повертати тимчасовий об'єкт */ \
        const auto vb_ = (b);                                                                   \
        if (!(va_ == vb_))                                                                      \
        {                                                                                       \
            throw testfw::CheckFailure(                                                         \
                testfw::describeMismatch(#a, #b, va_, vb_, __FILE__, __LINE__));               \
        }                                                                                       \
    } while (0)

#define CHECK_THROWS(expr, ExceptionType)                                                       \
    do                                                                                          \
    {                                                                                           \
        bool thrown_ = false;                                                                   \
        try                                                                                     \
        {                                                                                       \
            (void)(expr);                                                                       \
        }                                                                                       \
        catch (const ExceptionType&)                                                            \
        {                                                                                       \
            thrown_ = true;                                                                     \
        }                                                                                       \
        if (!thrown_)                                                                           \
        {                                                                                       \
            throw testfw::CheckFailure(std::string(__FILE__) + ":" + std::to_string(__LINE__) + \
                                       ": очікувався виняток " #ExceptionType);                  \
        }                                                                                       \
    } while (0)
