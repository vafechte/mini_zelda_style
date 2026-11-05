NAME = game

install:
	cmake -B build
	
run:
	cmake --build build
	./build/$(NAME)