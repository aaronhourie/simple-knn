CC = g++
OBJS = $(build/dataset.o)

all: dataset.o
	$(CC) src/knn.cpp -o knn $(OBJS)

dataset.o: src/dataset.cpp
	$(CC) -c src/dataset.cpp -o build/dataset.o

test-all: memcheck

memcheck:
	valgrind --leak-check=full --show-leak-kinds=all ./knn -t data/train -s data/test

clean:
	-rm build/dataset.o
	-rm knn
