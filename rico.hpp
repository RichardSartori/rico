/** rico/rico.hpp
 *
 * What is this?
 * ~~~~~~~~~~~~~
 * This is a small attempt to recreate the marvelous OneLoneCoder's
 * PixelGameEngine using the SDL2 library, but in facts it more
 * closely ressemble the ConsoleGameEngine, without the console part
 * RICO stands for Rico Is Clearly OlcConsoleGameEngine
 *
 * Compiling in Linux
 * ~~~~~~~~~~~~~~~~~~
 * You will obviously need to install the SDL2 library
 * solution 1: debian package 'libsdl2-dev'
 * solution 2: clone & install https://github.com/libsdl-org/SDL
 *
 * Then use the command:
 * g++ -std=c++17 -o YourProgName YourSource.cpp -lSDL2
 */

#pragma once

#include <SDL2/SDL.h> // link with -lSDL2
#include <cstdlib>
#include <exception>
#include <stdexcept>
#include <utility>
#include <iostream>
#include <thread>

namespace rico {

	/**
	 * 2-component vector, to hold coordinates
	 * supports addition and comparison
	 */
	struct vec2D {
		int32_t x, y;
		vec2D(int32_t _x, int32_t _y) : x(_x), y(_y) {}
		vec2D(void) : vec2D(0, 0) {}
		vec2D(vec2D const&) = default;
		vec2D& operator=(vec2D const&) = default;
		vec2D& operator+=(vec2D const& rhs) { this->x += rhs.x; this->y += rhs.y; return *this; }
		vec2D operator+(vec2D const& rhs) const { vec2D copy(*this); copy += rhs; return copy; }
		bool operator==(vec2D const& rhs) const { return this->x == rhs.x && this->y == rhs.y; }
	}; // struct vec2D

	/**
	 * template for 2D matrix
	 */
	template<typename T>
	class mat2D {
	private:

		uint32_t rows, cols; // dimensions
		T* data; // underlying data array

		struct Row { // to allow the syntax 'matrix[i][j]'

			T* data;
			uint32_t length;

			Row(T* _data, uint32_t _length) :
				data(_data), length(_length)
			{}

			T& operator[](uint32_t index) {
				if (index >= length) {
					throw std::out_of_range("index out of range");
				}
				return data[index];
			}

		}; // struct mat2D::Row

	public:

		mat2D(void)
			: rows(0), cols(0), data(nullptr)
		{}

		mat2D(uint32_t _rows, uint32_t _cols)
			: rows(_rows), cols(_cols)
		{
			data = static_cast<T*>(std::malloc(rows * cols * sizeof(T)));
			if (data == NULL) throw std::runtime_error("malloc returned NULL");
		}

		~mat2D(void) noexcept {
			std::free(data);
		}

		mat2D(mat2D const& other)
			: mat2D(other.rows, other.cols)
		{
			for (uint32_t i = 0; i < rows * cols; ++i) {
				data[i] = other.data[i];
			}
		}

		mat2D& operator=(mat2D const& other) {
			if (this != &other) {
				this->~mat2D();
				new (this) mat2D(other);
			}
			return *this;
		}

		mat2D(mat2D&& other) noexcept :
			rows(other.rows), cols(other.cols), data(other.data)
		{
			new (&other) mat2D();
		}

		mat2D& operator=(mat2D&& other) noexcept {
			new (this) mat2D(std::move(other));
			return *this;
		}

		T* get_pointer(void) {
			return data;
		}

		Row operator[](uint32_t index) {
			if (index >= rows) throw std::out_of_range("index out of range");
			return Row(data + cols * index, cols);
		}

		T& operator[](vec2D pos) { // to allow the syntax 'matrix[vec2D(i, j)]'
			if (pos.x < 0 || pos.y < 0) throw std::out_of_range("negative index");
			uint32_t x = static_cast<uint32_t>(pos.x);
			uint32_t y = static_cast<uint32_t>(pos.y);
			// in a (x, y) position, x refer to the column and y to the row
			return (*this)[y][x];
		}

	}; // class mat2D

	/**
	 * classic RGB color
	 */
	struct Color {

		uint8_t r, g, b; // color components

		Color(uint8_t _r, uint8_t _g, uint8_t _b)
			: r(_r), g(_g), b(_b)
		{}

		Color(void)
			: Color(0, 0, 0)
		{}

		operator uint32_t(void) const {
			// use the RGBA8888 format
			uint32_t retval = 255;
			retval += (1 << 24) * r;
			retval += (1 << 16) * g;
			retval += (1 <<  8) * b;
			return retval;
		}

	}; // struct Color

