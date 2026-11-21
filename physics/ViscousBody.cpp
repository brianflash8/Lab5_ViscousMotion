#include "ViscousBody.h"

#include <cmath>
#include <limits>
#include <string>

namespace physics
{

void validate(const Params& p)
{
    if (!(p.mass >= kMinMass && p.mass <= kMaxMass))
        throw std::invalid_argument("mass out of range [" + std::to_string(kMinMass) + ", " + std::to_string(kMaxMass) + "]");
    if (!(p.drag >= 0.0 && p.drag <= kMaxDrag))
        throw std::invalid_argument("drag out of range [0, " + std::to_string(kMaxDrag) + "]");
    if (!(std::fabs(p.velocity0) <= kMaxVelocity))
        throw std::invalid_argument("initial velocity out of range");
}

double analyticVelocity(const Params& p, double t)
{
    return p.velocity0 * std::exp(-p.drag * t / p.mass);
}

double analyticPosition(const Params& p, double t)
{
    if (p.drag == 0.0)
        return p.velocity0 * t;
    return p.velocity0 * p.mass / p.drag * (1.0 - std::exp(-p.drag * t / p.mass));
}

double stoppingDistance(const Params& p)
{
    if (p.drag == 0.0)
        return std::numeric_limits<double>::infinity();
    return p.velocity0 * p.mass / p.drag;
}

double timeToFraction(const Params& p, double fraction)
{
    if (!(fraction > 0.0 && fraction < 1.0))
        throw std::invalid_argument("fraction must be in (0, 1)");
    if (p.drag == 0.0)
        return std::numeric_limits<double>::infinity();
    return -(p.mass / p.drag) * std::log(fraction);
}

ViscousBody::ViscousBody(const Params& p, Method m) : p_(p), method_(m)
{
    validate(p_);
    reset();
}

void ViscousBody::reset()
{
    t_ = 0.0;
    x_ = 0.0;
    v_ = p_.velocity0;
}

void ViscousBody::step(double dt)
{
    if (!(dt > 0.0))
        throw std::invalid_argument("dt must be positive");

    const double a = p_.drag / p_.mass; // dv/dt = -a * v
    auto f = [a](double v) { return -a * v; };

    double vNew = v_;
    double dx = 0.0;
    if (method_ == Method::Euler)
    {
        vNew = v_ + dt * f(v_);
        dx = v_ * dt; // явний Ейлер: положення за швидкістю на початку кроку
    }
    else
    {
        const double k1 = f(v_);
        const double k2 = f(v_ + 0.5 * dt * k1);
        const double k3 = f(v_ + 0.5 * dt * k2);
        const double k4 = f(v_ + dt * k3);
        vNew = v_ + dt / 6.0 * (k1 + 2.0 * k2 + 2.0 * k3 + k4);
        dx = 0.5 * (v_ + vNew) * dt; // інтеграл швидкості методом трапецій
    }
    v_ = vNew;
    x_ += dx;
    t_ += dt;
}

void ViscousBody::setDrag(double k)
{
    Params n = p_;
    n.drag = k;
    validate(n);
    p_ = n;
}

void ViscousBody::setMass(double m)
{
    Params n = p_;
    n.mass = m;
    validate(n);
    p_ = n;
}

void ViscousBody::setInitialVelocity(double v0)
{
    Params n = p_;
    n.velocity0 = v0;
    validate(n);
    p_ = n;
}

void ViscousBody::applyImpulse(double impulse)
{
    v_ += impulse / p_.mass;
}

} // namespace physics
