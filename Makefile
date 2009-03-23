CFLAGS = -Wall
LDFLAGS = -lSDL -lSDL_image -lSDL_ttf -llua -framework ApplicationServices
OBJECTS = legend.o ui.o scripting.o

legend: $(OBJECTS)