	/**
	 * a few colors
	 */
	static const Color
		BLACK   (  0,   0,   0),
		RED     (255,   0,   0),
		YELLOW  (255, 255,   0),
		GREEN   (  0, 255,   0),
		CYAN    (  0, 255, 255),
		BLUE    (  0,   0, 255),
		MAGENTA (255,   0, 255),
		WHITE   (255, 255, 255);

	/**
	 * hold the state of physical button (mouse button / keyboard key / ...)
	 */
	class HardwareButton {
	public:
		bool
			pressed, // true only for the frame it is pressed
			down, // true while the button is down
			released; // true only for the frame it is releaed
	private:
		bool previous; // state of this->down at the previous frame

	public:

		HardwareButton(void) :
			pressed(false),
			down(false),
			released(false),
			previous(false)
		{}

		void update(void) {
			pressed = released = false;
			if (down != previous) {
				if (down) {
					pressed = true;
				} else {
					released = true;
				}
			}
			previous = down;
		}

	}; // class HardwareButton

	/**
	 * tagged enum to ease call syntax for querying button state
	 */
	struct Button {

		enum Tag { MOUSE, KEYBOARD };
		Tag tag;
		union {
			uint8_t index;
			char key;
		};

		static constexpr uint8_t LEFT = 1;
		static constexpr uint8_t RIGHT = 2;
		Button(uint8_t _index) : tag(MOUSE), index(_index) {}
		Button(char _key) : tag(KEYBOARD), key(_key) {}
		Button(void) = delete; // always use ctor that set the tag

		static constexpr uint8_t first_index = 1;
		static constexpr uint8_t last_index = 2;
		static constexpr char first_key = 'a';
		static constexpr char last_key = 'z';

		static constexpr size_t index_count = last_index - first_index + 1;
		static constexpr size_t key_count = last_key - first_key + 1;

		/**
		 * @param[out] output filled with the index of the button, if valid
		 * @return true if and only if the button is valid
		 */
		bool valid(size_t *output) const {
			switch (tag) {
				case MOUSE:
					if (first_index <= index && index <= last_index) {
						if (output != NULL) *output = index - first_index;
						return true;
					}
					break;
				case KEYBOARD:
					if (first_key <= key && key <= last_key) {
						if (output != NULL) *output = key - first_key;
						return true;
					}
					break;
			}
			return false;
		}

	}; // struct Button

	/**
	 * singleton, wrapper around SDL elements
	 */
	class GameEngine {
	private:

		// true if and only if Construct was called successfully
		bool init;
		// window size
		uint32_t window_width, window_height;
		// texture size (before stretching)
		uint32_t texture_width, texture_height;
		// stretching factor
		uint32_t pixel_size;
		// SDL elements
		SDL_Window *window;
		SDL_Renderer *renderer;
		SDL_Texture *texture;
		// raw pixel data for the texture
		mat2D<uint32_t> data;
		// devices state
		HardwareButton mouse_state[Button::index_count];
		HardwareButton keyboard_state[Button::key_count];

		GameEngine(void) :
			init(false)
		{}

		/**
		 * access the single instance
		 * @return reference to the instance
		 */
		static GameEngine& Get(void) {
			static GameEngine instance;
			return instance;
		}

		/**
		 * print exception on standard error
		 * @param e exception to print
		 */
		static void PrintException(std::exception const& e) noexcept {
			std::cerr << "[ERROR] " << e.what();
			char const * sdl_error = SDL_GetError();
			if (*sdl_error != '\0') {
				std::cerr << " : " << sdl_error;
			}
			std::cerr << std::endl;
		}

	public:

