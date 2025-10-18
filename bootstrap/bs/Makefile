CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I.
SFML_FLAGS = -lsfml-graphics -lsfml-window -lsfml-system

# Targets
TARGET_DEMO = demo_ecs
TARGET_FINAL = final_demo
TARGET_EXAMPLE = example_systems

# Source files
DEMO_SRCS = demo_ecs.cpp
FINAL_SRCS = final_demo.cpp
EXAMPLE_SRCS = example_systems.cpp

# Object files
DEMO_OBJS = $(DEMO_SRCS:.cpp=.o)
FINAL_OBJS = $(FINAL_SRCS:.cpp=.o)
EXAMPLE_OBJS = $(EXAMPLE_SRCS:.cpp=.o)

# Build all targets
all: $(TARGET_DEMO) $(TARGET_FINAL)

# Build demo_ecs
$(TARGET_DEMO): $(DEMO_OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET_DEMO) $(DEMO_OBJS) $(SFML_FLAGS)

# Build final_demo
$(TARGET_FINAL): $(FINAL_OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET_FINAL) $(FINAL_OBJS) $(SFML_FLAGS)

# Build example_systems
$(TARGET_EXAMPLE): $(EXAMPLE_OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET_EXAMPLE) $(EXAMPLE_OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run targets
run: $(TARGET_DEMO)
	./$(TARGET_DEMO)

run-final: $(TARGET_FINAL)
	./$(TARGET_FINAL)

run-example: $(TARGET_EXAMPLE)
	./$(TARGET_EXAMPLE)

clean:
	rm -f $(DEMO_OBJS) $(FINAL_OBJS) $(EXAMPLE_OBJS) $(TARGET_DEMO) $(TARGET_FINAL) $(TARGET_EXAMPLE)

rebuild: clean all

.PHONY: all run run-final run-example clean rebuild
