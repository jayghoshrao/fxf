.PHONY: all clean run build

all:build

build:
	cmake -S . -B build && cmake --build build

run: build
	@./build/fxf pocket.list

clean:
	@rm -rf build
