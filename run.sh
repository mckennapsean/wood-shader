g++ -std=c++0x -lglui -lglut -lXmu  -lXext -lX11 -lGL -lGLU -lm -lGLEW -o $1 $1.c
./$1
rm $1
