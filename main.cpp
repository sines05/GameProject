#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <vector>
#include <random>
#include <string>
#include <cmath>
#include <ctime>


using namespace std;


class Aeroplane
{
private:
    int x;
    int y;
public:
    Aeroplane(SDL_Renderer* renderer) {

        x = 800;
        y = 0;
	}
    void draw(SDL_Renderer* renderer, SDL_Texture* texture) {
		SDL_Rect rect = { x, y, 80, 80 };
		SDL_RenderCopy(renderer, texture, NULL, &rect);
	}

    void move(int speed) {
        x -= speed;
    }
    int getX() {
		return x;
	}
    int getY() {
		return y;
	}

    void reset() {
        x = 800;
        y = 0;
    }
};

class Character
{
private:
    int x, y;
    float velY;
    float VelX;
    bool onPlane;
    Aeroplane& plane;


    SDL_Texture* parachuteTexture;

    SDL_Texture* leftFlag;
    SDL_Texture* rightFlag;


    public:
        SDL_Texture* landedTexture;
        float windVelX;
        bool hasScored;
        bool parachuteDeployed;
        enum CharacterState { Hidden, Jumping, Landed, Parachute };
        CharacterState state;
        Character(SDL_Renderer* renderer, int startX, int startY, Aeroplane& aero, SDL_Texture* parachuteTex, SDL_Texture* landedTex, SDL_Texture* leftFlag, SDL_Texture* rightFlag ) :
        state(Hidden), plane(aero), x(startX), y(startY), velY(0), VelX(0), onPlane(true),
        parachuteTexture(parachuteTex), landedTexture(landedTex) {
            parachuteDeployed = false;
            hasScored = false;
        }





        void draw(SDL_Renderer* renderer, SDL_Texture* texture) {
            if (state == Landed && parachuteDeployed) {
                SDL_Rect rect = { x, y, 100, 100 };
                SDL_RenderCopy(renderer, landedTexture, NULL, &rect);
            }
            else {
                if (state == Landed && !parachuteDeployed) {
                    SDL_SetTextureColorMod(texture, 0, 0, 0);

                }
                else if (state == Parachute) {
                    SDL_Rect parachuteRect = { x-15, y - 50, 110, 110 };
                    SDL_RenderCopy(renderer, parachuteTexture, NULL, &parachuteRect);
                    SDL_SetTextureColorMod(texture, 255, 255, 255); // Reset to original color
                }
                else {
                    SDL_SetTextureColorMod(texture, 255, 255, 255); // Reset to original color
                }

                SDL_Rect rect = { x, y - 0, 70, 70 };
                SDL_RenderCopy(renderer, texture, NULL, &rect);
            }
        }

        void land() {
            if (this->state == Parachute || this->state == Jumping) { // Adjusted to check for parachute state
                this->state = Landed;
                this->velY = 0;
                this->VelX = 0;
                hasScored = true;
            }
        }



        void update(float timeStep) {
            if (onPlane) {
                x = plane.getX() + 50;
            }
            else if (state == Parachute) {
                velY += (0 * timeStep);


                static int counter = 0;
                if (counter++ % 100 == 0) {


                }

                x += windVelX * timeStep;
                y += velY * timeStep;

                if (y >= 500) {
                    y = 450;
                    velY = 0;
                    state = Landed;
                }

            }
            else {
                if (state != Landed) {
                    x += VelX * timeStep;
                    y += velY * timeStep;
                    velY += 0.08 * timeStep;
                    if (y >= 500) {
                        y = 500;
                        velY = 0;
                        VelX = 0;
                        state = Landed;
                    }
                }
            }
        }

    void jump() {
        if (onPlane) {
            onPlane = false;
            velY = -2;
            VelX = -2;
            state = Jumping;
            hasScored = false;


        }
    }

        CharacterState getState() const{
			return state;
		}

        void handleEvent(const SDL_Event& event, Mix_Chunk* jumpSound) {
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_j) {
                    jump();
                    Mix_PlayChannel(-1, jumpSound, 0);

                }
                else if (event.key.keysym.sym == SDLK_p && state == Jumping) {
                    parachuteDeployed = true;
                    state = Parachute;

                }
            }
        }


        void reset() {
			x = plane.getX() + 50;
			y = plane.getY();
			velY = 0;
			VelX = 0;
			onPlane = true;
			parachuteDeployed = false;
			state = Hidden;
		}

        int getX() {
			return x;
		}
        int getY() {
			return y;
		}

};

class LandingSpot
{
private:
    int x, y;
public:
    SDL_Texture *texture;
    LandingSpot(SDL_Renderer *renderer, SDL_Texture *texture) : texture(texture) {reset();}
    LandingSpot()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(50, 700);

