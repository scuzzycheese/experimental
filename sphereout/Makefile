CC_OSX = g++
LIBS_OSX = -framework Carbon -lm -framework CoreServices -L/usr/X11R6/lib -lX11

CC_WIN = c:\\MinGW\\bin\\g++.exe
LIBS_WIN = -lm -ffast-math -ld3d9 -mconsole -mwindows -ld3d9

CC_LIN = g++
LIBS_LIN = -lm -L/usr/X11R6/lib -lX11 -lrt

NAME = sphereout
OBS = game.o vidBuffer.o cDraw.o cVec2.o cShape.o cMat2.o cCollision.o Pixeltoaster.o
PTC = PT
OPTIONS = -g -I$(PTC) -I.

osx: CC = $(CC_OSX)
osx: LIBS = $(LIBS_OSX)
osx: $(OBS)
	$(CC) $(OPTIONS) -o $(NAME) $(OBS) $(LIBS)

win: CC = $(CC_WIN)
win: LIBS = $(LIBS_WIN)
win: $(OBS)
	$(CC) $(OPTIONS) -o $(NAME) $(OBS) $(LIBS)

lin: CC = $(CC_LIN)
lin: LIBS = $(LIBS_LIN)
lin: $(OBS)
	$(CC) $(OPTIONS) -o $(NAME) $(OBS) $(LIBS)

%.o: %.cpp %.h
	$(CC) $(OPTIONS) -c -o $*.o $*.cpp

Pixeltoaster.o:
	$(CC) $(OPTIONS) -c -o Pixeltoaster.o $(PTC)/PixelToaster.cpp

clean:
	rm -Rf *.o
	rm -Rf $(NAME)
