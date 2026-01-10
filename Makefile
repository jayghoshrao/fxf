.PHONY: all clean run build

all:build

build:
	cmake -S . -B build && cmake --build build -j

run: build
	@./build/fxf test.list

clean:
	@rm -rf build
