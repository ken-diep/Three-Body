#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <thread>   // For std::this_thread::sleep_for
#include <chrono>   // For std::chrono::milliseconds
#include <utility>
#include <fstream>
#include <iomanip>
using namespace std;

//Variables
const double G = 6.674e-11;

// --- ASCII Visualization Parameters ---
const int GRID_WIDTH = 80;
const int GRID_HEIGHT = 40;
// These scaling and offset values will heavily depend on your initial conditions.
// You'll need to adjust them to fit your orbit within the grid.
// For Sun-Earth, Earth's orbit is roughly 1.5e11 meters radius.
// We want to map this to half the grid width/height.
// Max coordinate value (approx) for Earth's orbit: 1.5e11
// Max grid coordinate (approx): GRID_WIDTH/2 = 40
// So, SCALE_FACTOR = (GRID_WIDTH/2) / MaxCoord = 40 / 1.5e11 = approx 2.6e-10
const double SCALE_FACTOR = 2.6e-10; // Adjust this based on your orbital size
const double OFFSET_X = GRID_WIDTH / 2.0;
const double OFFSET_Y = GRID_HEIGHT / 2.0;

struct Vec3 {
    double x, y, z;

    Vec3(double x = 0.0, double y = 0.0, double z = 0.0) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3&other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }

    Vec3 operator*(double scalar) const {
        return Vec3(x*scalar, y*scalar, z*scalar);
    }

    Vec3 operator-(const Vec3&other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    Vec3 operator/(double scalar) const {
        return Vec3(x/scalar, y/scalar, z/scalar);
    }

    double magnitude() const {
        return sqrt(x*x + y*y + z*z);
    }

    Vec3 normalize() const {
        double mag = magnitude();
        if (mag > 1e-9) {
            return Vec3(x/mag, y/mag, z/mag);
        }
        else {
            return Vec3(0.0, 0.0, 0.0);
        }
    }
};

struct Body {
    double mass;
    Vec3 position;
    Vec3 velocity;

    Body(double m, Vec3 p, Vec3 v) : mass(m), position(p), velocity(v) {}
};

Vec3 calculate_grav_force (const Body& b1, const Body& b2) {
    Vec3 dp;
    double r = dp.magnitude();
    dp = b1.position - b2.position;
    if (r < 1e-9) {
        return Vec3(0.0, 0.0, 0.0);
    }
     
    return dp.normalize() * G * b1.mass * b2.mass / (r*r);
}

void update_body (Body& body, const Vec3& acceleration, double dt) {
    body.velocity = body.velocity + acceleration * dt;
    body.position = body.position + body.velocity * dt;
}

//Function to convert world coordinates to grid coordinates
std::pair<int, int> world_to_grid(const Vec3& world_pos) {
    int grid_x = static_cast<int>(world_pos.x * SCALE_FACTOR + OFFSET_X);
    int grid_y = static_cast<int>(world_pos.y * SCALE_FACTOR + OFFSET_Y);
    return {grid_x, grid_y};
}

