#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <cmath>

// Constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 960;
const int FPS = 60;
const int FRAME_DELAY = 1000 / FPS;
const int P = 22;
const int B = 7;
const int LEFT = 140;
const int RIGHT = 1140;
const int TOP = 105;
const int BOTTOM = 855;
const int GTOP = 330;
const int GBOTTOM = 630;
const int GKLEFT = 490;
const int GKRIGHT = 790;

// Global variables
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

int random(int range_from, int range_to) {
    std::random_device                  rand_dev;
    std::mt19937                        generator(rand_dev());
    std::uniform_int_distribution<int>    distr(range_from, range_to);
    return distr(generator);
}

//Texture wrapper class
class TextureWrapper
{
public:
    //Initializes variables
    TextureWrapper() {
        //Initialize
        texture = NULL;
        width = 0;
        height = 0;
        x = 0;
        y = 0;
        vx = 0;
        vy = 0;
    }

    //Loads image at specified path
    bool loadFromFile(std::string path) {
        //Get rid of preexisting texture
        free();

        //The texture
        texture = IMG_LoadTexture(renderer, path.c_str());

        if (texture == NULL)
        {
            printf("Unable to load texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
        }
        else
        {
            //Get image dimensions
            int result = SDL_QueryTexture(texture, NULL, NULL, &width, &height);
            if (result != 0)
            {
				printf("Unable to query texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
			}
        }

        return texture != NULL;
    }

    //Deallocates texture
    void free() {
        //Free texture if it exists
        if (texture != NULL)
        {
            SDL_DestroyTexture(texture);
            texture = NULL;
            width = 0;
            height = 0;
        }
    }

    //Renders texture at given point
    void render() {
        //Set rendering space and render to screen
        SDL_Rect renderQuad = { x - width / 2, y - height / 2, width, height };
        int result = SDL_RenderCopy(renderer, texture, NULL, &renderQuad);
        if (result != 0)
        {
            printf("Unable to render texture! SDL Error: %s\n", SDL_GetError());
        }
    }

    //Renders texture full screen
    void renderFull() {
		SDL_RenderCopy(renderer, texture, NULL, NULL);
	}


    //The texture
    SDL_Texture* texture;

    //Image dimensions
    int width;
    int height;
    int x;
    int y;
    int vx;
    int vy;
};

// Global variables
TextureWrapper bgTexture;
TextureWrapper player1Texture;
TextureWrapper player1GKTexture;
TextureWrapper player2Texture;
TextureWrapper player2GKTexture;
TextureWrapper ballTexture;
int score1 = 0, score2 = 0;
bool win1 = false, win2 = false;
int frame = 0;

bool isColliding(TextureWrapper obj1, TextureWrapper obj2) {
	// Use circle collision detection
	int dx = obj1.x - obj2.x;
	int dy = obj1.y - obj2.y;
	int distance = sqrt(dx * dx + dy * dy);
	return distance < obj1.width / 2 + obj2.width / 2;
}

bool init() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    //Initialize PNG loading
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }

    //Initialize SDL_ttf
    if (TTF_Init() == -1)
    {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        return false;
    }

    // Create window
    window = SDL_CreateWindow("SDL Soccer Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Initialize renderer color
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

    return true;
}

bool loadMedia()
{
    // Loading success flag
    bool success = true;

    // Load background texture
    if (!bgTexture.loadFromFile("assets/img/bg.png"))
    {
        printf("Failed to load background texture image!\n");
        success = false;
    }

    // Load player texture
    if (!player1Texture.loadFromFile("assets/img/player1.png"))
    {
		printf("Failed to load player1 texture image!\n");
		success = false;
	}
    if (!player1GKTexture.loadFromFile("assets/img/player1GK.png"))
    {
		printf("Failed to load player1GK texture image!\n");
		success = false;
	}

    if (!player2Texture.loadFromFile("assets/img/player2.png"))
    {
		printf("Failed to load player2 texture image!\n");
		success = false;
	}

    if (!player2GKTexture.loadFromFile("assets/img/player2GK.png"))
    {
		printf("Failed to load playerGK texture image!\n");
		success = false;
	}

    // Load ball texture
    if (!ballTexture.loadFromFile("assets/img/ball.png"))
    {
        printf("Failed to load ball texture image!\n");
        success = false;
    }

	return success;
}

void close()
{
    //Free loaded images
    bgTexture.free();
    player1Texture.free();
    player1GKTexture.free();
    player2Texture.free();
    player2GKTexture.free();
    ballTexture.free();

    //Destroy window	
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    renderer = NULL;
    window = NULL;

    //Quit SDL subsystems
    IMG_Quit();
    SDL_Quit();
}

int showStartScreen() {
    bool done = false;
    int mode = 1; // 1 for 1P, 2 for 2P
    SDL_Event e;

    TTF_Font* font = TTF_OpenFont("assets/font/font.ttf", 32); // Load your font
    SDL_Color textColor = { 255, 255, 255, 255 }; // White color

    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return -1;
    }

    SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Press 1 for 1P mode, Press 2 for 2P mode", textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    int textWidth = textSurface->w;
    int textHeight = textSurface->h;
    SDL_FreeSurface(textSurface);

    SDL_Rect textRect = { (SCREEN_WIDTH - textWidth) / 2, (SCREEN_HEIGHT - textHeight) / 2, textWidth, textHeight };

    while (!done) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                done = true;
            }
            else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                case SDLK_1:
                    mode = 1;
                    done = true;
                    break;
                case SDLK_2:
                    mode = 2;
                    done = true;
                    break;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Render text
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(textTexture);
    TTF_CloseFont(font);

    return mode;
}

void reset() {
    // Reset player positions
    player1Texture.x = 400;
    player1Texture.y = 480;

    player1GKTexture.x = 200;
    player1GKTexture.y = 480;

    player2Texture.x = 880;
    player2Texture.y = 480;

    player2GKTexture.x = 1080;
    player2GKTexture.y = 480;

    // Reset ball position
    ballTexture.x = 640;
    ballTexture.y = random(410, 550);

    // Reset ball velocity
    ballTexture.vx = 0;
    ballTexture.vy = 0;

    // Render background
    bgTexture.renderFull();

    // Render players and ball
    player1Texture.render();
    player1GKTexture.render();

    player2Texture.render();
    player2GKTexture.render();

    ballTexture.render();

    // Render scores
    TTF_Font* font = TTF_OpenFont("assets/font/font.ttf", 64); // Load your font
    SDL_Color textColor = { 255, 255, 255, 255 }; // White color

    if (!font) {
		std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
		return;
	}

    SDL_Surface* textSurface = TTF_RenderText_Solid(font, std::to_string(score1).c_str(), textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    int textWidth = textSurface->w;
    int textHeight = textSurface->h;
    SDL_FreeSurface(textSurface);

    SDL_Rect textRect = { 500, 30, textWidth, textHeight };
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    textSurface = TTF_RenderText_Solid(font, std::to_string(score2).c_str(), textColor);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    textWidth = textSurface->w;
    textHeight = textSurface->h;
    SDL_FreeSurface(textSurface);

    textRect = { 740, 30, textWidth, textHeight };
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    // Update screen
    SDL_RenderPresent(renderer);
}

void handleInput1P(int selectedPlayer1) {
    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
    // Handle player
    if (selectedPlayer1 == 1) {
        if (currentKeyStates[SDL_SCANCODE_W]) {
            player1Texture.vy = -4;
        }
        if (currentKeyStates[SDL_SCANCODE_A]) {
            player1Texture.vx = -4;
        }
        if (currentKeyStates[SDL_SCANCODE_S]) {
            player1Texture.vy = +4;
        }
        if (currentKeyStates[SDL_SCANCODE_D]) {
            player1Texture.vx = +4;
        }
    }
    else {
        if (currentKeyStates[SDL_SCANCODE_W]) {
            player1GKTexture.vy = -4;
        }
        if (currentKeyStates[SDL_SCANCODE_A]) {
            player1GKTexture.vx = -4;
        }
        if (currentKeyStates[SDL_SCANCODE_S]) {
            player1GKTexture.vy = +4;
        }
        if (currentKeyStates[SDL_SCANCODE_D]) {
            player1GKTexture.vx = +4;
        }
    }

    // Handle AI
    // handle first player
    if (ballTexture.y < player2Texture.y) {
		player2Texture.vy = -4;
	}
    else if (ballTexture.y > player2Texture.y) {
		player2Texture.vy = +4;
	}
    if (ballTexture.x < player2Texture.x) {
        player2Texture.vx = -4;
    }
    else if (ballTexture.x > player2Texture.x) {
		player2Texture.vx = +4;
	}
    
    // handle goal keeper
    if (ballTexture.y < player2GKTexture.y) {
        player2GKTexture.vy = -4;
	}
    else if (ballTexture.y > player2GKTexture.y) {
        player2GKTexture.vy = +4;
    }
}

void handleInput2P(int selectedPlayer1, int selectedPlayer2) {
    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
    // Handle player 1
    if (selectedPlayer1 == 1) {
        if (currentKeyStates[SDL_SCANCODE_W]) {
			player1Texture.vy = -4;
		}
        if (currentKeyStates[SDL_SCANCODE_A]) {
			player1Texture.vx = -4;
		}
        if (currentKeyStates[SDL_SCANCODE_S]) {
			player1Texture.vy = +4;
		}
        if (currentKeyStates[SDL_SCANCODE_D]) {
			player1Texture.vx = +4;
		}
	}
    else {
        if (currentKeyStates[SDL_SCANCODE_W]) {
			player1GKTexture.vy = -4;
		}
        if (currentKeyStates[SDL_SCANCODE_A]) {
			player1GKTexture.vx = -4;
		}
        if (currentKeyStates[SDL_SCANCODE_S]) {
			player1GKTexture.vy = +4;
		}
        if (currentKeyStates[SDL_SCANCODE_D]) {
			player1GKTexture.vx = +4;
		}
	}

    // Handle player 2
    if (selectedPlayer2 == 1) {
        if (currentKeyStates[SDL_SCANCODE_UP]) {
            player2Texture.vy = -4;
        }
        if (currentKeyStates[SDL_SCANCODE_LEFT]) {
            player2Texture.vx = -4;
        }
        if (currentKeyStates[SDL_SCANCODE_DOWN]) {
            player2Texture.vy = +4;
        }
        if (currentKeyStates[SDL_SCANCODE_RIGHT]) {
            player2Texture.vx = +4;
        }
    }
    else {
        if (currentKeyStates[SDL_SCANCODE_UP]) {
            player2GKTexture.vy = -4;
        }
        if (currentKeyStates[SDL_SCANCODE_LEFT]) {
            player2GKTexture.vx = -4;
        }
        if (currentKeyStates[SDL_SCANCODE_DOWN]) {
            player2GKTexture.vy = +4;
        }
        if (currentKeyStates[SDL_SCANCODE_RIGHT]) {
            player2GKTexture.vx = +4;
        }
    }
}

void updateVelocity() {
    // Check ball collision with walls
    if (ballTexture.x - ballTexture.width / 2 < LEFT) {
		ballTexture.vx = -ballTexture.vx;
		ballTexture.x = LEFT + ballTexture.width / 2;
	}
    else if (ballTexture.x + ballTexture.width / 2 > RIGHT) {
		ballTexture.vx = -ballTexture.vx;
		ballTexture.x = RIGHT - ballTexture.width / 2;
	}
    if (ballTexture.y - ballTexture.height / 2 < TOP) {
		ballTexture.vy = -ballTexture.vy;
		ballTexture.y = TOP + ballTexture.height / 2;
	}
    else if (ballTexture.y + ballTexture.height / 2 > BOTTOM) {
		ballTexture.vy = -ballTexture.vy;
		ballTexture.y = BOTTOM - ballTexture.height / 2;
	}

    // Check ball collision with players
    if (isColliding(player1Texture, ballTexture)) {
        ballTexture.vx += (ballTexture.x - player1Texture.x) / B;
        ballTexture.vy += (ballTexture.y - player1Texture.y) / B;
    }
    if (isColliding(player1GKTexture, ballTexture)) {
		ballTexture.vx += (ballTexture.x - player1GKTexture.x) / B;
		ballTexture.vy += (ballTexture.y - player1GKTexture.y) / B;
	}
    if (isColliding(player2Texture, ballTexture)) {
        ballTexture.vx += (ballTexture.x - player2Texture.x) / B;
        ballTexture.vy += (ballTexture.y - player2Texture.y) / B;
    }
    if (isColliding(player2GKTexture, ballTexture)) {
        ballTexture.vx += (ballTexture.x - player2GKTexture.x) / B;
        ballTexture.vy += (ballTexture.y - player2GKTexture.y) / B;
    }
    if (ballTexture.vx > 10) {
        ballTexture.vx = 10;
    }
    else if (ballTexture.vx < -10) {
		ballTexture.vx = -10;
	}
    if (ballTexture.vy > 10) {
		ballTexture.vy = 10;
	}
    else if (ballTexture.vy < -10) {
		ballTexture.vy = -10;
	}

    if (frame % 300 == 0) {
        if (ballTexture.vx >= 0) {
			ballTexture.vx = -10;
		}
        else if (ballTexture.vx < 0) {
			ballTexture.vx = 10;
		}
        if (ballTexture.vy >= 0) {
			ballTexture.vy = -10;
		}
        else if (ballTexture.vy < 0) {
			ballTexture.vy = 10;
		}
	}

	// Check player collision with walls
    if (player1Texture.x - player1Texture.width / 2 < LEFT) {
		player1Texture.vx = 0;
		player1Texture.x = LEFT + player1Texture.width / 2;
	}
    else if (player1Texture.x + player1Texture.width / 2 > RIGHT) {
		player1Texture.vx = 0;
		player1Texture.x = RIGHT - player1Texture.width / 2;
	}
    if (player1Texture.y - player1Texture.height / 2 < TOP) {
		player1Texture.vy = 0;
		player1Texture.y = TOP + player1Texture.height / 2;
	}
    else if (player1Texture.y + player1Texture.height / 2 > BOTTOM) {
		player1Texture.vy = 0;
		player1Texture.y = BOTTOM - player1Texture.height / 2;
	}

    if (player1GKTexture.x - player1GKTexture.width / 2 < LEFT) {
		player1GKTexture.vx = 0;
		player1GKTexture.x = LEFT + player1GKTexture.width / 2;
	}
    else if (player1GKTexture.x + player1GKTexture.width / 2 > GKLEFT) {
		player1GKTexture.vx = 0;
		player1GKTexture.x = GKLEFT - player1GKTexture.width / 2;
	}
    if (player1GKTexture.y - player1GKTexture.height / 2 < TOP) {
		player1GKTexture.vy = 0;
		player1GKTexture.y = TOP + player1GKTexture.height / 2;
	}
    else if (player1GKTexture.y + player1GKTexture.height / 2 > BOTTOM) {
		player1GKTexture.vy = 0;
        player1GKTexture.y = BOTTOM - player1GKTexture.height / 2;
    }

    if (player2Texture.x - player2Texture.width / 2 < LEFT) {
        player2Texture.vx = 0;
        player2Texture.x = LEFT + player2Texture.width / 2;
    }
    else if (player2Texture.x + player2Texture.width / 2 > RIGHT) {
		player2Texture.vx = 0;
		player2Texture.x = RIGHT - player2Texture.width / 2;
	}
    if (player2Texture.y - player2Texture.height / 2 < TOP) {
        player2Texture.vy = 0;
        player2Texture.y = TOP + player2Texture.height / 2;
    }
    else if (player2Texture.y + player2Texture.height / 2 > BOTTOM) {
        player2Texture.vy = 0;
        player2Texture.y = BOTTOM - player2Texture.height / 2;
    }

    if (player2GKTexture.x - player2GKTexture.width / 2 < GKRIGHT) {
		player2GKTexture.vx = 0;
		player2GKTexture.x = GKRIGHT + player2GKTexture.width / 2;
	}
    else if (player2GKTexture.x + player2GKTexture.width / 2 > RIGHT) {
		player2GKTexture.vx = 0;
		player2GKTexture.x = RIGHT - player2GKTexture.width / 2;
	}
    if (player2GKTexture.y - player2GKTexture.height / 2 < TOP) {
		player2GKTexture.vy = 0;
		player2GKTexture.y = TOP + player2GKTexture.height / 2;
	}
    else if (player2GKTexture.y + player2GKTexture.height / 2 > BOTTOM) {
		player2GKTexture.vy = 0;
		player2GKTexture.y = BOTTOM - player2GKTexture.height / 2;
	}
}

void updatePosition() {
	// Update player position
	player1Texture.x += player1Texture.vx;
	player1Texture.y += player1Texture.vy;


	player1GKTexture.x += player1GKTexture.vx;
	player1GKTexture.y += player1GKTexture.vy;

	player2Texture.x += player2Texture.vx;
	player2Texture.y += player2Texture.vy;

	player2GKTexture.x += player2GKTexture.vx;
	player2GKTexture.y += player2GKTexture.vy;

    // Check player collision with each other
    if (isColliding(player1GKTexture, player1Texture)) {
        int dx = player1GKTexture.x - player1Texture.x;
        int dy = player1GKTexture.y - player1Texture.y;
        if (dx > 0) {
            player1GKTexture.x += 2;
            player1Texture.x -= 2;
        }
        else {
            player1GKTexture.x -= 2;
            player1Texture.x += 2;
        }
        if (dy > 0) {
            player1GKTexture.y += 2;
            player1Texture.y -= 2;
        }
        else {
            player1GKTexture.y -= 2;
            player1Texture.y += 2;
        }
    }

    if (isColliding(player2GKTexture, player2Texture)) {
        int dx = player2GKTexture.x - player2Texture.x;
        int dy = player2GKTexture.y - player2Texture.y;
        if (dx > 0) {
            player2GKTexture.x += 2;
            player2Texture.x -= 2;
        }
        else {
            player2GKTexture.x -= 2;
            player2Texture.x += 2;
        }
        if (dy > 0) {
            player2GKTexture.y += 2;
            player2Texture.y -= 2;
        }
        else {
            player2GKTexture.y -= 2;
            player2Texture.y += 2;
        }
    }

    if (isColliding(player1Texture, player2Texture)) {
        int dx = player1Texture.x - player2Texture.x;
        int dy = player1Texture.y - player2Texture.y;
        if (dx > 0) {
            player1Texture.x += 2;
            player2Texture.x -= 2;
        }
        else {
            player1Texture.x -= 2;
            player2Texture.x += 2;
        }
        if (dy > 0) {
            player1Texture.y += 2;
            player2Texture.y -= 2;
        }
        else {
            player1Texture.y -= 2;
            player2Texture.y += 2;
        }
    }

    if (isColliding(player1GKTexture, player2Texture)) {
        int dx = player1GKTexture.x - player2Texture.x;
        int dy = player1GKTexture.y - player2Texture.y;
        if (dx > 0) {
            player1GKTexture.x += 2;
            player2Texture.x -= 2;
        }
        else {
            player1GKTexture.x -= 2;
            player2Texture.x += 2;
        }
        if (dy > 0) {
            player1GKTexture.y += 2;
            player2Texture.y -= 2;
        }
        else {
            player1GKTexture.y -= 2;
            player2Texture.y += 2;
        }
    }

    if (isColliding(player2GKTexture, player1Texture)) {
        int dx = player2GKTexture.x - player1Texture.x;
        int dy = player2GKTexture.y - player1Texture.y;
        if (dx > 0) {
            player2GKTexture.x += 2;
            player1Texture.x -= 2;
        }
        else {
            player2GKTexture.x -= 2;
            player1Texture.x += 2;
        }
        if (dy > 0) {
            player2GKTexture.y += 2;
            player1Texture.y -= 2;
        }
        else {
            player2GKTexture.y -= 2;
            player1Texture.y += 2;
        }
    }

	// Update ball position
	ballTexture.x += ballTexture.vx;
	ballTexture.y += ballTexture.vy;

    // Check goal
    if (ballTexture.x - ballTexture.width / 2 < LEFT) {
        if (ballTexture.y > GTOP && ballTexture.y < GBOTTOM) {
			win2 = true;
		}
	}
    else if (ballTexture.x + ballTexture.width / 2 > RIGHT) {
        if (ballTexture.y > GTOP && ballTexture.y < GBOTTOM) {
            win1 = true;
		}
	}
}

void render() {
    // Clear screen
    SDL_RenderClear(renderer);

    // Render background
    bgTexture.renderFull();

    // Render players and ball
    player1Texture.render();
    player1GKTexture.render();

    player2Texture.render();
    player2GKTexture.render();

    ballTexture.render();

    // Render scores
    TTF_Font* font = TTF_OpenFont("assets/font/font.ttf", 64); // Load your font
    SDL_Color textColor = { 255, 255, 255, 255 }; // White color

    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Surface* textSurface = TTF_RenderText_Solid(font, std::to_string(score1).c_str(), textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    int textWidth = textSurface->w;
    int textHeight = textSurface->h;
    SDL_FreeSurface(textSurface);

    SDL_Rect textRect = { 500, 30, textWidth, textHeight };
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    textSurface = TTF_RenderText_Solid(font, std::to_string(score2).c_str(), textColor);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    textWidth = textSurface->w;
    textHeight = textSurface->h;
    SDL_FreeSurface(textSurface);

    textRect = { 740, 30, textWidth, textHeight };
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    // Update screen
    SDL_RenderPresent(renderer);
}

int main(int argc, char* args[]) {
    //Start up SDL and create window
    if (!init())
    {
        printf("Failed to initialize!\n");
        return -1;
    }

    //Load media
    if (!loadMedia())
    {
        printf("Failed to load media!\n");
        return -1;
    }

    bool quit = false;
    SDL_Event e;

    int mode = showStartScreen();

    reset();

    // Main loop
    if (mode == 1) {
        int selectedPlayer1 = 1;
        // 1P game loop
        while (!quit) {
            Uint32 frameStart = SDL_GetTicks();

            while (SDL_PollEvent(&e) != 0) {
                //User requests quit
                if (e.type == SDL_QUIT) {
                    quit = true;
                }
                //User presses a key
                else if (e.type == SDL_KEYDOWN) {
                    switch (e.key.keysym.sym) {
                    case SDLK_w:
                        if (selectedPlayer1 == 1) {
                            player1Texture.vy = -4;
                        }
                        else {
                            player1GKTexture.vy = -4;
                        }
                        break;
                    case SDLK_a:
                        if (selectedPlayer1 == 1) {
                            player1Texture.vx = -4;
                        }
                        else {
                            player1GKTexture.vx = -4;
                        }
                        break;
                    case SDLK_s:
                        if (selectedPlayer1 == 1) {
                            player1Texture.vy = +4;
                        }
                        else {
                            player1GKTexture.vy = +4;
                        }
                        break;
                    case SDLK_d:
                        if (selectedPlayer1 == 1) {
                            player1Texture.vx = +4;
                        }
                        else {
                            player1GKTexture.vx = +4;
                        }
                        break;
                    case SDLK_LSHIFT:
                        if (selectedPlayer1 == 1) {
                            player1Texture.vx = 0;
                            player1Texture.vy = 0;
                            selectedPlayer1 = 2;
                        }
                        else {
                            player1GKTexture.vx = 0;
                            player1GKTexture.vy = 0;
                            selectedPlayer1 = 1;
                        }
                        break;
                    }
                }
                else if (e.type == SDL_KEYUP) {
                    switch (e.key.keysym.sym) {
                    case SDLK_w:
                        if (selectedPlayer1 == 1) {
                            player1Texture.vy = 0;
                        }
                        else {
                            player1GKTexture.vy = 0;
                        }
                        break;
                    case SDLK_a:
                        if (selectedPlayer1 == 1) {
                            player1Texture.vx = 0;
                        }
                        else {
                            player1GKTexture.vx = 0;
                        }
                        break;
                    case SDLK_s:
                        if (selectedPlayer1 == 1) {
                            player1Texture.vy = 0;
                        }
                        else {
                            player1GKTexture.vy = 0;
                        }
                        break;
                    case SDLK_d:
                        if (selectedPlayer1 == 1) {
                            player1Texture.vx = 0;
                        }
                        else {
                            player1GKTexture.vx = 0;
                        }
                        break;
                    }
                }
            }
            handleInput1P(selectedPlayer1);

            updateVelocity();
            updatePosition();

            render();

            if (win1) {
                score1++;
                win1 = false;
                reset();
                continue;
            }
            else if (win2) {
                score2++;
                win2 = false;
                reset();
                continue;
            }

            frame++;

            int frameTime = SDL_GetTicks() - frameStart;
            if (frameTime < FRAME_DELAY) {
                SDL_Delay(FRAME_DELAY - frameTime);
            }
        }
    }
    else {
        int selectedPlayer1 = 1, selectedPlayer2 = 1;
        // 2P game loop
		while (!quit) {
			Uint32 frameStart = SDL_GetTicks();

			while (SDL_PollEvent(&e) != 0) {
                //User requests quit
				if (e.type == SDL_QUIT) {
					quit = true;
				}
                //User presses a key
                else if (e.type == SDL_KEYDOWN) {
                    switch (e.key.keysym.sym) {
					case SDLK_w:
                        if (selectedPlayer1 == 1) {
							player1Texture.vy = -4;
						}
                        else {
							player1GKTexture.vy = -4;
						}
						break;
					case SDLK_a:
                        if (selectedPlayer1 == 1) {
							player1Texture.vx = -4;
						}
                        else {
							player1GKTexture.vx = -4;
						}
						break;
                    case SDLK_s:
                            if (selectedPlayer1 == 1) {
							player1Texture.vy = +4;
						}
                            else {
							player1GKTexture.vy = +4;
						}
						break;
					case SDLK_d:
                        if (selectedPlayer1 == 1) {
							player1Texture.vx = +4;
						}
                        else {
							player1GKTexture.vx = +4;
						}
                        break;
                    case SDLK_UP:
                        if (selectedPlayer2 == 1) {
                            player2Texture.vy = -4;
                        }
                        else {
							player2GKTexture.vy = -4;
						}
                        break;
					case SDLK_LEFT:
                        if (selectedPlayer2 == 1) {
							player2Texture.vx = -4;
						}
                        else {
							player2GKTexture.vx = -4;
						}
						break;
					case SDLK_DOWN:
                        if (selectedPlayer2 == 1) {
							player2Texture.vy = +4;
						}
                        else {
							player2GKTexture.vy = +4;
						}
						break;
					case SDLK_RIGHT:
                        if (selectedPlayer2 == 1) {
							player2Texture.vx = +4;
						}
                        else {
							player2GKTexture.vx = +4;
						}
						break;
                    case SDLK_LSHIFT:
                        if (selectedPlayer1 == 1) {
                            player1Texture.vx = 0;
                            player1Texture.vy = 0;
							selectedPlayer1 = 2;
						}
                        else {
                            player1GKTexture.vx = 0;
							player1GKTexture.vy = 0;
							selectedPlayer1 = 1;
						}
						break;
                    case SDLK_RSHIFT:
                        if (selectedPlayer2 == 1) {
                            player2Texture.vx = 0;
                            player2Texture.vy = 0;
                            selectedPlayer2 = 2;
                        }
                        else {
                            player2GKTexture.vx = 0;
							player2GKTexture.vy = 0;
							selectedPlayer2 = 1;
						}
                        break;
					}
				}
                else if (e.type == SDL_KEYUP) {
                    switch (e.key.keysym.sym) {
					case SDLK_w:
                        if (selectedPlayer1 == 1) {
							player1Texture.vy = 0;
						}
                        else {
							player1GKTexture.vy = 0;
						}
						break;
					case SDLK_a:
                        if (selectedPlayer1 == 1) {
							player1Texture.vx = 0;
						}
                        else {
							player1GKTexture.vx = 0;
						}
						break;
					case SDLK_s:
                        if (selectedPlayer1 == 1) {
							player1Texture.vy = 0;
						}
                        else {
							player1GKTexture.vy = 0;
						}
						break;
					case SDLK_d:
                        if (selectedPlayer1 == 1) {
							player1Texture.vx = 0;
						}
                        else {
							player1GKTexture.vx = 0;
						}
						break;
					case SDLK_UP:
                        if (selectedPlayer2 == 1) {
							player2Texture.vy = 0;
						}
                        else {
							player2GKTexture.vy = 0;
						}
						break;
					case SDLK_LEFT:
                        if (selectedPlayer2 == 1) {
							player2Texture.vx = 0;
						}
                        else {
							player2GKTexture.vx = 0;
						}
						break;
					case SDLK_DOWN:
                        if (selectedPlayer2 == 1) {
							player2Texture.vy = 0;
						}
                        else {
							player2GKTexture.vy = 0;
						}
						break;
					case SDLK_RIGHT:
                        if (selectedPlayer2 == 1) {
							player2Texture.vx = 0;
						}
                        else {
							player2GKTexture.vx = 0;
						}
						break;
					}
				}
			}
            handleInput2P(selectedPlayer1, selectedPlayer2);

            updateVelocity();
            updatePosition();

			render();

            if (win1) {
				score1++;
                win1 = false;
                reset();
                continue;
			}
            else if (win2) {
				score2++;
                win2 = false;
                reset();
                continue;
			}

            frame++;

			int frameTime = SDL_GetTicks() - frameStart;
			if (frameTime < FRAME_DELAY) {
				SDL_Delay(FRAME_DELAY - frameTime);
			}
		}
	}

    close();

    return 0;
}
