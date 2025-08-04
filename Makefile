all: app

app: src/routes/router.cpp src/development/builder.cpp main.cpp
	@g++ -std=c++23 -g -Wall -Wextra -pedantic \
		-I./include \
		-I./include/development \
		-I./include/routes \
		-I./include/external/asio-1.30.0/include \
		-I./include/external/Crow/include \
		src/routes/router.cpp src/development/builder.cpp main.cpp -o app \
		-lpthread

.PHONY: clean run

clean:
	@rm -rf app

run: app
	@./app
	@rm -rf app