        x = dist(gen);
        y = 420;
    }

    void draw(SDL_Renderer *renderer, SDL_Texture *texture)
    {
        SDL_Rect rect = {x, y, 100, 100};
        SDL_RenderCopy(renderer, texture, NULL, &rect);
    }

    int getX() {return x;}
    int getY() {return y;}

    void reset()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(50, 700);

        x = dist(gen);
        y = 420;
    }

    void draw(SDL_Renderer* renderer) {
            SDL_Rect rect = { x, y, 100, 150 };
            SDL_RenderCopy(renderer, this->texture, NULL, &rect);
        }
};


class GameManager {
public:
    int currentLevel;
    int totallevels;
    bool levelcompleted;
    bool waitingForNextLevel;
    float delayTimer;
    const float delayDuration = 3.0f;
    float windSpeed;
    int totalScore;


    vector<LandingSpot> landingSpots;
    int activeLandingSpotIndex;

    GameManager(const vector<SDL_Texture*>& landingTextures, SDL_Renderer* renderer) : currentLevel(1), totallevels(10), levelcompleted(false), delayTimer(0.0f), activeLandingSpotIndex(-1) {
        for (auto& texture : landingTextures) {
            landingSpots.emplace_back(renderer, texture);
        }
        activeLandingSpotIndex = -1;
        selectRandomLandingSpot();
    }
    void setWindSpeed(int speed) {
        windSpeed = speed;
    }

    void resetLevel(Aeroplane& plane, Character& character) {
        plane.reset();
        character.reset();

        levelcompleted = false;
        waitingForNextLevel = false;
        delayTimer = 0.0f;
        character.windVelX = (rand()% 5) - 2;
    }

    void resetLevel() {
        levelcompleted = false;
        selectRandomLandingSpot();
        for (auto& spot : landingSpots) {
            spot.reset();
        }
    }
    void nextLevel(Aeroplane& plane, Character& character) {
        if (currentLevel < totallevels) {
            currentLevel++;
            resetLevel(plane, character);
            selectRandomLandingSpot();
        }
        else {



        }
    }

    void selectRandomLandingSpot() {
        activeLandingSpotIndex = (activeLandingSpotIndex + 1) % landingSpots.size();
	}

    void update(Aeroplane& areo, Character& character, float deltaTime) {
        setWindSpeed(character.windVelX);

        if (levelcompleted && !waitingForNextLevel) {
            waitingForNextLevel = true;
            delayTimer = delayDuration; // Start the delay timer
        }


        if (waitingForNextLevel) {
            delayTimer -= deltaTime;
            if (delayTimer <= 0) {
                waitingForNextLevel = false;
                if (currentLevel < totallevels) {
                    nextLevel(areo, character); // Proceed to next level
                }
                else {

                }
            }
        }
    }

    void completeLevel() {
        levelcompleted = true;
    }

    int getWindSpeed() const {
        return abs(windSpeed);
    }
};

class Text
{
public:
    SDL_Renderer* renderer;
    TTF_Font* font;
    SDL_Color color;
    SDL_Rect rect;
    SDL_Texture* texture;
    int score = 0;

    Text(SDL_Renderer* renderer, const string& fontPath, int fontSize, const SDL_Color& color, int x, int y) :
        renderer(renderer), color(color), texture(nullptr)
    {
        font = TTF_OpenFont(fontPath.c_str(), fontSize);
        rect.x = x;
        rect.y = y;
    }

void write(const std::string &text, int x, int y)
{
    if (texture != nullptr) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }

    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (surface == nullptr) {
        cout << "Error rendering text: " << TTF_GetError() << endl;
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    rect.w = surface->w;
    rect.h = surface->h;
    rect.x = x;
    rect.y = y;
    SDL_FreeSurface(surface);
}


    int getScore() {
		return score;
	}

    int updateScore(int d) {
        score+= d;
        return score;
    }

    ~Text() {
        TTF_CloseFont(font);
        SDL_DestroyTexture(texture); // Add this line
    }
};

SDL_Texture* loadTexture(const string &path, SDL_Renderer* renderer)
{
    SDL_Texture* newTexture = IMG_LoadTexture(renderer, path.c_str());
    if (!newTexture)  {
        cerr << "Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << endl;
    }
    return newTexture;
}



