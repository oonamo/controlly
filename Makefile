.PHONY: build run test
CC = gcc

all: test

build:
	${CC} -c ./src/tf.c -o ./build/tf.o
	${CC} -c ./src/arena.c -o ./build/arena.o
	gcc-ar rcs ./build/libcontrols.a ./build/arena.o ./build/tf.o

test: build
	${CC} main.c -o ./build/test -L./build/ -lcontrols
	./build/test
