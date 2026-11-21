// Графічна програма: рух тіла у в'язкому середовищі (SFML 3).
// Три логічні частини: handleInput() - обробка введення, updatePhysics() - фізична модель,
// drawScene() - відображення.
#include <SFML/Graphics.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "ViscousBody.h"

namespace
{

constexpr unsigned kWidth = 1100;
constexpr unsigned kHeight = 680;
constexpr float kSceneBottom = 330.f;
constexpr float kGroundY = 250.f;
constexpr float kScale = 25.f; // пікселів на метр
constexpr float kGraphLeft = 90.f, kGraphRight = 1060.f, kGraphTop = 380.f, kGraphBottom = 610.f;

struct Sample
{
    double t, vNumeric, vAnalytic;
};

struct App
{
    physics::ViscousBody body;
    physics::Params initial;                     // параметри для порівняння з аналітичним розв'язком
    std::vector<Sample> history;
    std::vector<float> trail;                    // положення тіла в моменти кожних 0.5 с
    double dt = 0.01;
    double accumulator = 0.0;
    double nextTrail = 0.5;
    bool running = true;
    bool shouldClose = false;
};

void resetSimulation(App& a)
{
    a.body.reset();
    a.initial = a.body.params();
    a.history.clear();
    a.trail.clear();
    a.accumulator = 0.0;
    a.nextTrail = 0.5;
}

template <class F>
void tryChange(F&& f)
{
    try { f(); } catch (const std::invalid_argument&) { /* значення поза межами - ігноруємо */ }
}

// ---- 1. Обробка введення ----
void handleInput(sf::RenderWindow& window, App& a)
{
    using Key = sf::Keyboard::Key;
    while (const std::optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
            a.shouldClose = true;
        else if (const auto* key = event->getIf<sf::Event::KeyPressed>())
        {
            const physics::Params& p = a.body.params();
            switch (key->code)
            {
            case Key::Escape: a.shouldClose = true; break;
            case Key::Space: a.running = !a.running; break;
            case Key::R: resetSimulation(a); break;
            case Key::Up: tryChange([&] { a.body.setDrag(p.drag + 0.1); }); break;
            case Key::Down: tryChange([&] { a.body.setDrag(std::max(0.0, p.drag - 0.1)); }); break;
            case Key::W: tryChange([&] { a.body.setMass(p.mass + 0.5); }); break;
            case Key::S: tryChange([&] { a.body.setMass(p.mass - 0.5); }); break;
            case Key::Right: tryChange([&] { a.body.setInitialVelocity(p.velocity0 + 1.0); }); break;
            case Key::Left: tryChange([&] { a.body.setInitialVelocity(p.velocity0 - 1.0); }); break;
            case Key::I: a.body.applyImpulse(10.0); a.initial = a.body.params(); break;
            case Key::M:
                a.body.setMethod(a.body.method() == physics::Method::RK4 ? physics::Method::Euler : physics::Method::RK4);
                break;
            case Key::LBracket: a.dt = std::max(0.001, a.dt / 2.0); break;
            case Key::RBracket: a.dt = std::min(0.5, a.dt * 2.0); break;
            default: break;
            }
        }
        else if (const auto* wheel = event->getIf<sf::Event::MouseWheelScrolled>())
        {
            const double k = a.body.params().drag + 0.1 * wheel->delta;
            tryChange([&] { a.body.setDrag(std::max(0.0, k)); });
        }
    }
}

// ---- 2. Фізична модель ----
void updatePhysics(App& a, double frameTime)
{
    if (!a.running)
        return;
    a.accumulator += std::min(frameTime, 0.1);
    while (a.accumulator >= a.dt)
    {
        a.body.step(a.dt);
        a.accumulator -= a.dt;
        physics::Params p0 = a.initial;
        const double t = a.body.time();
        a.history.push_back({t, a.body.velocity(), physics::analyticVelocity(p0, t)});
        if (t >= a.nextTrail)
        {
            a.trail.push_back(static_cast<float>(a.body.position()));
            a.nextTrail += 0.5;
        }
    }
}

// ---- 3. Відображення ----
std::string fmt(const char* f, double v)
{
    char buf[128];
    std::snprintf(buf, sizeof buf, f, v);
    return buf;
}

void drawText(sf::RenderWindow& w, const sf::Font& font, const std::wstring& s, float x, float y,
              unsigned size = 18, sf::Color c = sf::Color(30, 30, 30))
{
    sf::Text t(font, sf::String(s), size);
    t.setFillColor(c);
    t.setPosition({x, y});
    w.draw(t);
}

std::wstring wfmt(const wchar_t* f, double v)
{
    wchar_t buf[128];
    std::swprintf(buf, 128, f, v);
    return buf;
}

void drawScene(sf::RenderWindow& w, const sf::Font& font, const App& a)
{
    w.clear(sf::Color(245, 247, 250));
    const physics::Params& p = a.body.params();

    // фон «в'язкого середовища»: чим більший опір, тим насиченіший колір
    sf::RectangleShape medium({static_cast<float>(kWidth), kSceneBottom - 40.f});
    medium.setPosition({0.f, 40.f});
    const auto alpha = static_cast<std::uint8_t>(std::min(150.0, 20.0 + p.drag * 12.0));
    medium.setFillColor(sf::Color(60, 140, 220, alpha));
    w.draw(medium);

    // камера: тіло зсувається до 60 % ширини, далі рухається сцена
    const float bodyPx = static_cast<float>(a.body.position()) * kScale;
    const float camPx = std::max(0.f, bodyPx - 0.6f * static_cast<float>(kWidth));
    const float x0 = 60.f - camPx;

    sf::RectangleShape ground({static_cast<float>(kWidth), 4.f});
    ground.setPosition({0.f, kGroundY + 18.f});
    ground.setFillColor(sf::Color(90, 70, 50));
    w.draw(ground);
    for (int m = static_cast<int>(camPx / kScale); m * kScale - camPx < kWidth; ++m)
    {
        const float sx = x0 + static_cast<float>(m) * kScale;
        if (sx < 0.f) continue;
        sf::RectangleShape tick({1.f, (m % 5 == 0) ? 12.f : 6.f});
        tick.setPosition({sx, kGroundY + 22.f});
        tick.setFillColor(sf::Color(90, 70, 50));
        w.draw(tick);
        if (m % 5 == 0)
            drawText(w, font, std::to_wstring(m) + L" м", sx - 8.f, kGroundY + 36.f, 14);
    }

    // позначки положення кожні 0.5 с (за відстанню між ними видно уповільнення)
    for (float tx : a.trail)
    {
        sf::CircleShape dot(3.f);
        dot.setOrigin({3.f, 3.f});
        dot.setPosition({x0 + tx * kScale, kGroundY + 18.f});
        dot.setFillColor(sf::Color(200, 60, 60));
        w.draw(dot);
    }

    sf::CircleShape ball(18.f);
    ball.setOrigin({18.f, 18.f});
    ball.setPosition({x0 + bodyPx, kGroundY});
    ball.setFillColor(sf::Color(230, 120, 30));
    ball.setOutlineThickness(2.f);
    ball.setOutlineColor(sf::Color(120, 60, 10));
    w.draw(ball);

    // вектор швидкості
    const float arrow = std::clamp(static_cast<float>(a.body.velocity()) * 6.f, -300.f, 300.f);
    if (std::fabs(arrow) > 1.f)
    {
        sf::RectangleShape line({std::fabs(arrow), 4.f});
        line.setPosition({x0 + bodyPx + 20.f, kGroundY - 2.f});
        line.setFillColor(sf::Color(20, 120, 60));
        w.draw(line);
    }

    // панель параметрів
    drawText(w, font, L"Рух тіла у в'язкому середовищі (F = -k·v)", 12.f, 6.f, 20, sf::Color(20, 40, 90));
    drawText(w, font, wfmt(L"k = %.2f кг/с", p.drag) + wfmt(L"    m = %.1f кг", p.mass) +
                          wfmt(L"    v0 = %.1f м/с", p.velocity0) + wfmt(L"    dt = %.3f с", a.dt) +
                          (a.body.method() == physics::Method::RK4 ? L"    метод: РК4" : L"    метод: Ейлер") +
                          (a.running ? L"    [іде]" : L"    [пауза]"),
             12.f, 44.f, 17);
    drawText(w, font, wfmt(L"t = %.2f с", a.body.time()) + wfmt(L"    v = %.3f м/с", a.body.velocity()) +
                          wfmt(L"    x = %.2f м", a.body.position()) +
                          wfmt(L"    аналіт. x = %.2f м", physics::analyticPosition(a.initial, a.body.time())),
             12.f, 70.f, 17);
    const double stop = physics::stoppingDistance(a.initial);
    drawText(w, font, L"Шлях до зупинки (аналітично): " + (std::isinf(stop) ? std::wstring(L"∞") : wfmt(L"%.2f м", stop)),
             12.f, 96.f, 17);

    // графік v(t)
    sf::RectangleShape frame({kGraphRight - kGraphLeft, kGraphBottom - kGraphTop});
    frame.setPosition({kGraphLeft, kGraphTop});
    frame.setFillColor(sf::Color::White);
    frame.setOutlineThickness(1.f);
    frame.setOutlineColor(sf::Color(120, 120, 120));
    w.draw(frame);
    drawText(w, font, L"Швидкість v(t): синя - чисельно, зелена - аналітично", kGraphLeft, kGraphTop - 28.f, 16);
    const double tMax = std::max(10.0, a.body.time() * 1.05);
    const double vMax = std::max(1.0, std::fabs(a.initial.velocity0));
    auto px = [&](double t) { return kGraphLeft + static_cast<float>(t / tMax) * (kGraphRight - kGraphLeft); };
    auto py = [&](double v) {
        const float y = kGraphBottom - static_cast<float>(v / vMax) * (kGraphBottom - kGraphTop - 10.f);
        return std::clamp(y, kGraphTop, kGraphBottom); // значення поза графіком обрізаються по рамці
    };
    drawText(w, font, wfmt(L"%.0f", vMax), kGraphLeft - 40.f, kGraphTop, 14);
    drawText(w, font, L"0", kGraphLeft - 16.f, kGraphBottom - 18.f, 14);
    drawText(w, font, wfmt(L"t, с (до %.0f)", tMax), kGraphRight - 110.f, kGraphBottom + 4.f, 14);

    sf::VertexArray num(sf::PrimitiveType::LineStrip), ana(sf::PrimitiveType::LineStrip);
    const std::size_t stride = std::max<std::size_t>(1, a.history.size() / 1500);
    for (std::size_t i = 0; i < a.history.size(); i += stride)
    {
        num.append(sf::Vertex{{px(a.history[i].t), py(a.history[i].vNumeric)}, sf::Color(30, 80, 220)});
        ana.append(sf::Vertex{{px(a.history[i].t), py(a.history[i].vAnalytic)}, sf::Color(30, 170, 70)});
    }
    w.draw(ana);
    w.draw(num);

    drawText(w, font,
             L"Пробіл - пуск/пауза   R - перезапуск   ↑/↓ або колесо миші - k   W/S - маса   ←/→ - v0 (після R)   "
             L"I - імпульс   M - метод   [ ] - dt",
             12.f, kHeight - 30.f, 15, sf::Color(70, 70, 70));
}

struct Options
{
    physics::Params params;
    double advance = 0.0;
    double dt = 0.01;
    physics::Method method = physics::Method::RK4;
};

Options parseArgs(int argc, char** argv)
{
    Options o;
    for (int i = 1; i + 1 < argc; i += 2)
    {
        const std::string key = argv[i];
        const char* val = argv[i + 1];
        if (key == "--k") o.params.drag = std::atof(val);
        else if (key == "--m") o.params.mass = std::atof(val);
        else if (key == "--v0") o.params.velocity0 = std::atof(val);
        else if (key == "--dt") o.dt = std::atof(val);
        else if (key == "--advance") o.advance = std::atof(val);
        else if (key == "--method") o.method = std::strcmp(val, "euler") == 0 ? physics::Method::Euler : physics::Method::RK4;
    }
    return o;
}

} // namespace

int main(int argc, char** argv)
{
    Options opt = parseArgs(argc, argv);
    physics::validate(opt.params);

    sf::Font font;
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf"))
        return 1;

    sf::RenderWindow window(sf::VideoMode({kWidth, kHeight}), "Lab 5: viscous medium (SFML)");
    window.setFramerateLimit(60);

    App app{physics::ViscousBody(opt.params, opt.method), opt.params, {}, {}, 0.01, 0.0, 0.5, true, false};
    app.dt = std::clamp(opt.dt, 0.001, 0.5);
    resetSimulation(app);
    // «перемотка» на задану кількість секунд (для відтворюваних знімків)
    for (double t = 0.0; t < opt.advance; t += app.dt)
        updatePhysics(app, app.dt);

    sf::Clock clock;
    while (window.isOpen() && !app.shouldClose)
    {
        handleInput(window, app);
        updatePhysics(app, clock.restart().asSeconds());
        drawScene(window, font, app);
        window.display();
    }
    return 0;
}
