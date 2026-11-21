#include <cmath>

#include "TestFramework.h"
#include "ViscousBody.h"

using namespace physics;

static bool near(double a, double b, double eps) { return std::fabs(a - b) <= eps; }

TEST(Validate_AcceptsDefaults) { validate(Params{}); }

TEST(Validate_RejectsBadParameters)
{
    CHECK_THROWS(validate(Params{0.0, 0.5, 10.0}), std::invalid_argument);
    CHECK_THROWS(validate(Params{1.0, -0.1, 10.0}), std::invalid_argument);
    CHECK_THROWS(validate(Params{1.0, 0.5, 1000.0}), std::invalid_argument);
    CHECK_THROWS(ViscousBody(Params{-1.0, 0.5, 1.0}), std::invalid_argument);
}

TEST(Constructor_StartsWithInitialVelocity)
{
    ViscousBody b(Params{2.0, 1.0, 7.0});
    CHECK(b.velocity() == 7.0);
    CHECK(b.position() == 0.0);
    CHECK(b.time() == 0.0);
}

TEST(Step_RejectsNonPositiveDt)
{
    ViscousBody b;
    CHECK_THROWS(b.step(0.0), std::invalid_argument);
    CHECK_THROWS(b.step(-0.01), std::invalid_argument);
}

TEST(Rk4_MatchesAnalyticSolution)
{
    Params p{2.0, 1.5, 12.0};
    ViscousBody b(p, Method::RK4);
    for (int i = 0; i < 300; ++i) b.step(0.01);
    CHECK(near(b.velocity(), analyticVelocity(p, 3.0), 1e-6));
    CHECK(near(b.position(), analyticPosition(p, 3.0), 1e-4));
}

TEST(Euler_IsLessAccurateThanRk4)
{
    Params p{1.0, 2.0, 10.0};
    ViscousBody e(p, Method::Euler), r(p, Method::RK4);
    for (int i = 0; i < 50; ++i) { e.step(0.04); r.step(0.04); }
    const double ex = analyticVelocity(p, 2.0);
    CHECK(std::fabs(e.velocity() - ex) > std::fabs(r.velocity() - ex));
    CHECK(near(e.velocity(), ex, 0.2)); // але з малим dt Ейлер усе ж близький
}

TEST(Velocity_DecreasesMonotonically)
{
    ViscousBody b(Params{1.0, 1.0, 10.0});
    double prev = b.velocity();
    for (int i = 0; i < 100; ++i)
    {
        b.step(0.05);
        CHECK(b.velocity() < prev);
        CHECK(b.velocity() > 0.0);
        prev = b.velocity();
    }
}

TEST(ZeroDrag_MovesUniformly)
{
    ViscousBody b(Params{1.0, 0.0, 5.0});
    for (int i = 0; i < 100; ++i) b.step(0.1);
    CHECK(near(b.velocity(), 5.0, 1e-12));
    CHECK(near(b.position(), 50.0, 1e-9));
}

TEST(HigherDrag_StopsSooner)
{
    Params low{1.0, 0.2, 10.0}, high{1.0, 2.0, 10.0};
    CHECK(timeToFraction(high, 0.05) < timeToFraction(low, 0.05));
    CHECK(stoppingDistance(high) < stoppingDistance(low));
}

TEST(StoppingDistance_Formula)
{
    Params p{4.0, 2.0, 10.0};
    CHECK(near(stoppingDistance(p), 20.0, 1e-12));
    CHECK(near(timeToFraction(p, 0.5), 2.0 * std::log(2.0), 1e-12));
    CHECK_THROWS(timeToFraction(p, 1.5), std::invalid_argument);
    CHECK(std::isinf(stoppingDistance(Params{1.0, 0.0, 1.0})));
}

TEST(Position_ApproachesStoppingDistance)
{
    Params p{1.0, 1.0, 10.0};
    ViscousBody b(p);
    for (int i = 0; i < 2000; ++i) b.step(0.01);
    CHECK(near(b.position(), stoppingDistance(p), 1e-3));
}

TEST(Reset_RestoresInitialState)
{
    ViscousBody b;
    for (int i = 0; i < 10; ++i) b.step(0.1);
    b.reset();
    CHECK(b.time() == 0.0 && b.position() == 0.0 && b.velocity() == b.params().velocity0);
}

TEST(SetParameters_AreValidatedAndKeepOldValueOnError)
{
    ViscousBody b;
    b.setDrag(3.0);
    CHECK(b.params().drag == 3.0);
    CHECK_THROWS(b.setDrag(-1.0), std::invalid_argument);
    CHECK(b.params().drag == 3.0);
    CHECK_THROWS(b.setMass(0.0), std::invalid_argument);
    b.setInitialVelocity(20.0);
    CHECK(b.velocity() == 10.0); // діє лише після reset()
    b.reset();
    CHECK(b.velocity() == 20.0);
}

TEST(Impulse_ChangesVelocityByImpulseOverMass)
{
    ViscousBody b(Params{2.0, 0.5, 0.0});
    b.applyImpulse(10.0);
    CHECK(near(b.velocity(), 5.0, 1e-12));
}
