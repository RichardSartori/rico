#include "rico.hpp"
#include "random.hpp"
#include <iostream>

class Demo : public rico::Game {
protected:

	double ms_count;
	uint32_t frames_count;

	bool OnUserCreate(int argc, char const **argv) override {
		(void) argc;
		(void) argv;
		ms_count = 0.0;
		frames_count = 0;
		Clear(rico::WHITE);
		return true;
	}

	bool OnUserUpdate(double elapsed_ms) override {
		for (uint32_t i = 0; i < Width(); ++i) {
			for (uint32_t j = 0; j < Height(); ++j) {
				rico::Position pos(i, j);
				rico::Color color(Random::rangeUint(0, 255), Random::rangeUint(0, 255), Random::rangeUint(0, 255));
				SetPixel(pos, color);
			}
		}
		++frames_count;
		ms_count += elapsed_ms;
		if (ms_count >= 1000.0) {
			std::cout << "FPS=" << frames_count << std::endl;
			frames_count = 0;
			ms_count -= 1000.0;
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