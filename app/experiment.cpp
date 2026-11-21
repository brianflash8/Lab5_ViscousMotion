// Обчислювальний експеримент (без графіки): вплив коефіцієнта опору k на рух тіла
// та порівняння чисельних методів. Вивід - таблиці в консоль.
#include <cmath>
#include <cstdio>
#include <initializer_list>

#include "ViscousBody.h"

using namespace physics;

int main()
{
    std::printf("Експеримент 1. Вплив коефіцієнта опору k (m = 1 кг, v0 = 10 м/с, РК4, dt = 0.01 с)\n");
    std::printf("%-6s %-10s %-10s %-10s %-12s %-12s %-12s\n", "k", "v(1c)", "v(3c)", "v(5c)", "t до 5% v0", "шлях x(10c)", "шлях до зупинки");
    for (double k : {0.0, 0.2, 0.5, 1.0, 2.0, 5.0})
    {
        Params p{1.0, k, 10.0};
        ViscousBody b(p, Method::RK4);
        double v1 = 0, v3 = 0, v5 = 0, x10 = 0;
        for (int i = 1; i <= 1000; ++i)
        {
            b.step(0.01);
            if (i == 100) v1 = b.velocity();
            if (i == 300) v3 = b.velocity();
            if (i == 500) v5 = b.velocity();
            if (i == 1000) x10 = b.position();
        }
        const double t5 = timeToFraction(p, 0.05);
        const double s = stoppingDistance(p);
        if (k == 0.0)
            std::printf("%-6.1f %-10.3f %-10.3f %-10.3f %-12s %-12.2f %-12s\n", k, v1, v3, v5, "inf", x10, "inf");
        else
            std::printf("%-6.1f %-10.3f %-10.3f %-10.3f %-12.2f %-12.2f %-12.2f\n", k, v1, v3, v5, t5, x10, s);
    }

    std::printf("\nЕксперимент 2. Вплив маси (k = 1 кг/с, v0 = 10 м/с): стала часу tau = m/k\n");
    std::printf("%-8s %-8s %-14s %-14s\n", "m, кг", "tau, с", "t до 5% v0, с", "шлях до зупинки, м");
    for (double m : {0.5, 1.0, 2.0, 5.0})
    {
        Params p{m, 1.0, 10.0};
        std::printf("%-8.1f %-8.2f %-14.2f %-14.2f\n", m, m / p.drag, timeToFraction(p, 0.05), stoppingDistance(p));
    }

    std::printf("\nЕксперимент 3. Точність методів при t = 3 с (m = 1, k = 2, v0 = 10)\n");
    std::printf("%-8s %-22s %-22s\n", "dt, с", "похибка v: Ейлер", "похибка v: РК4");
    Params p{1.0, 2.0, 10.0};
    const double exact = analyticVelocity(p, 3.0);
    for (double dt : {0.5, 0.1, 0.05, 0.01, 0.001})
    {
        ViscousBody e(p, Method::Euler), r(p, Method::RK4);
        const int n = static_cast<int>(std::lround(3.0 / dt));
        for (int i = 0; i < n; ++i) { e.step(dt); r.step(dt); }
        std::printf("%-8.3f %-22.3e %-22.3e\n", dt, std::fabs(e.velocity() - exact), std::fabs(r.velocity() - exact));
    }

    std::printf("\nЕксперимент 4. Стійкість Ейлера: k*dt/m > 2 -> швидкість осцилює і зростає (m = 1, k = 5, v0 = 10)\n");
    for (double dt : {0.1, 0.3, 0.5})
    {
        Params q{1.0, 5.0, 10.0};
        ViscousBody e(q, Method::Euler);
        for (int i = 0; i < 10; ++i) e.step(dt);
        std::printf("dt = %.1f (k*dt/m = %.1f): v після 10 кроків = %.4f м/с\n", dt, q.drag * dt / q.mass, e.velocity());
    }
    return 0;
}
