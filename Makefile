CPPFLAGS = -Wall -Wextra -Werror -fmax-errors=1

default:
	@echo "usage: make [demo|life|gravity]"

%: %.cpp rico.hpp random.hpp
	g++ $(CPPFLAGS) -o $@ $< -lSDL2

clean:
	find ./* -executable -exec rm {} \;