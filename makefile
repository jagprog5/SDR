BUILDDIR = build
SOURCEDIR = src
SOURCES := $(wildcard $(SOURCEDIR)/*.cpp)
OBJECTS := $(patsubst $(SOURCEDIR)/%.cpp,$(BUILDDIR)/%.o,$(SOURCES))

CFLAGS = -std=c++17
LINKFLAGS = # -lm #-lncurses 
EXECUTABLE := $(BUILDDIR)/sdrgaph

.PHONY: all clean run BOOST_CHECK

all: $(EXECUTABLE)

BOOST_CHECK:
	[ -z "$${BOOST_ROOT}" ] && echo "\n====> Set BOOST_ROOT env var\n" && exit 1 || exit 0

$(EXECUTABLE): BOOST_CHECK $(OBJECTS)
	g++ $(OBJECTS) -o $@ $(LINKFLAGS)

$(BUILDDIR)/%.o: $(SOURCEDIR)/%.cpp
	g++ -Iheaders -I"$$BOOST_ROOT" -c $< -o $@ $(CFLAGS)

clean:
	rm -vf $(EXECUTABLE) $(OBJECTS)

run: all
	$(EXECUTABLE)