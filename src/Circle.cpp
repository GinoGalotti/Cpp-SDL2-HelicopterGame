/** Author: Surth (Luis Galotti Muñoz)
    I've used LTexture class from Lazy Foo's SDL2 tutorial.
    This is my first C++ game using SDL2, and it's a Helicopter type of game (like Flappy Bird)
    If you want to use it, or you want to help me improving it, feel free to write me to: ginogalotti at google dot com.
*/

#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include <cstdlib>
#include <LTexture.h>
#include <SDL_ttf.h>
#include <cstlib>
#include <cmath>
#include <sstream>

//Screen dimension constants, the frames that we need to do the animation, the time between pipes and the hole space of the pipe in pixels.
const int SCREEN_WIDTH = 1600;
const int SCREEN_HEIGHT = 900;
const int MAX_TIME_PIPE = SCREEN_WIDTH / 21;
const int FREE_SPACE = 180;
const int CHARACTER_X_POS = SCREEN_WIDTH / 4;
const int PIPE_MOVEMENT = 1;
const int CHARACTER_MOVEMENT = 5;
const unsigned int NUM_COLORS = 16;

const unsigned int NUM_CIRCLES = 5 ;
const unsigned int INCREMENTAL_RADIUS = (SCREEN_WIDTH > SCREEN_HEIGHT) ? ((3 * SCREEN_HEIGHT / 4)/NUM_CIRCLES) 
								: ((3 * SCREEN_WIDTH / 4)/NUM_CIRCLES);
const unsigned int INCREMENTAL_PIES = 2;

#define RANDOM_COLOR = rand() % NUM_COLORS

struct custColor {
	Uint8 r;
	Uint8 g;
	Uint8 b;
}

const cusColor colores[NUM_COLORES] = { { 163, 73, 164 }// Lilac
                        , {  237, 28, 36 }		// Red
                        , {  185, 122, 87 }		// LightBrown
                        , {  255, 242, 0 }		// Yellow
                        , {  195, 195, 195 }		// LightGrey
                        , {  181, 230, 29 }		// GreenLime
                        , {  255, 127, 39 }		// Orange
                        , {  63, 60, 224 }		// SeaBlue
                        , {  255, 206, 222 }		// LightPink
                        , {  64, 0, 64 }		// Purple
                        , {  255, 0, 255 }		// Fuscia
                        , {  102, 28, 15 }		// DirtBrown
                        , {  153, 217, 234 }		// LightBlue
                        , {  128, 128, 64 }		// MilitaryGreen                       
                        , {  0, 255, 128 }		// GayLordGreenBlue						
						};
						
strutc custPie{
	double degrees; 	//Starting degree of the pie
	unsigned int color;	//An index for the cusColor array
}
						
struct custCircle {
	unsigned int radius;
	unsigned int numPies;	//Number of slots that the circle will have
	custPie[] pies;		//An array with all the pies inside the circle
}

//Loads individual image as texture
SDL_Texture* loadTexture( std::string path );

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//The font renderer
TTF_Font *gFont = NULL;
SDL_Color textColor;

//This way we manage the starting and restarting text and the point text
LTexture gTextTextureStart;
LTexture gTextTexturePoints;

//This way we are going to save our circles
custCircle* circles;

//This way we are going to save our circles make the feel when a circle is focused
unsigned int circleSelected;
bool highlight;

//Here we are going to handle the selected color of the puzzle
unsigned int selectedColor;

//--------------------
//Starts up SDL and creates window
bool init();

//Loads media regardless of the format (png, jpeg, etc.)
bool loadMedia();

//Frees media and shuts down SDL
void close();

//
void initCircles(custCircle* circles, unsigned int numCircles = 1);

//Restart the game
void restart ();

//Our hand-made collisionDetector
bool collisionWithCharacter(int posXObj, int posYObj, int widthObj, int heightObj);

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			printf( "Warning: Linear texture filtering not enabled!" );
		}

		//Create window
		gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize renderer color to WHITE
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

				   //Initialize SDL_ttf
                if( TTF_Init() == -1 )
                {
                    printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
                    success = false;
                }
			}
		}
	}

	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

     //Open the font
    gFont = TTF_OpenFont( "University.ttf", 20 );
    if( gFont == NULL )
    {
        printf( "Failed to load font! SDL_ttf Error: %s\n", TTF_GetError() );
        success = false;
    }
    else
    {
        //Render text
        textColor = { 0, 0, 0 };
        if( !gTextTextureStart.loadFromRenderedText( "Hi bro! Do you want to try my game? Press Enter to start playing, or Esc to exit", textColor, gFont ,gRenderer ) )
        {
            printf( "Failed to render text texture!\n" );
            success = false;
        }
    }
    
	return success;
}

