CXX      := aarch64-linux-gnu-g++-11
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -pthread -DLINUX_PLATFORM
LDFLAGS  := -static-libstdc++ -static-libgcc -pthread
TARGET   := imu_app
SRCS     := $(wildcard *.cpp)
OBJS     := $(SRCS:.cpp=.o)
DEPS     := $(OBJS:.o=.d)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(LDFLAGS)
	@file $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

.PHONY: clean
