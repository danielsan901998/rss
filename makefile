CC=g++
CFLAGS=$(shell pkg-config --cflags --libs libmongocxx curlpp libxml++-3.0) -Wl,-rpath,/usr/local/lib -g
OBJ = rss.o request.o parse.o
PCH_SRC = rss.hpp
PCH_OUT = rss.hpp.gch
TARGET = rss
$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
$(PCH_OUT): $(PCH_SRC)
	$(CC)  -c $< $(CFLAGS) -shared
%.o: %.cpp $(PCH_OUT)
	$(CC) -c -o $@ $< $(CFLAGS) -include $(PCH_SRC)
clean:
	rm -f $(OBJ) $(TARGET) $(PCH_OUT) *~
