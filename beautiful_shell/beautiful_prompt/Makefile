TARGET = beautiful_prompt

# Компилятор и флаги
CXX = g++
CXXFLAGS = -std=c++17 -O2 -march=x86-64-v2 -static -static-libstdc++ -static-libgcc -Wall -Wextra

PREFIX = ~/.local
BINDIR = $(PREFIX)/bin

SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

install: $(TARGET)
	mkdir -p $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)
	@echo "Successfully installed to $(DESTDIR)$(BINDIR)/$(TARGET)"

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	@echo "Successfully removed."