		/**
		 * create window of size window_width x window_height (in pixels)
		 * composed of macro pixels of size pixel_size x pixel_size
		 * destroy any previously constructed window
		 * @param window_[width/height] dimensions of the window
		 * @param pixel_size dimension of macro pixels
		 * @return 0 on success, -1 on failure
		 */
		static int Construct(
			uint32_t window_width,
			uint32_t window_height,
			uint32_t pixel_size)
			noexcept
		{
			GameEngine& engine = Get();
			if (engine.init) {
				engine.~GameEngine();
			}

			engine.window = NULL;
			engine.renderer = NULL;
			engine.texture = NULL;
			for (uint32_t i = 0; i < Button::index_count; ++i) {
				engine.mouse_state[i] = HardwareButton();
			}
			for (uint32_t i = 0; i < Button::key_count; ++i) {
				engine.keyboard_state[i] = HardwareButton();
			}

			try {

				if (pixel_size == 0) throw std::runtime_error("invalid pixel_size");
				if (window_width  % pixel_size != 0) throw std::runtime_error("invalid window_width");
				if (window_height % pixel_size != 0) throw std::runtime_error("invalid window_height");

				engine.window_width = window_width;
				engine.window_height = window_height;
				engine.pixel_size = pixel_size;
				engine.texture_width = window_width / pixel_size;
				engine.texture_height = window_height / pixel_size;

				int retval = SDL_Init(SDL_INIT_VIDEO);
				if (retval != 0) throw std::runtime_error("SDL_Init");

				engine.window = SDL_CreateWindow(
					"App", // window name
					SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, // position on window
					engine.window_width, engine.window_height, // window size
					0); // creation flags
				if (engine.window == NULL) throw std::runtime_error("SDL_CreateWindow");

				engine.renderer = SDL_CreateRenderer(
					engine.window, // associated window
					-1, // index of the rendering driver
					SDL_RENDERER_ACCELERATED); // creation flags
				if (engine.renderer == NULL) throw std::runtime_error("SDL_CreateRenderer");

				engine.texture = SDL_CreateTexture(
					engine.renderer, // associated renderer
					SDL_PIXELFORMAT_RGBA8888, // pixel format
					SDL_TEXTUREACCESS_STREAMING, // texture access
					engine.texture_width, engine.texture_height); // texture size
				if (engine.texture == NULL) throw std::runtime_error("SDL_CreateTexture");

				// height is the number of rows and width is the number of cols
				engine.data = mat2D<uint32_t>(engine.texture_height, engine.texture_width);

			} catch (std::exception const& e) {
				PrintException(e);
				return -1;
			}

			engine.init = true;
			return 0;
		}

		~GameEngine(void) noexcept {
			if (init) {
				SDL_DestroyTexture(texture);
				SDL_DestroyRenderer(renderer);
				SDL_DestroyWindow(window);
				SDL_QuitSubSystem(SDL_INIT_EVENTS);
				SDL_Quit();
				init = false;
			}
		}

		/**
		 * @return the width of window if constructed, 0 otherwise
		 */
		static uint32_t GetWidth(void) {
			GameEngine& engine = Get();
			if (engine.init) {
				return engine.texture_width;
			}
			return 0;
		}

		/**
		 * @return the height of window if constructed, 0 otherwise
		 */
		static uint32_t GetHeight(void) {
			GameEngine& engine = Get();
			if (engine.init) {
				return engine.texture_height;
			}
			return 0;
		}

		/**
		 * set the color of a specific pixel if constructed
		 * @param pos position of the pixel
		 * @param color color of the pixel
		 */
		static void SetPixel(vec2D pos, Color color) {
			GameEngine& engine = Get();
			if (engine.init) {
				engine.data[pos] = uint32_t(color);
			}
		}

		/**
		 * query mouse position
		 * @param[out] output if not NULL, position of the pixel under mouse
		 * @return true if the mouse is above the window, false otherwise
		 */
		static bool GetMousePos(vec2D *output) {
			GameEngine& engine = Get();
			if (!engine.init) return false;
			int32_t window_x, window_y;
			SDL_GetWindowPosition(engine.window, &window_x, &window_y);
			int32_t global_x, global_y;
			(void) SDL_GetGlobalMouseState(&global_x, &global_y);
			int32_t x = global_x - window_x, y = global_y - window_y;
			if (output != NULL) {
				output->x = x / engine.pixel_size;
				output->y = y / engine.pixel_size;
			}
			return x >= 0 && static_cast<uint32_t>(x) < engine.window_width
				&& y >= 0 && static_cast<uint32_t>(y) < engine.window_height;
		}

		/**
		 * query button state
		 * @param button button to query
		 * @return state of the button if constructed, HardwareButton() otherwise
		 */
		static HardwareButton GetButton(Button button) {
			GameEngine& engine = Get();
			size_t index;
			if (engine.init && button.valid(&index)) {
				switch (button.tag) {
					case Button::MOUSE: return engine.mouse_state[index];
					case Button::KEYBOARD: return engine.keyboard_state[index];
				}
			}
			return HardwareButton();
		}

		/**
		 * @param ms minimal idle duration (in milliseconds) if constructed
		 */
		static void WaitMs(uint32_t ms) {
			GameEngine& engine = Get();
			if (engine.init) {
				std::chrono::milliseconds duration(ms);
				std::this_thread::sleep_for(duration);
			}
		}

		/**
		 * run main loop of the app
		 * forward declaration for dependancies
		 * @param argc forwarded to OnUserCreate
		 * @param argv forwarded to OnUserCreate
		 * @return EXIT_SUCCESS on success, EXIT_FAILURE otherwise
		 */
		template<class C>
		static int Run(int argc, char const ** argv) noexcept;

	}; // class GameEngine

	/**
	 * abstract class for user to inherit from
	 */
	class Game {
	public:

		/**
		 * called once before game loop
		 * @param argc forwarded from main
		 * @param argv forwarded from main
		 * @return true on success, false otherwise
		 */
		virtual bool OnUserCreate(int argc, char const **argv) = 0;