void showIntroScreen(SDL_Renderer* renderer) {
    TTF_Font* font = TTF_OpenFont("ThaleahFat.TTF", 24);
    if (font == nullptr) {
        cerr << "Failed to load font: " << TTF_GetError() << endl;
        return;
    }

    vector<string> lines = {
        "The Duck is counting on you to help him score a perfect landing",
        "Press 'J' to launch Duck from the plane",
        "Press 'P' to open his parachute",
        "Watch the flag to check the wind direction. Good Luck!",
        "Press Enter to play the game"
    };

    SDL_Color textColor = { 255, 255, 255 }; // Màu văn bản (trắng)

    // Tạo một bề mặt văn bản cho mỗi dòng văn bản
    vector<SDL_Texture*> textTextures;
    int totalHeight = 0;
    for (const string& line : lines) {
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, line.c_str(), textColor);
        if (textSurface == nullptr) {
            cerr << "Failed to create text surface: " << TTF_GetError() << endl;
            continue;
        }
        textTextures.push_back(SDL_CreateTextureFromSurface(renderer, textSurface));
        totalHeight += textSurface->h;
        SDL_FreeSurface(textSurface);
    }

    int startY = (640 - totalHeight) / 2; // Tính toán vị trí bắt đầu để căn giữa theo trục y

    SDL_RenderClear(renderer);

    // Vẽ từng dòng văn bản lên renderer
    int y = startY;
    for (SDL_Texture* texture : textTextures) {
        int texW, texH;
        SDL_QueryTexture(texture, nullptr, nullptr, &texW, &texH);
        SDL_Rect textRect = { (850 - texW) / 2, y, texW, texH };
        SDL_RenderCopy(renderer, texture, nullptr, &textRect);
        y += texH;
        SDL_DestroyTexture(texture);
    }

    SDL_RenderPresent(renderer);

    // Chờ nhấn phím Enter để bắt đầu chơi
    bool enterPressed = false;
    SDL_Event event;
    while (!enterPressed) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                exit(0);
            }
            else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) {
                enterPressed = true;
            }
        }
        SDL_Delay(10);
    }

    TTF_CloseFont(font);
}




// Main function
int main(int argc, char** argv)
{
    const int SCREEN_WIDTH = 850;
    const int SCREEN_HEIGHT = 640;



    // Create the window and the renderer
    SDL_Window* window = SDL_CreateWindow("Daffy Jump", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 850, 640, SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);



    if (TTF_Init() == -1) {
            cerr << "SDL_ttf could not initialize! SDL_ttf Error: " <<
                             TTF_GetError();
        }
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
        return 1;
    }


    showIntroScreen(renderer);

    // Load the textures
    SDL_Texture* backgroundTexture = IMG_LoadTexture(renderer, "bg.jpg");
    SDL_Texture* Aero = IMG_LoadTexture(renderer, "aeroplane.png");
    SDL_Texture* characterTexture = IMG_LoadTexture(renderer, "character.png");
    SDL_Texture* landingTexture = IMG_LoadTexture(renderer, "landing.png");
    SDL_Texture* parachuteTexture = IMG_LoadTexture(renderer, "parachute.png");
    SDL_Texture* landedParachuteTexture = IMG_LoadTexture(renderer, "landed.png");
    SDL_Texture* leftFlag = IMG_LoadTexture(renderer, "left.png");
    SDL_Texture* rightFlag = IMG_LoadTexture(renderer, "right.png");

    vector <string> imgPaths = {
        "land1.png",
        "land1.png",
        "land3.png",
        "landing.png",
    };

    vector<SDL_Texture*> textures;

    for (const auto& path : imgPaths) {
		textures.push_back(loadTexture(path, renderer));
	}




    Aeroplane plane(renderer);
    Character character(renderer, plane.getX(), plane.getY(), plane, parachuteTexture, landedParachuteTexture, leftFlag, rightFlag);
    GameManager gameManager(textures, renderer);


    // Initialize the SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		cout << "Error: " << Mix_GetError() << endl;
	}

	// Load the background music
    Mix_Music* bgm = Mix_LoadMUS("bg.mp3");
    if (bgm == nullptr) {
        cerr<< "Error loading music: " << Mix_GetError() << endl;
    }

    // Load the sound effects
    Mix_Chunk* jumpSound = Mix_LoadWAV("jump.wav");
    if (jumpSound == nullptr) {
		cerr << "Error loading sound: " << Mix_GetError() << endl;
	}
    else {
        Mix_VolumeChunk(jumpSound, MIX_MAX_VOLUME/3);
    }

    Mix_Chunk* areoSound = Mix_LoadWAV("aeroplane.mp3");

    if (areoSound == nullptr) {
		cerr << "Error loading sound: " << Mix_GetError() << endl;
	}
    else {
        Mix_VolumeChunk(areoSound, MIX_MAX_VOLUME / 4);
    }

    Mix_PlayMusic(bgm, -1);


    // Define the time step
    Uint32 time_step_ms = 1000 / 100;
    float time_step_s = time_step_ms / 10;
    Uint32 next_game_step = SDL_GetTicks();
    Uint32 lastTime = SDL_GetTicks(), currentTime;
    // Define the random number generator
    mt19937 rng;
    rng.seed(time(NULL));
    uniform_int_distribution<int> xDist(0, 800);
    uniform_int_distribution<int> yDist(0, 600);



    Text text(renderer, "ThaleahFat.TTF", 24, { 0, 0, 0}, 10, 10 );
    Text text2(renderer, "ThaleahFat.TTF", 24, { 0, 0, 0 }, 10, 40);
    Text text3(renderer, "ThaleahFat.TTF", 24, { 0, 0, 0 }, 10, 70);
    Text text4(renderer, "ThaleahFat.TTF", 24, { 0, 0, 0 }, 10, 90);
    Text scoreText(renderer, "ThaleahFat.TTF", 24, { 0, 0, 0 }, 450, 10);

