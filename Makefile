CFLAGS= `cat lib/Cflags`
LIB_DIR = ./lib
SOURCES=Stack.cpp Stack_Private.cpp
OBJECTS=$(SOURCES:.cpp=.o)
SANITIZE = `cat lib/SanitizeFlags`

all: $(SOURCES) main
	
main: $(OBJECTS) 
	$(cat OBJECTS)
	g++ main.cpp $(addprefix build/, $(OBJECTS)) -L$(LIB_DIR) -lLogger -o build/$@ $(SANITIZE)

.cpp.o:
	g++ -c $(CFLAGS) $< -o build/$@

clean:
	rm build/*

lib: $(OBJECTS) 
	ar rvs lib/libStack.a  $(addprefix build/, $(OBJECTS))
	cp Stack.h lib/Stack.h
	cp config.h lib/config.h
