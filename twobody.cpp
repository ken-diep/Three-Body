#include <string>
#include <cmath>
#include <SFML/Graphics.hpp>

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
    RK4 // (For future expansion)
};

// Set this variable to swap methods!
OrbitMethod currentMethod = OrbitMethod::Euler;

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
    
}

int main() {
    //Initialise SFML window
    sf::RenderWindow window(sf::VideoMode({1000, 1000}), "Two-Body Orbit - SFML 3");
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

    // Optional: Limit trail length so it doesn't grow forever
    const size_t maxTrailPoints = 1000;

    double daysPassed = 0; 

    while (window.isOpen()) {

        // SFML 3 Event Handling (Modern Style)
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }
        // Physics
        Vec3 force_earth = calculate_grav_force(earth, sun);
        update_body(earth, force_earth / earth.mass, dt);

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
        window.display();
    }

    return 0;
}