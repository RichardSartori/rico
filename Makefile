CPPFLAGS = -Wall -Wextra -Werror -fmax-errors=1

default:
	@echo "usage: make [demo|life|gravity]"

%: examples/%.cpp $(wildcard src/*.hpp)
	g++ $(CPPFLAGS) -I src -o $@ $< -lSDL2

clean:
	find ./* -executable -type f -exec rm {} \;