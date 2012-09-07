#Created by Aaron Lefohn
#This makefile is designed to be used on the cs Linux workstations
# Modified by John Schreiner

SOURCE= sampleGUI.cpp
EXEC= sample
CC = gcc -ggdb

# The rest should be okay!
GLUT_LIB=/usr/lib
GLUT_INCLUDE=/usr/include
GLUI_LIB=lib
GLUI_INCLUDE=.

FLAGS = -I$(GLUI_INCLUDE) -I$(GLUT_INCLUDE) -L$(GLUT_LIB)  -L$(GLUI_LIB) \
	-lglui -lglut -lXmu  -lXext -lX11 -lGL -lGLU -lm -lGLEW

$(EXEC): $(SOURCE)
	$(CC) -o $(EXEC) $(SOURCE) $(FLAGS)



