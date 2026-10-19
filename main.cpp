#include "CS3113/Entity.h"

// Global Constants
constexpr int SCREEN_WIDTH  = 1000,
              SCREEN_HEIGHT = 600,
              FPS           = 120;

constexpr char BACKGROUND_IMAGE[] = "assets/space background.png";
constexpr char PLAYER_IMAGE[]     = "assets/space ship.png";

// How quickly gravity pulls the ship down and thrust changes its speed
constexpr float ACCELERATION_OF_GRAVITY = 10.0f,
                HORIZONTAL_ACCELERATION = 420.0f,
                HORIZONTAL_BRAKING     = 130.0f,
                FIXED_TIMESTEP         = 1.0f / 60.0f;

// Global Variables
AppStatus gAppStatus       = RUNNING;
float     gPreviousTicks   = 0.0f;

Entity     *gPlayer = nullptr;
Texture2D   gBackgroundTexture;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lunar Lander");

    // Load the space backdrop and create the player ship at top of screen
    gBackgroundTexture = LoadTexture(BACKGROUND_IMAGE);
    gPlayer = new Entity(
        { SCREEN_WIDTH / 2.0f, 110.0f },
        { 90.0f, 90.0f },
        PLAYER_IMAGE,
        PLAYER
    );

    // Smaller collision box than the ship so the collision area fits inside ship
    gPlayer->setColliderDimensions({ 45.0f, 55.0f });
    gPlayer->setAcceleration({ 0.0f, ACCELERATION_OF_GRAVITY });

    SetTargetFPS(FPS);
    gPreviousTicks = static_cast<float>(GetTime());
}

void processInput()
{
    if (IsKeyPressed(KEY_Q) || WindowShouldClose())
    {
        gAppStatus = TERMINATED;
        return;
    }

    // Set horizontal acceleration from the keys based on keys. When neither is held, brake against the drift
    Vector2 acceleration = gPlayer->getAcceleration();
    acceleration.x = 0.0f;

    if (IsKeyDown(KEY_A))
        acceleration.x = -HORIZONTAL_ACCELERATION;
    else if (IsKeyDown(KEY_D))
        acceleration.x = HORIZONTAL_ACCELERATION;
    else if (gPlayer->getVelocity().x > 0.0f)
        acceleration.x = -HORIZONTAL_BRAKING;
    else if (gPlayer->getVelocity().x < 0.0f)
        acceleration.x = HORIZONTAL_BRAKING;

    // Keep gravity applied while allowing the horizontal acceleration to change.
    acceleration.y = ACCELERATION_OF_GRAVITY;
    gPlayer->setAcceleration(acceleration);
}

void update()
{
    // Measure elapsed time that entity uses to run its fixed-step physics.
    float ticks     = static_cast<float>(GetTime());
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    // Pass empty collision lists for now.
    gPlayer->update(
        deltaTime,
        nullptr,
        0,
        nullptr,
        0,
        FIXED_TIMESTEP
    );
}

void render()
{
    BeginDrawing();

    ClearBackground(BLACK);

    // Stretch the background image to cover the full game window
    Rectangle textureBackgroundArea = {
        0.0f, 0.0f,
        static_cast<float>(gBackgroundTexture.width),
        static_cast<float>(gBackgroundTexture.height)
    };
    Rectangle destinationBackgroundArea = {
        0.0f, 0.0f,
        static_cast<float>(SCREEN_WIDTH),
        static_cast<float>(SCREEN_HEIGHT)
    };

    DrawTexturePro(
        gBackgroundTexture,
        textureBackgroundArea,
        destinationBackgroundArea,
        { 0.0f, 0.0f },
        0.0f,
        WHITE
    );

    // Draw the ship over the background
    gPlayer->render();

    EndDrawing();
}

void shutdown()
{
    // Release the textures 
    delete gPlayer;
    UnloadTexture(gBackgroundTexture);
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}
