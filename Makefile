CFLAGS = -std=c++17 -O2 -Wall -Wextra
GLEW_LIBS = $(shell pkg-config glew --libs)
GLFW_LIBS = $(shell pkg-config glfw3 --libs)

TARGET = cg
OBJS = main.o tinyobj/tiny_obj_loader.o

$(TARGET): $(OBJS)
	g++ $(CFLAGS) -o $@ $^ $(GLEW_LIBS) $(GLFW_LIBS)

%.o: %.cpp
	g++ $(CFLAGS) -c $<

tinyobj/tiny_obj_loader.o: tinyobj/tiny_obj_loader.cc
	g++ $(CFLAGS) -c $< -o $@

.PHONY: test clean

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)
