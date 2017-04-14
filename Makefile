CC := g++
SRCDIR := src
INC := -I include
CFLAGS := -g -O2 -Wall -Wextra
BUILDDIR := build
TARGETDIR := bin
LIBS := -L. -lmp3lame
TARGET := a
SRCEXT := cpp
SOURCES=$(wildcard $(SRCDIR)/*.$(SRCEXT))
OBJECTS=$(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(SOURCES))

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LIBS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $< $(LIBS)

clean:
	-rm -rf /s $(TARGETDIR) $(BUILDDIR)
	-mkdir -p bin build

.PHONY: clean