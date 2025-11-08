# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Iinclude -I.

# Linker flags for OpenGL libraries
LDFLAGS = -lglfw -lGL -ldl -pthread -lm

# --- Target for Part 1 (Bezier Control) ---
TARGET1 = assignment4_part1
SRCS1 = src/main_part1.cpp src/glad.c

# --- Target for Part 2 (Original Shading) ---
TARGET2 = assignment4_part2
SRCS2 = src/main_part2.cpp src/glad.c

# --- Target for Part 3, Program 1 (Image Texture on Bezier) ---
TARGET3 = texture_mapping
SRCS3 = src/texture_mapping.cpp src/glad.c

# --- Target for Part 3, Program 2 (Procedural Texture on SMF) ---
TARGET4 = shading_demo
SRCS4 = src/shading_demo.cpp src/glad.c


# The default rule to build everything
all: $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4)

# Rule for each target
$(TARGET1): $(SRCS1)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(TARGET2): $(SRCS2)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(TARGET3): $(SRCS3)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(TARGET4): $(SRCS4)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# Rule to clean up all build files
clean:
	rm -f $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4)
