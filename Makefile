
PROGRAM = aixi
CFLAGS = -O3 -Wall
LDFLAGS =

SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

$(PROGRAM): $(OBJECTS)
	g++ $(CFLAGS) $(LDFLAGS) -o $(PROGRAM) $(OBJECTS)

# Include known dependecies from -MMD
-include $(OBJECTS:.o=.d)

%.o: %.cpp
	g++ -MMD $(CFLAGS) -o $@ -c $<


test-predict-build: aixi tests/test-predict.o
	g++ -g -o test-predict src/{util,predict}.o tests/test-predict.o

test-predict: test-predict-build
	./test-predict

test-agent-build: aixi tests/test-agent.o
	g++ -g -o test-agent src/{util,agent,predict}.o tests/test-agent.o

test-agent: test-agent-build
	./test-agent

clean:
	rm -f $(PROGRAM) test-predict src/*.o src/*.d


