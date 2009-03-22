CFLAGS = -Wall
LDFLAGS = -lSDL -lSDL_image -lSDL_ttf -llua
OBJECTS = legend.o ui.o scripting.o

legend: $(OBJECTS)

