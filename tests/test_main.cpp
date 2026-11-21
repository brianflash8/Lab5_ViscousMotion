#include <exception>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

#include "TestFramework.h"

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    int passed = 0;
    int failed = 0;

    for (const testfw::TestCase& test : testfw::registry())
    {
        try
        {
            test.body();
            ++passed;
            std::cout << "[ OK   ] " << test.name << '\n';
        }
        catch (const std::exception& e)
        {
            ++failed;
            std::cout << "[ FAIL ] " << test.name << "\n         " << e.what() << '\n';
        }
    }

    std::cout << "\nУсього тестів: " << (passed + failed) << ", успішно: " << passed
              << ", з помилками: " << failed << '\n';
    return failed == 0 ? 0 : 1;
}
