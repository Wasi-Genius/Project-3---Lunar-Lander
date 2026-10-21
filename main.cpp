#include "CS3113/Entity.h"

// Global Constants
constexpr int SCREEN_WIDTH  = 1000,
              SCREEN_HEIGHT = 600,
              FPS           = 120;

constexpr char BACKGROUND_IMAGE[] = "assets/space background.png";
constexpr char PLAYER_IMAGE[]     = "assets/space ship.png";
constexpr char GOOD_PLATFORM_IMAGE[] = "assets/platform.png";
constexpr char BAD_PLATFORM_IMAGE[]  = "assets/asteroid.png";

constexpr int NUMBER_OF_PLATFORMS = 5;
constexpr Vector2 GOOD_PLATFORM_POSITIONS[NUMBER_OF_PLATFORMS] = {
    { 125.0f, 280.0f },
    { 500.0f, 355.0f },
    { 875.0f, 280.0f },
    { 290.0f, 485.0f },
    { 750.0f, 500.0f }
};
constexpr Vector2 BAD_PLATFORM_POSITIONS[NUMBER_OF_PLATFORMS] = {
    { 320.0f, 220.0f },
    { 700.0f, 220.0f },
    { 130.0f, 425.0f },
    { 520.0f, 505.0f },
    { 890.0f, 420.0f }
};

constexpr Vector2 GOOD_PLATFORM_SIZE = { 140.0f, 79.0f };
constexpr Vector2 BAD_PLATFORM_SIZE  = { 52.0f, 52.0f };

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
Texture2D   gGoodPlatformTexture;
Texture2D   gBadPlatformTexture;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();
void drawTexture(const Texture2D *texture, const Vector2 *position,
                 const Vector2 *scale);

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lunar Lander");

    // Load the space backdrop and create the player ship at top of screen
    gBackgroundTexture = LoadTexture(BACKGROUND_IMAGE);
    gGoodPlatformTexture = LoadTexture(GOOD_PLATFORM_IMAGE);
    gBadPlatformTexture = LoadTexture(BAD_PLATFORM_IMAGE);
    gPlayer = new Entity(
        { SCREEN_WIDTH / 2.0f, 110.0f },
        { 72.0f, 72.0f },
        PLAYER_IMAGE,
        PLAYER
    );

    // Smaller collision box than the ship so the collision area fits inside ship
    gPlayer->setColliderDimensions({ 36.0f, 44.0f });
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
    Vector2 position = gPlayer->getPosition();
    float halfShipWidth = gPlayer->getScale().x / 2.0f;
    acceleration.x = 0.0f;

    if (IsKeyDown(KEY_A) && position.x > halfShipWidth)
        acceleration.x = -HORIZONTAL_ACCELERATION;
    else if (IsKeyDown(KEY_D) && position.x < SCREEN_WIDTH - halfShipWidth)
        acceleration.x = HORIZONTAL_ACCELERATION;
    else if (IsKeyDown(KEY_A) || IsKeyDown(KEY_D))
        acceleration.x = 0.0f;
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

    Vector2 position = gPlayer->getPosition();
    Vector2 scale = gPlayer->getScale();
    Vector2 velocity = gPlayer->getVelocity();
    float halfShipWidth = scale.x / 2.0f;

    // Keep the ship inside the screen and bounce it away from a side edge
    if (position.x < halfShipWidth)
    {
        position.x = halfShipWidth;
        if (velocity.x < 0.0f)
            velocity.x = -velocity.x;
    }
    else if (position.x > SCREEN_WIDTH - halfShipWidth)
    {
        position.x = SCREEN_WIDTH - halfShipWidth;
        if (velocity.x > 0.0f)
            velocity.x = -velocity.x;
    }

    gPlayer->setPosition(position);
    gPlayer->setVelocity(velocity);

    // End the game when the ship's center passes the bottom edge
    if (position.y > SCREEN_HEIGHT)
        gAppStatus = TERMINATED;
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

    // Draw the platforms both good and bad
    for (int i = 0; i < NUMBER_OF_PLATFORMS; i++)
    {
        drawTexture(
            &gGoodPlatformTexture,
            &GOOD_PLATFORM_POSITIONS[i],
            &GOOD_PLATFORM_SIZE
        );
        drawTexture(
            &gBadPlatformTexture,
            &BAD_PLATFORM_POSITIONS[i],
            &BAD_PLATFORM_SIZE
        );
    }

    // Draw the ship over the background
    gPlayer->render();

    EndDrawing();
}

void shutdown()
{
    // Release textures 
    delete gPlayer;
    UnloadTexture(gBackgroundTexture);
    UnloadTexture(gGoodPlatformTexture);
    UnloadTexture(gBadPlatformTexture);
    CloseWindow();
}

// Drawing a texture helper function
void drawTexture(const Texture2D *texture, const Vector2 *position,
                 const Vector2 *scale)
{
    Rectangle textureArea = {
        0.0f, 0.0f,
        static_cast<float>(texture->width),
        static_cast<float>(texture->height)
    };
    Rectangle destinationArea = {
        position->x,
        position->y,
        scale->x,
        scale->y
    };
    Vector2 originOffset = { scale->x / 2.0f, scale->y / 2.0f };

    DrawTexturePro(
        *texture,
        textureArea,
        destinationArea,
        originOffset,
        0.0f,
        WHITE
    );
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
