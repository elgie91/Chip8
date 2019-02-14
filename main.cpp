//Using SDL and standard IO
#include <SDL.h>
#include <stdio.h>  //printf
#include <stdlib.h> //exit()
#include <string.h> //memset


#include "emulator.h"
#include <iostream>

using namespace std;

const int WIDTH = 64;
const int HEIGHT = 32;
int multiplier = 8; //change this to change the multiplier of the dimensions

//The surface contained by the window
SDL_Window *window = NULL;

//The window renderer
SDL_Renderer* renderer = NULL;

//Current displayed texture
SDL_Texture* texture = NULL;


Uint32 *pixelArray  = new Uint32[WIDTH*HEIGHT]; //used to set the pixels read from the emulator



chip8 emulator; //needs to be in the scope of all the functions

void drawScreen()
{
    /*Our screen is defined as unsigned char b/c the smallest possible unit is char,
    so when there is a bit change even if it is not the first bit of the char, it will change the value from 0 to some #
    so you can check if the char value != 1 in order to draw*/

    for (int i = 0; i < WIDTH * HEIGHT; i++)
    {
        Uint8 pixel = emulator.screen[i];
        /*from init(), have line texture = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_STREAMING,WIDTH, HEIGHT);
        this means that the texture created is of the format RGBA8888 or R, G, B, A = all 8 bits. Since the pixel from the screen is going to
        be either 1 or 0, we can multiply the RGB by this value (which are the first 24 bits). We want the pixel to be completely opaque (A(lpha) = 1)
        thus we add (or we can bitwise 'or') FF to the number to get a 32 bit number capable of being stored in the pixelArray*/
        pixelArray[i] = (pixel * 0xFFFFFF00) + 0x000000FF;


    }
}

bool init() //initialize SDL window, texture, and renderer
{
    // initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		return false;
	}

    //Create window
    window = SDL_CreateWindow( "Chip 8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH * multiplier, HEIGHT * multiplier, SDL_WINDOW_SHOWN );
    if( window == NULL )
    {
        printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        return false;
    }

	//create renderer
    renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
        printf( "Couldn't create Render %s \n",SDL_GetError() );
        return false;
    }
    SDL_RenderSetLogicalSize(renderer,WIDTH * multiplier, HEIGHT * multiplier);

    /*//Create surface from the window dimensions (there's another way to create the surface)
    surf = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 32, 0, 0, 0, 0);*/

    //Create Texture
    texture = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_STREAMING,WIDTH, HEIGHT);

    //Attempt to set texture filtering to linear
    if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
        printf( "Warning: Linear texture filtering not enabled!" );

    return true;
}


void closeWindow()
{
    //Destroy the texture
    SDL_DestroyTexture(texture);

    //Destroy the render
    SDL_DestroyRenderer(renderer);

    texture = NULL;
    renderer = NULL;

	return;
}




int main(int argc, char **argv) {

	if (argc < 2)
        exit(-1);

    if (!emulator.loadApplication(argv[1])) //couldn't load the application
    {
        cout << "Couldn't run program, exiting!!" << endl;
        exit(-1);
    }


    if (!init())
    {
        printf("Something Went Wrong :{\n");
    }


    //memset(pixelArray, 0, WIDTH * HEIGHT * sizeof(Uint32)); //write white to the pixels
    SDL_Event e;
    bool leave = false;
//    int flicker = 0;

    while (1)
    {

        while(SDL_PollEvent(&e) != 0 )
        {
            if (e.type == SDL_QUIT)
                leave = true;
        }

        emulator.emulateCycle();
        if (emulator.drawFlag)
        {
            emulator.drawFlag = false;
            drawScreen();
            SDL_UpdateTexture(texture, NULL, pixelArray, WIDTH * sizeof(Uint32)); //last number is the number of pixels per row [width] * size of the color format for each pixel in bytes (RGBA8888 = 32 bits or 4 bytes or int)
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }

        SDL_Delay( 10 );


        if (leave)
            break;
    }

    /*SDL_Event e;
    bool leave = false;
    while (1)
    {

        //SDL_UpdateTexture(texture, NULL, pixelArray, WIDTH * sizeof(Uint32));
        while(SDL_PollEvent(&e) != 0 )
            {
                if (e.type == SDL_QUIT)
                    leave = true;
            }

        if (leave)
            break;

        drawScreen();
    }*/

    //Destroy window & surface
    closeWindow();

	//Quit SDL subsystems
	SDL_Quit();



	return 1;
}


