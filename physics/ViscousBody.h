#pragma once

// Фізична модель руху тіла у в'язкому середовищі (без залежності від SFML).
//
// Другий закон Ньютона: m * dv/dt = F_зовн - k * v, де k * v - сила опору середовища,
// пропорційна швидкості (k - коефіцієнт опору, кг/с). Після початкового імпульсу
// зовнішня сила не діє (F_зовн = 0), тому dv/dt = -(k/m) * v.
// Аналітичний розв'язок: v(t) = v0 * exp(-k t / m), x(t) = v0 * m / k * (1 - exp(-k t / m)).

#include <stdexcept>

namespace physics
{

enum class Method
{
    Euler, // явний метод Ейлера (перший порядок точності)
    RK4    // метод Рунге-Кутти 4-го порядку
};

struct Params
{
    double mass = 1.0;      // маса тіла, кг (> 0)
    double drag = 0.5;      // коефіцієнт опору k, кг/с (>= 0)
    double velocity0 = 10.0; // початкова швидкість, м/с
};

// Допустимі межі параметрів (використовуються і для обмеження введення в програмі).
inline constexpr double kMinMass = 0.1;
inline constexpr double kMaxMass = 100.0;
inline constexpr double kMaxDrag = 50.0;
inline constexpr double kMaxVelocity = 100.0;

// Перевіряє параметри, кидає std::invalid_argument при некоректних значеннях.
void validate(const Params& p);

// Аналітичні залежності (еталон для порівняння з чисельним розв'язком).
double analyticVelocity(const Params& p, double t);
double analyticPosition(const Params& p, double t);
// Гальмівний шлях до повної зупинки: v0 * m / k (для k = 0 - нескінченність).
double stoppingDistance(const Params& p);
// Час, за який швидкість зменшується до частки fraction від початкової: -(m/k) ln(fraction).
double timeToFraction(const Params& p, double fraction);

class ViscousBody
{
public:
    explicit ViscousBody(const Params& p = Params{}, Method m = Method::RK4);

    // Один крок моделювання з кроком dt > 0.
    void step(double dt);
    // Повернення в початковий стан (час 0, положення 0, швидкість v0).
    void reset();

    // Зміна параметрів "на льоту": маса і опір діють одразу, v0 - при наступному reset().
    void setDrag(double k);
    void setMass(double m);
    void setInitialVelocity(double v0);
    // Миттєвий імпульс сили: v += impulse / m.
    void applyImpulse(double impulse);
    void setMethod(Method m) { method_ = m; }

    double time() const { return t_; }
    double position() const { return x_; }
    double velocity() const { return v_; }
    const Params& params() const { return p_; }
    Method method() const { return method_; }

private:
    Params p_;
    Method method_;
    double t_ = 0.0;
    double x_ = 0.0;
    double v_ = 0.0;
};

} // namespace physics
