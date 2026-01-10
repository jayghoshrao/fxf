.PHONY: all clean run build

all:build

build:
	cmake -S . -B build && cmake --build build -j

run: build
	@./build/fxf test.list

clean:
	@rm -rf build

read: build
	@./build/fxf test.list

pipe: build
	@cat test.list | ./build/fxf | vipe

ls: build
	@ls . --color=never | ./build/fxf

vimgrep: build
	@rg --vimgrep label | ./build/fxf
