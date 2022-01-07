#include "rico.hpp"
#include "random.hpp"
#include <iostream>

class Demo : public rico::Game {
protected:

	uint32_t ms_count, frames;

	bool OnUserCreate(int argc, char const **argv) override {
		(void) argc;
		(void) argv;
		ms_count = frames = 0;
		Clear(rico::WHITE);
		return true;
	}

	bool OnUserUpdate(uint32_t elapsed_ms) override {
		for (uint32_t i = 0; i < Width(); ++i) {
			for (uint32_t j = 0; j < Height(); ++j) {
				rico::vec2D pos(i, j);
				// do not use the range [0, 255] with this RNG (see random.hpp)
				rico::Color color(Random::rand_r(1, 254), Random::rand_r(1, 254), Random::rand_r(1, 254));
				SetPixel(pos, color);
			}
		}
		++frames;
		ms_count += elapsed_ms;
		if (ms_count >= 1000) {
			std::cout << "FPS=" << frames << std::endl;
			frames = 0;
			ms_count -= 1000;
		}
		if (GetButton('q').pressed) return false;
		return true;
	}

	void OnUserDestroy(void) override {
		return;
	}
};

int main(int argc, char const **argv) {
	int retval = rico::GameEngine::Construct(640, 480, 10);
	if (retval != 0) return EXIT_FAILURE;
	return rico::GameEngine::Run<Demo>(argc, argv);
}