		/**
		 * called once every frame
		 * @param elapsed_ms duration of the previous frame (in milliseconds)
		 * @return true while game loop should continue, false othewise
		 */
		virtual bool OnUserUpdate(uint32_t elapsed_ms) = 0;

		/**
		 * called once after game loop
		 * will not be called if OnUserCreate returned false
		 */
		virtual void OnUserDestroy(void) = 0;

		virtual ~Game(void) noexcept = default;

	protected:

		// useful shortcuts for the user, see GameEngine for documentation
		uint32_t Width(void) const { return GameEngine::GetWidth(); }
		uint32_t Height(void) const { return GameEngine::GetHeight(); }
		void SetPixel(vec2D pos, Color value) const { GameEngine::SetPixel(pos, value); }
		bool GetMousePos(vec2D *output) const { return GameEngine::GetMousePos(output); }
		HardwareButton GetButton(Button button) const { return GameEngine::GetButton(button); }
		void WaitMs(uint32_t ms) const { GameEngine::WaitMs(ms); }

		/**
		 * clear window with the given color
		 * @param color color used to fill
		 */
		void Clear(Color color) const {
			for (uint32_t col = 0; col < Height(); ++col) {
				for (uint32_t row = 0; row < Width(); ++row) {
					SetPixel(vec2D(row, col), color);
				}
			}
		}

	}; // class Game

	// see GameEngine for documentation
	template<class C>
	int GameEngine::Run(
		int argc,
		char const **argv)
		noexcept
	{
		GameEngine& engine = Get();
		if (!engine.init) return EXIT_FAILURE;

		// purge old events
		SDL_Event event;
		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT) return EXIT_SUCCESS;
		}

		// to test return values
		bool ok;
		int retval;

		// initialization
		uint32_t start_of_last_frame = SDL_GetTicks(), now, elapsed_ms;
		Game *app;
		try {
			app = new C();
			ok = app->OnUserCreate(argc, argv);
		} catch (std::exception const& e) {
			PrintException(e);
			ok = false;
		}
		if (!ok) {
			return EXIT_FAILURE;
		}

		// game loop
		int status = EXIT_SUCCESS;
		bool end = false;
		while (!end) {

			// timing
			now = SDL_GetTicks();
			elapsed_ms = now - start_of_last_frame;
			start_of_last_frame = now;

			// user inputs
			while (SDL_PollEvent(&event) != 0) {
				Button button = '\0';
				size_t index;
				switch (event.type) {
					break; case SDL_QUIT:
						end = true;
					break; case SDL_KEYDOWN:
						button = static_cast<char>(event.key.keysym.sym);
						if (button.valid(&index)) engine.keyboard_state[index].down = true;
					break; case SDL_KEYUP:
						button = static_cast<char>(event.key.keysym.sym);
						if (button.valid(&index)) engine.keyboard_state[index].down = false;
					break; case SDL_MOUSEBUTTONDOWN:
						if (event.button.button == SDL_BUTTON_LEFT)  button = Button::LEFT;
						if (event.button.button == SDL_BUTTON_RIGHT) button = Button::RIGHT;
						if (button.valid(&index)) engine.mouse_state[index].down = true;
					break; case SDL_MOUSEBUTTONUP:
						if (event.button.button == SDL_BUTTON_LEFT)  button = Button::LEFT;
						if (event.button.button == SDL_BUTTON_RIGHT) button = Button::RIGHT;
						if (button.valid(&index)) engine.mouse_state[index].down = false;
				}
			}
			for (size_t i = 0; i < Button::index_count; ++i) {
				engine.mouse_state[i].update();
			}
			for (size_t i = 0; i < Button::key_count; ++i) {
				engine.keyboard_state[i].update();
			}

			// update
			try {
				end |= !app->OnUserUpdate(elapsed_ms);
			} catch (std::exception const& e) {
				PrintException(e);
				status = EXIT_FAILURE;
				end = true;
			}

			// display
			try {
				retval = SDL_UpdateTexture(
					engine.texture, // texture to update
					NULL, // update the entire texture
					engine.data.get_pointer(), // raw pixel data
					engine.texture_width * sizeof(uint32_t)); // bytes per line
				if (retval != 0) throw std::runtime_error("SDL_UpdateTexture");

				retval = SDL_RenderCopy(
					engine.renderer, // rendering context
					engine.texture, // source texture
					NULL, // take the entire texture
					NULL); // display it to the entire context
				if (retval != 0) throw std::runtime_error("SDL_RenderCopy");

				SDL_RenderPresent(engine.renderer);

			} catch (std::exception const& e) {
				PrintException(e);
				status = EXIT_FAILURE;
				end = true;
			}
		}

		// finalization
		app->OnUserDestroy();
		delete app;

		return status;
	}

} // namespace rico