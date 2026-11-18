NAME = game

build-system:
	cmake -B build
	
run: build-system
	cmake --build build
	./build/$(NAME)