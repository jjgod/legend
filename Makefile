CFLAGS = -Wall
LDFLAGS = -lSDL -Bstatic -lSDL_image -lSDL_ttf -llua -Bdynamic
OBJECTS = legend.o ui.o scripting.o

legend: $(OBJECTS)