void close()
{
	gTextTextureStart.free();
	//Destroy window
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}

SDL_Texture* loadTexture( std::string path )
{
	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL )
	{
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
	}
	else
	{
		//Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
		if( newTexture == NULL )
		{
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}

		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	return newTexture;
}


void initCircles(custCircle* circles, unsigned int numCircles = 1){
	for (int i = 0; i < numCircles; i++){
		circles[i].radius = INCREMENTAL_RADIUS * (i+1);
		unsigned int numPiesTemp = 1 + (INCREMENTAL_PIES * i);
		circles[i].numPies = numPiesTemp;
		circles[i].pies = new custPies[numPiesTemp];
		bool haveSelectedColor = false;
		for (int j = 0; j < numPiesTemp; j++){
			circles[i].pie[j].degrees = ((360 / numPiesTemp) * (j+1)) % 360;
			circles[i].pie[j].color = RANDOM_COLOR;
			if (circles[i].pie[j].color == selectedColor) haveSelectedColor = true;
		}
		if (!haveSelectedColor){
			circles[i].pie[rand() % numPiesTemp].color = selectedColor;
		}
	}
}

//This reset the data to restart the game
void restart (){
    //Handler of the points counting
    points = 0;
    std::stringstream ss;
    ss << points;
    if( !gTextTexturePoints.loadFromRenderedText( "Points: " + ss.str(), textColor, gFont ,gRenderer ) )
        {
            printf( "Failed to render text texture of points!\n" );
        }

    //This is going to handle the movement
    frame = 0;
	
    //
    selectedColor = RANDOM_COLOR;
    circles = new custonCircle[NUM_CIRCLES];
    initCircles(circles, NUM_CIRCLES);
    }

//TODO ARREGLAR LA PUTA COLLISION
bool collisionWithCharacter(int posXObj, int posYObj, int widthObj, int heightObj){
    bool collision = false;

    if (posXObj <= CHARACTER_X_POS+60 and posXObj + widthObj >= CHARACTER_X_POS){
        if (posYObj <= posY + 60 and posYObj + heightObj >= posY ) collision = true;
    }

    return collision;
}

