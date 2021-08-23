BUILDDIR = build
SOURCEDIR = src
HEADERDIR = headers
SOURCES := $(wildcard $(SOURCEDIR)/*.cpp)
OBJECTS := $(patsubst $(SOURCEDIR)/%.cpp,$(BUILDDIR)/%.o,$(SOURCES))

CFLAGS = #-std=c99 #-Ofast -std=c11
LINKFLAGS = # -lm #-lncurses 
EXECUTABLE := $(BUILDDIR)/sdrgaph

.PHONY: all clean run

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	g++ $^ -o $@ $(LINKFLAGS)

$(BUILDDIR)/%.o: $(SOURCEDIR)/%.cpp
	g++ -I$(HEADERDIR) -c $< -o $@ $(CFLAGS)

clean:
	rm -vf $(EXECUTABLE) $(OBJECTS)

run: all
	$(EXECUTABLE)