int main() {
    double dt = 10000.0; //seconds per step
    double total_time = 365 * 24 * 3600;
    int num_steps = static_cast<int>(total_time / dt);

    Body sun(1.989e30, Vec3(0,0,0), Vec3(0,0,0));
    Body earth(5.972e24, Vec3(1.496e11,0,0), Vec3(0, 2.784e4, 0));

    vector<string> all_frames_output;

    for (int i = 0; i < num_steps; ++i) {
        Vec3 force_earth = calculate_grav_force(earth, sun); //force acting on earth
        Vec3 force_sun = calculate_grav_force(sun, earth);

        Vec3 acc_earth = force_earth / earth.mass;
        Vec3 acc_sun = force_sun / sun.mass;

        update_body(earth, acc_earth, dt);
        update_body(sun, acc_sun, dt);

        //Create an empty grid
        std::vector<string> grid(GRID_HEIGHT, string(GRID_WIDTH, ' '));

        //Place bodies on grid
        std::pair<int, int> sun_pos_grid = world_to_grid(sun.position);
        std::pair<int, int> earth_pos_grid = world_to_grid(earth.position);

        //Ensure the positions are within grid bounds
        if (sun_pos_grid.first >= 0 && sun_pos_grid.first < GRID_WIDTH &&
            sun_pos_grid.second >= 0 && sun_pos_grid.second < GRID_HEIGHT) {
                grid[sun_pos_grid.second][sun_pos_grid.first] = 'S';
            }
        if (earth_pos_grid.first >= 0 && earth_pos_grid.first < GRID_WIDTH &&
            earth_pos_grid.second >= 0 && earth_pos_grid.second < GRID_HEIGHT) {
                grid[earth_pos_grid.second][earth_pos_grid.first] = 'E';
            }

        string current_frame_str;

        for (const auto& row : grid) {
            current_frame_str += row + "\n";
        }
        all_frames_output.push_back(current_frame_str);

        //Generate HTML file
        ofstream html_file("orbital_simulation.html");
        if (!html_file.is_open()) {
            return 1;
        }

    html_file << R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Two-Body Orbital Simulation (ASCII)</title>
    <script src="https://cdn.tailwindcss.com"></script>
    <style>
        body {
            font-family: 'Inter', monospace;
            background-color: #1a202c;
            color: #e2e8f0;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            margin: 0;
            overflow: hidden;
        }
        .container {
            background-color: #2d3748;
            padding: 1.5rem;
            border-radius: 0.75rem;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            text-align: center;
            max-width: 90%;
            width: fit-content;
        }
        pre {
            background-color: #1a202c;
            color: #48bb78;
            padding: 1rem;
            border-radius: 0.5rem;
            overflow: auto;
            white-space: pre;
            font-size: 0.75rem;
            line-height: 1;
            margin-top: 1rem;
            min-width: 80ch;
            min-height: 40em;
            max-height: 80vh;
        }
        h1 {
            font-size: 1.5rem;
            font-weight: bold;
            margin-bottom: 1rem;
            color: #cbd5e0;
        }
        .info {
            font-size: 0.875rem;
            color: #a0aec0;
            margin-bottom: 0.5rem;
        }
        .controls {
            margin-top: 1rem;
            display: flex;
            justify-content: center;
            gap: 1rem;
        }
        button {
            background-color: #48bb78;
            color: #1a202c;
            padding: 0.5rem 1rem;
            border-radius: 0.375rem;
            font-weight: bold;
            cursor: pointer;
            transition: background-color 0.2s;
        }
        button:hover {
            background-color: #38a169;
        }
        input[type="range"] {
            -webkit-appearance: none;
            width: 100%;
            height: 8px;
            background: #4a5568;
            border-radius: 5px;
            outline: none;
            opacity: 0.7;
            transition: opacity .2s;
            margin-top: 0.5rem;
        }
        input[type="range"]:hover {
            opacity: 1;
        }
        input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 20px;
            height: 20px;
            background: #48bb78;
            border-radius: 50%;
            cursor: pointer;
        }
        input[type="range"]::-moz-range-thumb {
            width: 20px;
            height: 20px;
            background: #48bb78;
            border-radius: 50%;
            cursor: pointer;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Two-Body Orbital Simulation (ASCII)</h1>
        <div class="info">
            <p>Euler Integration Method</p>
            <p>Simulation Time Step (dt): )" << std::fixed << std::setprecision(2) << dt << R"( seconds</p>
            <p>Total Simulated Time: )" << std::fixed << std::setprecision(0) << total_time / (24 * 3600) << R"( days</p>
            <p>Frames Generated: )" << all_frames_output.size() << R"(</p>
        </div>
        <pre id="simulationOutput">Loading simulation...</pre>
        <div class="controls">
            <button id="playPauseBtn">Play</button>
            <button id="resetBtn">Reset</button>
            <label for="speedSlider">Speed:</label>
            <input type="range" id="speedSlider" min="10" max="1000" value="100">
        </div>
    </div>

    <script>
        const simulationOutput = document.getElementById('simulationOutput');
        const playPauseBtn = document.getElementById('playPauseBtn');
        const resetBtn = document.getElementById('resetBtn');
        const speedSlider = document.getElementById('speedSlider');

        // This array will be populated by the C++ code
        const frames = [
)";

    // Write each frame into the JavaScript array
    for (size_t i = 0; i < all_frames_output.size(); ++i) {
        html_file << "            `"; // Use backticks for template literal
        // Escape backticks and dollar signs if they appear in your ASCII art
        std::string escaped_frame = all_frames_output[i];
        size_t pos = escaped_frame.find('`');
        while (pos != std::string::npos) {
            escaped_frame.replace(pos, 1, "\\`");
            pos = escaped_frame.find('`', pos + 2);
        }
        pos = escaped_frame.find('$');
        while (pos != std::string::npos) {
            escaped_frame.replace(pos, 1, "\\$");
            pos = escaped_frame.find('$', pos + 2);
        }
        html_file << escaped_frame;
        html_file << "`";
        if (i < all_frames_output.size() - 1) {
            html_file << ",\n";
        }
    }

    html_file << R"(
        ];

        let currentFrameIndex = 0;
        let animationInterval;
        let isPlaying = false;
        let animationSpeed = 100; // milliseconds per frame

        function displayFrame() {
            if (frames.length === 0) {
                simulationOutput.textContent = "No simulation data generated.";
                return;
            }
            simulationOutput.textContent = frames[currentFrameIndex];
            currentFrameIndex = (currentFrameIndex + 1) % frames.length;
        }

        function startAnimation() {
            if (!isPlaying && frames.length > 0) {
                isPlaying = true;
                playPauseBtn.textContent = 'Pause';
                animationInterval = setInterval(displayFrame, animationSpeed);
            }
        }

        function pauseAnimation() {
            if (isPlaying) {
                isPlaying = false;
                playPauseBtn.textContent = 'Play';
                clearInterval(animationInterval);
            }
        }

        function resetAnimation() {
            pauseAnimation();
            currentFrameIndex = 0;
            displayFrame(); // Display the first frame
            playPauseBtn.textContent = 'Play'; // Reset button text
        }

        playPauseBtn.addEventListener('click', () => {
            if (isPlaying) {
                pauseAnimation();
            } else {
                startAnimation();
            }
        });

        resetBtn.addEventListener('click', resetAnimation);

        speedSlider.addEventListener('input', (event) => {
            animationSpeed = 1000 - event.target.value; // Invert slider for intuitive speed control
            if (isPlaying) {
                pauseAnimation();
                startAnimation(); // Restart with new speed
            }
        });

        // Initial display and start animation
        displayFrame();
        startAnimation(); // Auto-play when loaded
    </script>
</body>
</html>)";

    html_file.close();

        /*
        if (i % 200 == 0) {
            cout << "Time: " << i * dt / (24 * 3600) << " days\n";
            cout << "Earth position: (" << earth.position.x << ", " << earth.position.y << ", " << earth.position.z << ")\n";
            cout << "Sun position: (" << sun.position.x << ", " << sun.position.y << ", " << sun.position.z << ")\n";
        }
        */
    }
    return 0;
}

