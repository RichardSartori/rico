/** Conway's Game Of Life
 * q = QUIT
 * p = toggle PAUSE
 * s = compute one step (while pausing)
 * left click = toggle cell
 * right click = spawn glider
 */

#include "rico.hpp"
#include "random.hpp"

#define ALIVE rico::BLACK
#define DEAD rico::WHITE

class GameOfLife : public rico::Game {
private:

	using vec = rico::vec2D;
	using mat = rico::mat2D<bool>;
	bool pause, step;
	mat current, next;
	static constexpr uint32_t FPS = 30;

	// return the position in the matrix with wrap around
	vec wrap(vec v) {
		int32_t x = (v.x + Width()) % Width();
		int32_t y = (v.y + Height()) % Height();
		return vec(x, y);
	}

	// create glider centered around position p
	void glider(vec p) {
		vec const offsets[5] = {
			{+0, -1}, {+1, +0}, {-1, +1}, {+0, +1}, {+1, +1}
		};
		for (uint32_t i = 0; i < 5; ++i) {
			current[wrap(p + offsets[i])] = true;
			SetPixel(wrap(p + offsets[i]), ALIVE);
		}
	}

	// compute next state
	void update(void) {
		vec const offsets[8] = {
			{-1, -1}, {+0, -1}, {+1, -1},
			{-1, +0},           {+1, +0},
			{-1, +1}, {+0, +1}, {+1, +1},
		};
		for (uint32_t x = 0; x < Width(); ++x) {
			for (uint32_t y = 0; y < Height(); ++y) {
				vec pos(x , y);
				int sum = 0;
				for (int i = 0; i < 8; ++i) {
					if (current[wrap(pos + offsets[i])]) ++sum;
				}
				bool& src = current[pos];
				bool& dst = next[pos];
				if (sum <= 1) { dst = false; SetPixel(pos, DEAD); }
				if (sum == 2) { dst = src; /* pixel keep color */ }
				if (sum == 3) { dst = true; SetPixel(pos, ALIVE); }
				if (sum >= 4) { dst = false; SetPixel(pos, DEAD); }
			}
		}
	}

protected:

	bool OnUserCreate(int argc, char const **argv) override {
		pause = step = false;
		current = mat(Height(), Width());
		next = mat(Height(), Width());
		double ratio = 0.5;
		if (argc >= 2) {
			ratio = std::strtod(argv[1], NULL);
			if (ratio < 0.0 || ratio > 1.0) return false;
		}
		for (uint32_t y = 0; y < Height(); ++y) {
			for (uint32_t x = 0; x < Width(); ++x) {
				vec pos(x, y);
				bool& cell = current[pos];
				cell = Random::Double() < ratio;
				if (cell) {
					SetPixel(pos, ALIVE);
				} else {
					SetPixel(pos, DEAD);
				}
			}
		}
		return true;
	}

	void OnUserDestroy(void) override {
		return;
	}

	bool OnUserUpdate(uint32_t elapsed_ms) override {
		(void) elapsed_ms;
		if (!pause || step) {
			// update next state based on current state
			update();
			// swap buffers for the next update
			mat tmp(std::move(current));
			current = std::move(next);
			next = std::move(tmp);
			// if step was true, set it false to pause at the next frame
			step = false;
		}
		// handle user inputs
		vec pos;
		if (GetButton('q').pressed) return false;
		if (GetButton('p').pressed) pause = !pause;
		if (GetButton('s').pressed && pause) step = true;
		if (GetButton(rico::Button::LEFT).pressed) {
			if (GetMousePos(&pos)) {
				bool& cell = current[pos];
				cell = !cell;
				if (cell) {
					SetPixel(pos, ALIVE);
				} else {
					SetPixel(pos, DEAD);
				}
			}
		}
		if (GetButton(rico::Button::RIGHT).pressed) {
			if (GetMousePos(&pos)) glider(pos);
		}
		// limit the number of updates per second
		WaitMs(1000 / FPS);
		return true;
	}
};

int main(int argc, char const **argv) {
	int retval = rico::GameEngine::Construct(640, 480, 10);
	if (retval != 0) return EXIT_FAILURE;
	return rico::GameEngine::Run<GameOfLife>(argc, argv);
}