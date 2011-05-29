
aixi: src/main.o src/agent.o src/search.o src/predict.o src/environment.o src/util.o src/pacman.o src/tictactoe.o src/tiger.o src/kuhnpoker.o src/maze.o src/rock-paper-scissors.o src/extendedtiger.o src/coinflip.o
	g++ -O3 -Wall -o aixi src/*.o

test-predict-build: aixi tests/test-predict.o
	g++ -g -o test-predict src/{util,predict}.o tests/test-predict.o

test-predict: test-predict-build
	./test-predict

test-agent-build: aixi tests/test-agent.o
	g++ -g -o test-agent src/{util,agent,predict}.o tests/test-agent.o

test-agent: test-agent-build
	./test-agent

clean:
	rm -f aixi test-predict src/*.o


