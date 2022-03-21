TARGET = rss
CC=g++
CPPFLAGS=$(shell pkg-config --cflags libmongocxx curlpp libxml++-2.6) -I. -Wall -Wextra -pedantic
LIBS=$(shell pkg-config --libs libmongocxx curlpp libxml++-2.6) -Wl,-rpath,/usr/local/lib
SRC := $(wildcard *.cpp)
OBJ_DIR  := objects
OBJECTS := $(SRC:%.cpp=$(OBJ_DIR)/%.o)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(CPPFLAGS) $(LIBS)

$(OBJ_DIR)/%.o: %.cpp %.hpp
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -o $@ -c $<

debug: CPPFLAGS += -Og -g
debug: $(TARGET)
release: CPPFLAGS += -O3
release: $(TARGET)
check: CPPFLAGS += -fsyntax-only
check: $(OBJECTS)

clean:
	-@rm -rvf $(OBJ_DIR)
	-@rm -vf $(TARGET)

.PHONY: clean debug release check