int main( int argc, char* args[] )
{
	//Start up SDL and create window
	if( !init() )
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{
		//Load media
		if( !loadMedia() )
		{
			printf( "Failed to load media!\n" );
		}
		else
		{
			//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;

			//Handler about the state
			bool pause = true;

            restart();
            SDL_SetRenderDrawColor(gRenderer,0xFF,0xAE,0xC9,0xFF);
            SDL_RenderClear( gRenderer );
            gTextTextureStart.render(gRenderer,SCREEN_WIDTH/2 - gTextTextureStart.getWidth() / 2,SCREEN_HEIGHT/2 - gTextTextureStart.getHeight() / 2);
            SDL_RenderPresent( gRenderer );

			//Main loop
			while( !quit )
			{
				//Handle events on queue
				while( SDL_PollEvent( &e ) != 0 )
				{
					//User requests quit
					if( e.type == SDL_QUIT )
					{
						quit = true;
					}
					 else if( e.type == SDL_KEYDOWN )
                    {
                        switch( e.key.keysym.sym )
                        {
                            case SDLK_ESCAPE:
                            quit = true;
                            break;
                            
                            //Our egg is going to fly!!
                            case SDLK_UP:
                            flying = MAXIMUN_FRAMES * 4 * 2;
                            degrees = -45;
                            break;
                            
                            //Our game is going to start/restart
                            case SDLK_RETURN:
                            if (pause){
                                pause = !pause;
                                restart();
                            }
                        }
                    }
				}

				if (!pause){
                    SDL_SetRenderDrawColor(gRenderer,0xFF,0xAE,0xC9,0xFF);

                    //Clear screen
                    SDL_RenderClear( gRenderer );

                    //This is how we spawn new pipes. We reset the timer, put the pipe at the end of the Screen, select the random free spot and set the next Pipe that we are going to paint
                    //The max pipes that we are going to allow is 7; so we don't use too much memory
                    if (timingPipe == 0){
                        timingPipe = MAX_TIME_PIPE;
                        pipes[nextPipe].xPosition = SCREEN_WIDTH;
                        pipes[nextPipe].freeSpotPosition = (rand() % 4) + 1;
                        pipes[nextPipe].pointCounted = false;
                        nextPipe = (nextPipe  + 1)% 7;
                        if (totalPipe < 7) totalPipe++;
                        }

                    --timingPipe;

                    //Firstly we draw the sun
                    gSunTexture.render(gRenderer,0,0);

                    //Now we draw all the pipes.
                    for (int i = 0; i < totalPipe; i++){
                        int yPos = 0;
                        int free = pipes[i].freeSpotPosition;
                        //Here we are going to count the points. When the pipe past the character position, we flag it as counted and increment the points
                        if (pipes[i].xPosition < (CHARACTER_X_POS) and !pipes[i].pointCounted){
                            points++;
                            pipes[i].pointCounted = true;
                            std::stringstream ss;
                            ss << points;
                            if( !gTextTexturePoints.loadFromRenderedText( "Points: " + ss.str(), textColor, gFont ,gRenderer ) )
                                {
                                    printf( "Failed to render text texture of points!\n" );
                                }
                        }
                        //And here we draw all the textures in the pipe
                        for (int j = 0; j < 10; j++){
                                    if (j == free) yPos += FREE_SPACE;
                                    else {
                                        pipes[i].xPosition = pipes[i].xPosition - PIPE_MOVEMENT;
                                        if (collisionWithCharacter(pipes[i].xPosition, yPos , 75, 100)) pause = true;
                                        gPipeTexture.render(gRenderer,pipes[i].xPosition,yPos);
                                        yPos += gPipeTexture.getHeight();
                                        }}}

                    gTextTexturePoints.render(gRenderer,SCREEN_WIDTH - (gTextTexturePoints.getWidth() + 20), 10);
                    //Now we render the floor on top of the pipe
                    gFloorTexture.render(gRenderer,0,SCREEN_HEIGHT-gFloorTexture.getHeight());

                    //We select the frame of the egg that we're going to paint
                    SDL_Rect* currentClip = &gSpriteClips[frame / 4];
                    gSpritedMonigote.render(gRenderer,CHARACTER_X_POS, posY , currentClip, degrees,NULL);

                    //This case is when our egg if falling. We accelerate until reaching the maximum speed (15px), we rotate the animation and we set the position
                    if (flying < 0 ){
                        if (flying > -15)
                            flying = flying * 1.1112;
                        if (degrees <= 80 ) degrees += 3;
                        if (posY <= SCREEN_HEIGHT - gFloorTexture.getHeight() - currentClip->h + flying )
                                posY -= flying;
                        else {
                                posY = SCREEN_HEIGHT - gFloorTexture.getHeight() - currentClip->h;
                                //YOU'VE LOST, BABY!
                                pause = true;
                        }}

                    //This case is when we stop flying and start falling
                    if (flying == 0){
                        flying = -1;
                        degrees = 0;}

                    //This case is when we are flying. We rotate the animation, set the frame and fly a little bit
                    if (flying >0){
                        degrees = -45;
                        --flying;
                        ++frame;
                        if (posY > CHARACTER_MOVEMENT) posY -= CHARACTER_MOVEMENT;
                        if (frame/4 >= MAXIMUN_FRAMES){
                            frame = 0;}}
                    else
                        {frame = 0;
                        }

                    if (pause){
                            std::stringstream ss;
                            ss << points;
                            if( !gTextTexturePoints.loadFromRenderedText( "Congratulations... or maybe not. You've reach " + ss.str() + " points. Press Enter to restart, and Esc to exit", textColor, gFont ,gRenderer ) )
                                {
                                    printf( "Failed to render text texture of restart!\n" );
                                } else{
                                    gFrameTexture.setAlpha(0xA0);
                                    gFrameTexture.render(gRenderer, SCREEN_WIDTH/2 - gFrameTexture.getWidth() /2, SCREEN_HEIGHT/2 - gFrameTexture.getHeight() / 2);
                                    gTextTexturePoints.render(gRenderer,SCREEN_WIDTH/2 - gTextTexturePoints.getWidth() /2, SCREEN_HEIGHT/2);}
                    }
                    //Update screen
                    SDL_RenderPresent( gRenderer );			}}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}
