ALLEGRO_VERSION=5.0.10
MINGW_VERSION=4.7.0
FOLDER=C:\Users\maria\Desktop

FOLDER_NAME=\allegro-$(ALLEGRO_VERSION)-mingw-$(MINGW_VERSION)
PATH_ALLEGRO=$(FOLDER)$(FOLDER_NAME)
LIB_ALLEGRO=\lib\liballegro-$(ALLEGRO_VERSION)-monolith-mt.a
INCLUDE_ALLEGRO=\include

all: rtypemod.exe 


rtypemod.exe: rtypemod.o
	gcc -o rtypemod.exe rtypemod.o $(PATH_ALLEGRO)$(LIB_ALLEGRO)

rtypemod.o: rtypemod.c		
	gcc -I $(PATH_ALLEGRO)$(INCLUDE_ALLEGRO) -c rtypemod.c
	
	
clean:
	del rtypemod.o
	del rtypemod.exe