//GAME LOOP
    bool running = true;
    while (running) {
        // Input Processing
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                exit(0);
            }
            else if (event.type == SDL_KEYDOWN) {
                character.handleEvent(event, jumpSound);
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    exit(0);
                }
            }

        }


        character.handleEvent(event, jumpSound);

        // Delay the game to match the time step
        Uint32 now = SDL_GetTicks();
        if (next_game_step > now) {
            SDL_Delay(next_game_step - now);
        }
        next_game_step += time_step_ms;

        // Update the spawn timer
        float timeElapsed = 0.0f;
        timeElapsed += time_step_s;

        /////////////
        currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        // Update
        plane.move(3);





        if (character.getState() == Character::Landed && !gameManager.levelcompleted) {
            gameManager.completeLevel();
        }

        if (plane.getX() < 0) {
			gameManager.completeLevel();
		}

		gameManager.update(plane, character, deltaTime);

		character.update(time_step_s);

        if (gameManager.activeLandingSpotIndex >= 0 && (character.getState() == Character::Parachute)) {
            SDL_Rect characterRect = { character.getX(), character.getY(), 70, 70 };
            SDL_Rect landingRect = { gameManager.landingSpots[gameManager.activeLandingSpotIndex].getX(),
                                    gameManager.landingSpots[gameManager.activeLandingSpotIndex].getY(), 30, 70 };

            if (SDL_HasIntersection(&characterRect, &landingRect)) {
                character.land();
                scoreText.updateScore(10);

            }
        }




        if (plane.getX() >= 750 && plane.getX()<=800) {
            Mix_PlayChannel(1, areoSound, 0);

        }


        // Drawing the background
        SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

        if (gameManager.activeLandingSpotIndex >= 0) {
            gameManager.landingSpots[gameManager.activeLandingSpotIndex].draw(renderer);
        }

        // Drawing the plane
        plane.draw(renderer, Aero);

        if (character.getState() != Character::Hidden) {
            character.draw(renderer, characterTexture);
        }

        SDL_Texture* currentWindTexture = character.windVelX < 0 ? leftFlag : rightFlag;
        SDL_Rect windRect = { 800, 480, 50, 50 };

        SDL_RenderCopy(renderer, currentWindTexture, NULL, &windRect);


        text.write("Level: " + to_string(gameManager.currentLevel), 20, 10);
        text2.write("'j' to Jump", 20, 40);
        text4.write("'p' to deploy parachute", 20, 70);
        text3.write("Wind Speed: " + to_string(gameManager.getWindSpeed()), 20, 90);

        scoreText.write("Score: " + to_string(scoreText.getScore()), 730, 10);
        SDL_RenderCopy(renderer, text3.texture, NULL, &text3.rect);
        SDL_RenderCopy(renderer, text.texture, NULL, &text.rect);
        SDL_RenderCopy(renderer, text2.texture, NULL, &text2.rect);
        SDL_RenderCopy(renderer, scoreText.texture, NULL, &scoreText.rect);
        SDL_RenderCopy(renderer, text4.texture, NULL, &text4.rect);




        // Show what was drawn
        SDL_RenderPresent(renderer);



    }



    // Giải phóng tài nguyên
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(Aero);
    // Giải phóng SDL_mixer
    Mix_FreeMusic(bgm);
    Mix_FreeChunk(jumpSound);
    Mix_FreeChunk(areoSound);
    Mix_CloseAudio();

    // Giải phóng renderer và cửa sổ
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    // Giải phóng SDL_ttf và thoát SDL
    TTF_Quit();
    SDL_Quit();

    return 0;


}
