/** n-bodies simulation
 * q = QUIT
 * p = toggle PAUSE
 * f = reduce precision (faster computation)
 * s = increase precision (slower computation)
 */

#include "rico.hpp"
#include "random.hpp"
#include <cmath>
#include <vector>

constexpr double G = 1.0; //6.674e-11; // gravitational constant
constexpr uint8_t fading = 2; // trace fading speed

using vec = rico::Tvec2D<double>;

struct Body {
	double mass;
	vec position;
	vec speed;
	rico::Color color;
	int8_t radius;

	constexpr Body(double _mass, vec _position, vec _speed, rico::Color _color, int8_t _radius) :
		mass(_mass), position(_position), speed(_speed), color(_color), radius(_radius)
	{}

	// random body
	Body(void) :
		Body(
			std::pow(10.0, Random::rangeDouble(0.0, 3.0)),
			vec(Random::rangeDouble(-1.0, +1.0), Random::rangeDouble(-1.0, +1.0)),
			vec(Random::rangeDouble(-1e1, +1e1), Random::rangeDouble(-1e1, +1e1)),
			rico::BLACK,
			0
		)
	{
		uint32_t mask = Random::rangeUint(1, 6);
		uint8_t r = 255 * (mask % 2); mask >>= 1;
		uint8_t g = 255 * (mask % 2); mask >>= 1;
		uint8_t b = 255 * (mask % 2);
		color = rico::Color(r, g, b);
		radius = 0;
		if (mass >  10.0) radius = 1;
		if (mass > 100.0) radius = 2;
	}

	// return the distance between *this and other
	double distance(Body const& other) const {
		double delta_x = this->position.x - other.position.x;
		double delta_y = this->position.y - other.position.y;
		double d = std::hypot(delta_x, delta_y);
		// do not return 0.0
		return std::max(d, 5e-2);
	}

	// gravitational force that other apply to *this
	vec force(Body const& other) const {
		// direction of the force
		vec AB = other.position - this->position;
		// distance (has a minimal value, lead to energy loss in the system)
		double d = distance(other);
		// proportionality constant (gravity formula)
		double C = (G * this->mass * other.mass) / (d*d);
		// force is equivalent to Cu (where u is the unit vector of AB)
		return (AB / d) * C;
	}

	// apply the force for the given duration
	void apply(vec const& force, double delta_time) {
		// Newton's law
		vec acceleration = force / this->mass;
		// after integration, delta_speed = acceleration*t
		speed += acceleration * delta_time;
		// after integration, delta_position = speed*t+(acceleration/2)*t^2
		position += (speed + ((acceleration * delta_time) / 2.0)) * delta_time;
	}

	// draw body
	void draw(void) const {
		// map [-1.0, +1.0] to [0, +width]
		int32_t x = static_cast<int32_t>(((1.0 + position.x) / 2.0) * static_cast<double>(rico::GameEngine::GetWidth()));
		// map [-1.0, +1.0] to [0, +height]
		int32_t y = static_cast<int32_t>(((1.0 + position.y) / 2.0) * static_cast<double>(rico::GameEngine::GetHeight()));
		// for each pixel in the radius
		for (int32_t col = x - radius; col <= x + radius; ++col) {
			for (int32_t row = y - radius; row <= y + radius; ++row) {
				// convert to position
				if (row < 0 || col < 0) continue;
				rico::Position pos(static_cast<uint32_t>(col), static_cast<uint32_t>(row));
				// try to display pixel
				try {
					rico::GameEngine::SetPixel(pos, color);
				} catch (std::out_of_range const&) {
					// do nothing on error (pixel out of screen)
				}
			}
		}
	}

}; // struct Body

/*
// 3 bodies, similar to Sun Earth and Moon
constexpr Body Sun   {1.989e30, {0.0, 0.0                       }, {0.0, 0.0    }, rico::YELLOW,               5};
constexpr Body Earth {5.972e24, {0.0, 1.496e11                  }, {0.0, 2.978e4}, rico::BLUE,                 2};
constexpr Body Moon  {7.342e22, {0.0, Earth.position.y + 3.847e5}, {0.0, 1.022e3}, rico::Color(128, 128, 128), 1};
*/

class NBodies : public rico::Game {
private:
	std::vector<Body> bodies;
	std::vector<vec> forces;
	bool pause;
	double delta_time;

	void DarkenScreen(void) const {
		rico::Color color;
		rico::Position pos;
		for (uint32_t row = 0; row < Height(); ++row) {
			for (uint32_t col = 0; col < Width(); ++col) {
				pos = {col, row};
				if(GetPixel(pos, &color)) {
					color.r = (color.r > fading) ? color.r - fading : 0;
					color.g = (color.g > fading) ? color.g - fading : 0;
					color.b = (color.b > fading) ? color.b - fading : 0;
					SetPixel(pos, color);
				}
			}
		}
	}

	void UpdatePositions(void) {
		uint32_t i, j, n = bodies.size();
		// compute force for each body
		for (i = 0; i < n; ++i) {
			forces[i] = vec();
			for (j = 0; j < n; ++j) {
				if (i != j) {
					forces[i] += bodies[i].force(bodies[j]);
				}
			}
		}
		// apply forces to bodies
		for (i = 0; i < n; ++i) {
			bodies[i].apply(forces[i], delta_time);
		}
	}

protected:

	bool OnUserCreate(int argc, char const **argv) override {
		(void) argc; (void) argv;
		if (Width() != Height()) return false;
		Clear(rico::BLACK);

		// insert random bodies
		bodies = std::vector<Body>(3);
		// insert 2 massive bodies orbiting the center
		bodies.push_back(Body(5e3, {-0.25, 0.0}, {0.0, -75.0}, rico::WHITE, 4));
		bodies.push_back(Body(5e3, {+0.25, 0.0}, {0.0, +75.0}, rico::WHITE, 4));
		// insert 2 light bodies orbiting the center
		bodies.push_back(Body(1e2, {+0.75, 0.0}, {0.0, -100.0}, rico::GREEN, 2));
		bodies.push_back(Body(1e2, {-0.75, 0.0}, {0.0, +100.0}, rico::RED, 2));

		forces.insert(forces.begin(), bodies.size(), vec());
		pause = false;
		delta_time = 1e-5;
		return true;
	}

	bool OnUserUpdate(double elapsed_ms) override {
		(void) elapsed_ms;
		// handle user input
		if (GetButton('q').pressed) return false;
		if (GetButton('p').pressed) pause = !pause;
		if (GetButton('f').pressed) delta_time *= 2.0;
		if (GetButton('s').pressed) delta_time /= 2.0;
		if (!pause) {
			// fade traces
			DarkenScreen();
			// apply gravity
			UpdatePositions();
		}
		// draw elements
		for (Body const& body : bodies) {
			body.draw();
		}
		return true;
	}

	void OnUserDestroy(void) override {
		return;
	}

}; // class NBodies

int main(int argc, char const **argv) {
	int retval = rico::GameEngine::Construct(1000, 1000, 5);
	if (retval != 0) return EXIT_FAILURE;
	return rico::GameEngine::Run<NBodies>(argc, argv);
}