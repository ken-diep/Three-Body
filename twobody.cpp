#include <string>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <iostream>

using namespace std;

// Physical Constants
const double G = 6.674e-11;
const double AU = 1.496e11; // 1 Astronomical Unit (meters)
const double MASS_SUN = 1.989e30;
const double MASS_EARTH = 5.972e24;

enum class OrbitMethod {
    Euler,
    SemiImplicitEuler,
    VelocityVerlet,
    RK4, // (For future expansion)
    NewtonRaphson
};

// Set this variable to swap methods!
OrbitMethod currentMethod = OrbitMethod::SemiImplicitEuler;

struct Vec3 {
    double x, y, z;
    Vec3(double x = 0.0, double y = 0.0, double z = 0.0) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& other) const { return Vec3(x + other.x, y + other.y, z + other.z); }
    Vec3 operator-(const Vec3& other) const { return Vec3(x - other.x, y - other.y, z - other.z); }
    Vec3 operator*(double scalar) const { return Vec3(x * scalar, y * scalar, z * scalar); }
    Vec3 operator/(double scalar) const { return Vec3(x / scalar, y / scalar, z / scalar); }

    double magnitude() const { return sqrt(x*x + y*y + z*z); }

    Vec3 normalize() const {
        double mag = magnitude();
        return (mag > 1e-9) ? Vec3(x/mag, y/mag, z/mag) : Vec3(0.0, 0.0, 0.0);
    }
};

struct Body {
    double mass;
    Vec3 position;
    Vec3 velocity;
    Body(double m, Vec3 p, Vec3 v) : mass(m), position(p), velocity(v) {}
};

// Fixed Gravity Function: Calculate distance AFTER defining the vector
Vec3 calculate_grav_force(const Body& b1, const Body& b2) {
    Vec3 dp = b2.position - b1.position; // Vector pointing to the other body
    double r = dp.magnitude();
    
    if (r < 1e-9) return Vec3(0.0, 0.0, 0.0);
     
    return dp.normalize() * (G * b1.mass * b2.mass / (r * r));
}

// Semi-Implicit Euler for better orbital stability
void update_body(Body& body, const Vec3& acceleration, double dt) {
    body.velocity = body.velocity + acceleration * dt;
    body.position = body.position + body.velocity * dt;
}

void updatePhysics(Body& earth, const Body& sun, double dt, OrbitMethod method) {
    switch (method) {
        case OrbitMethod::Euler: {
            break;
        }

        case OrbitMethod::SemiImplicitEuler: {
            Vec3 a = calculate_grav_force(earth, sun) / earth.mass;
            earth.velocity = earth.velocity + a * dt;
            earth.position = earth.position + earth.velocity * dt;
            break;
        }

        case OrbitMethod::VelocityVerlet: {
            Vec3 a1 = calculate_grav_force(earth, sun) / earth.mass;
            earth.position = earth.position + (earth.velocity * dt) + (a1 * (0.5 * dt * dt));
            Vec3 a2 = calculate_grav_force(earth, sun) / earth.mass;
            earth.velocity = earth.velocity + (a1 + a2) * (0.5 * dt);
            break;
        }

        case OrbitMethod::RK4: {
            break;
        }

        case OrbitMethod::NewtonRaphson: {
            break;
        }

    }
}

