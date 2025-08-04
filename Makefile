all: app

app: main.cpp
	@g++ -std=c++23 -g -Wall -Wextra -pedantic \
		-I./include \
		-I./include/external/asio-1.30.0/include \
		-I./include/external/Crow/include \
		main.cpp -o app \
		-lpthread

.PHONY: clean run

clean:
	@rm -rf app

run: app
	@./app
	@rm -rf app
