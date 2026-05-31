TARGET = beautiful_prompt

UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

CXX = c++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra

ifeq ($(UNAME_M),x86_64)
    CXXFLAGS += -march=x86-64-v2
endif

ifeq ($(UNAME_S),Linux)
    CXXFLAGS += -static -static-libstdc++ -static-libgcc
else ifeq ($(UNAME_S),FreeBSD)
    CXXFLAGS += -static -static-libstdc++ -static-libgcc
endif

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