int main() {
    //Initialise SFML window
    sf::RenderWindow window(sf::VideoMode({1100, 1000}), "Two-Body Orbit - SFML 3");
    window.setFramerateLimit(60);
    
    sf::Font font;
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) return -1;
    sf::Text hud(font, "", 20);
    hud.setFillColor(sf::Color::White);
    hud.setPosition({10.f, 10.f});

    double dt = 40000.0; 
    Body sun(MASS_SUN, Vec3(0,0,0), Vec3(0,0,0));
    Body earth(MASS_EARTH, Vec3(1.496e11, 0, 0), Vec3(0, 2.978e4, 0));

    // Scaling to fit on screen
    float screenScale = 350.0f / static_cast<float>(AU);
    sf::Vector2f center = {500.0f, 500.0f};

    // 3. Graphical Objects
    sf::CircleShape sunShape(20.f);
    sunShape.setFillColor(sf::Color::Yellow);
    sunShape.setOrigin({20.f, 20.f});
    sunShape.setPosition(center);

    sf::CircleShape earthShape(8.f);
    earthShape.setFillColor(sf::Color(100, 180, 255));
    earthShape.setOrigin({8.f, 8.f});

    // Create a vertex array for the trail
    sf::VertexArray trail(sf::PrimitiveType::LineStrip);

    // Button aesthetics
    sf::RectangleShape button({120.f, 40.f});
    button.setFillColor(sf::Color(50, 50, 50));
    button.setPosition({800.f, 20.f}); // Top right corner

    // Label for the button
    sf::Text btnText(font, "Switch Method", 16);
    btnText.setFillColor(sf::Color::White);
    btnText.setPosition({805.f, 30.f});

      // Button aesthetics
    sf::RectangleShape restartButton({90.f, 40.f});
    restartButton.setFillColor(sf::Color(50, 50, 50));
    restartButton.setPosition({930.f, 20.f}); // Top right corner

    // Label for the button
    sf::Text restartButtonText(font, "Restart", 16);
    restartButtonText.setFillColor(sf::Color::White);
    restartButtonText.setPosition({935.f, 30.f});

    // --- Slider Settings ---
    float trackX = 800.f;
    float trackY = 120.f;      // Positioned below your button
    float trackWidth = 200.f;
    bool isDragging = false;
    double timeMultiplier = 1.0; 

    // --- Slider Shapes ---
    sf::RectangleShape sliderTrack({trackWidth, 4.f});
    sliderTrack.setFillColor(sf::Color(150, 150, 150));
    sliderTrack.setPosition({trackX, trackY});

    sf::CircleShape sliderHandle(10.f);
    sliderHandle.setOrigin({10.f, 10.f}); // Centers the circle on the track
    sliderHandle.setFillColor(sf::Color::White);
    sliderHandle.setPosition({trackX, trackY + 2.f}); 

    sf::Text sliderLabel(font, "Time step: ", 16);
    sliderLabel.setString("Time step: " + std::to_string((int)trunc(dt / 360)) + " hours");

    sliderLabel.setFillColor(sf::Color::White);
    sliderLabel.setPosition({trackX, trackY - 30.f});

    // Optional: Limit trail length so it doesn't grow forever
    const size_t maxTrailPoints = 1000;

    double daysPassed = 0; 

    while (window.isOpen()) {

        // SFML 3 Event Handling (Modern Style)
        while (const std::optional event = window.pollEvent()) {

            // Check for mouse click
            if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseEvent->button == sf::Mouse::Button::Left) {
                    //sf::Vector2f mousePos(static_cast<float>(mouseEvent->position.x), 
                    //                   static_cast<float>(mouseEvent->position.y));
                    
                    sf::Vector2f mousePos = window.mapPixelToCoords({mouseEvent->position.x, mouseEvent->position.y});
                    if (sliderHandle.getGlobalBounds().contains(mousePos)) {
                            isDragging = true;
                        }
                    
                    if (button.getGlobalBounds().contains(mousePos)) {
                        // Button was clicked!
                        // Change your currentMethod here
                    }
                }
            }

            if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (mouseEvent->button == sf::Mouse::Button::Left) {
                    isDragging = false;
                }
            }

            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        if (isDragging) {
            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f mousePos = window.mapPixelToCoords(pixelPos);

            // Keep handle on the track (Clamping)
            float newX = std::max(trackX, std::min(mousePos.x, trackX + trackWidth));
            sliderHandle.setPosition({newX, sliderHandle.getPosition().y});

            // Calculate speed: 1x (left) to 100x (right)
            float percentage = (newX - trackX) / trackWidth;
            timeMultiplier = 1.0 + static_cast<double>(percentage * 99.0);
            
            sliderLabel.setString("Time step: " + std::to_string((int) trunc(dt * timeMultiplier / 360)) + " hours");
        }
        
        // Physics
        //Vec3 force_earth = calculate_grav_force(earth, sun);
        //update_body(earth, force_earth / earth.mass, dt);

        updatePhysics(earth, sun, dt * timeMultiplier, currentMethod);

        daysPassed += dt / 86400.0; // Convert seconds to days

        // --- Update Text ---
        std::string status = "Two-body simulation of Earth orbiting around Sun, using semi-implicit Euler" 
                            "\nDays: " + std::to_string((int)daysPassed) + 
                             "\nVel: " + std::to_string((int)earth.velocity.magnitude()) + " m/s";
        hud.setString(status);

        // --- Rendering (Projecting 3D to 2D) ---
        window.clear(sf::Color(5, 5, 15));

        window.draw(sunShape);

        // Project 3D x and y to 2D screen coordinates
        float ex = center.x + static_cast<float>(earth.position.x) * screenScale;
        float ey = center.y + static_cast<float>(earth.position.y) * screenScale;
        earthShape.setPosition({ex, ey});

        sf::Vector2f currentPos = { ex, ey };
        sf::Color trailColor = sf::Color(0, 255, 255, 100); // Cyan with transparency

        // Explicitly creating the Vertex
        sf::Vertex v{currentPos, trailColor};
        trail.append(v);

        window.draw(earthShape);

        window.draw(trail);
        
        window.draw(hud); // Draw the text last so it's on top

        window.draw(sliderTrack);
        window.draw(sliderLabel);
        window.draw(sliderHandle); // Draw handle last so it's on top

        // Inside the main loop, before window.clear()
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        if (button.getGlobalBounds().contains(window.mapPixelToCoords(mousePos))) {
            button.setFillColor(sf::Color(80, 80, 80)); // Highlight on hover
        } else {
            button.setFillColor(sf::Color(50, 50, 50)); // Normal color
        }

        if (restartButton.getGlobalBounds().contains(window.mapPixelToCoords(mousePos))) {
            restartButton.setFillColor(sf::Color(80, 80, 80)); // Highlight on hover
        } else {
            restartButton.setFillColor(sf::Color(50, 50, 50)); // Normal color
        }

        window.draw(restartButton);
        window.draw(restartButtonText);
        window.draw(button);
        window.draw(btnText);
        window.display();
    }

    return 0;
}