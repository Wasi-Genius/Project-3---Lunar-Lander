#include "CS3113/Entity.h"

// Global Constants
constexpr int SCREEN_WIDTH  = 1000,
              SCREEN_HEIGHT = 600,
              FPS           = 120;

constexpr char BACKGROUND_IMAGE[] = "assets/space background.png";
constexpr char PLAYER_IMAGE[]     = "assets/space ship.png";
constexpr char GOOD_PLATFORM_IMAGE[] = "assets/platform.png";
constexpr char BAD_PLATFORM_IMAGE[]  = "assets/asteroid.png";
constexpr char MISSION_ACCOMPLISHED_IMAGE[] = "assets/Mission Accomplished Screen.png";
constexpr char MISSION_FAILED_IMAGE[] = "assets/Mission Failed Screen.png";

constexpr int NUMBER_OF_PLATFORMS = 5;
Vector2 gGoodPlatformPositions[NUMBER_OF_PLATFORMS] = {
    { 125.0f, 280.0f },
    { 500.0f, 355.0f },
    { 875.0f, 280.0f },
    { 290.0f, 485.0f },
    { 750.0f, 500.0f }
};
constexpr float GOOD_PLATFORM_LEFT_LIMITS[NUMBER_OF_PLATFORMS] = {
    80.0f, 455.0f, 830.0f, 245.0f, 705.0f
};
constexpr float GOOD_PLATFORM_RIGHT_LIMITS[NUMBER_OF_PLATFORMS] = {
    170.0f, 545.0f, 920.0f, 335.0f, 795.0f
};
float gGoodPlatformDirections[NUMBER_OF_PLATFORMS] = {
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f
};

Vector2 gBadPlatformPositions[NUMBER_OF_PLATFORMS] = {
    { 320.0f, 220.0f },
    { 700.0f, 220.0f },
    { 130.0f, 425.0f },
    { 520.0f, 505.0f },
    { 890.0f, 420.0f }
};
constexpr float BAD_PLATFORM_LEFT_LIMITS[3] = {
    275.0f, 655.0f, 85.0f
};
constexpr float BAD_PLATFORM_RIGHT_LIMITS[3] = {
    365.0f, 745.0f, 175.0f
};
constexpr float BAD_PLATFORM_TOP_LIMITS[2] = {
    465.0f, 380.0f
};
constexpr float BAD_PLATFORM_BOTTOM_LIMITS[2] = {
    545.0f, 460.0f
};
float gBadPlatformDirections[NUMBER_OF_PLATFORMS] = {
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f
};

constexpr Vector2 GOOD_PLATFORM_SIZE = { 140.0f, 79.0f };
constexpr Vector2 BAD_PLATFORM_SIZE  = { 52.0f, 52.0f };
constexpr Vector2 BAD_PLATFORM_COLLIDER_SIZE = { 44.0f, 38.0f };
constexpr float GOOD_PLATFORM_SPEED = 35.0f,
                BAD_PLATFORM_SPEED = 30.0f;
constexpr float END_SCREEN_DURATION = 3.0f;
constexpr Vector2 END_SCREEN_SIZE = { 800.0f, 400.0f };

// How quickly gravity pulls the ship down and thrust changes its speed
constexpr float ACCELERATION_OF_GRAVITY = 10.0f,
                HORIZONTAL_ACCELERATION = 420.0f,
                HORIZONTAL_BRAKING     = 130.0f,
                FIXED_TIMESTEP         = 1.0f / 60.0f;

// Global Variables
AppStatus gAppStatus       = RUNNING;
float     gPreviousTicks   = 0.0f;
float     gEndScreenTimer  = 0.0f;
bool      gGameOver        = false;
bool      gMissionAccomplished = false;

Entity     *gPlayer = nullptr;
Texture2D   gBackgroundTexture;
Texture2D   gGoodPlatformTexture;
Texture2D   gBadPlatformTexture;
Texture2D   gMissionAccomplishedTexture;
Texture2D   gMissionFailedTexture;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();
void drawTexture(const Texture2D *texture, const Vector2 *position, const Vector2 *scale);
bool isColliding(Vector2 positionA, Vector2 scaleA, Vector2 positionB, Vector2 scaleB);
void endGame(bool missionAccomplished);

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lunar Lander");

    // Load the space backdrop and create the player ship at top of screen
    gBackgroundTexture = LoadTexture(BACKGROUND_IMAGE);
    gGoodPlatformTexture = LoadTexture(GOOD_PLATFORM_IMAGE);
    gBadPlatformTexture = LoadTexture(BAD_PLATFORM_IMAGE);
    gMissionAccomplishedTexture = LoadTexture(MISSION_ACCOMPLISHED_IMAGE);
    gMissionFailedTexture = LoadTexture(MISSION_FAILED_IMAGE);
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
    if (WindowShouldClose())
    {
        gAppStatus = TERMINATED;
        return;
    }

    // Ignore gameplay controls while the result screen is shown
    if (gGameOver)
        return;

    if (IsKeyPressed(KEY_Q))
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

    // Keep the result screen visible for a few seconds before closing
    if (gGameOver)
    {
        gEndScreenTimer -= deltaTime;
        if (gEndScreenTimer <= 0.0f)
            gAppStatus = TERMINATED;
        return;
    }

    // Move all good platforms left and right
    for (int i = 0; i < NUMBER_OF_PLATFORMS; i++)
    {
        gGoodPlatformPositions[i].x +=
            GOOD_PLATFORM_SPEED * gGoodPlatformDirections[i] * deltaTime;

        if (gGoodPlatformPositions[i].x < GOOD_PLATFORM_LEFT_LIMITS[i])
        {
            gGoodPlatformPositions[i].x = GOOD_PLATFORM_LEFT_LIMITS[i];
            gGoodPlatformDirections[i] = 1.0f;
        }
        else if (gGoodPlatformPositions[i].x > GOOD_PLATFORM_RIGHT_LIMITS[i])
        {
            gGoodPlatformPositions[i].x = GOOD_PLATFORM_RIGHT_LIMITS[i];
            gGoodPlatformDirections[i] = -1.0f;
        }
    }

    // The first three asteroids move horizontally
    for (int i = 0; i < 3; i++)
    {
        gBadPlatformPositions[i].x +=
            BAD_PLATFORM_SPEED * gBadPlatformDirections[i] * deltaTime;

        if (gBadPlatformPositions[i].x < BAD_PLATFORM_LEFT_LIMITS[i])
        {
            gBadPlatformPositions[i].x = BAD_PLATFORM_LEFT_LIMITS[i];
            gBadPlatformDirections[i] = 1.0f;
        }
        else if (gBadPlatformPositions[i].x > BAD_PLATFORM_RIGHT_LIMITS[i])
        {
            gBadPlatformPositions[i].x = BAD_PLATFORM_RIGHT_LIMITS[i];
            gBadPlatformDirections[i] = -1.0f;
        }
    }

    // Remaining two asteroids move vertically
    for (int i = 3; i < NUMBER_OF_PLATFORMS; i++)
    {
        int limitIndex = i - 3;
        gBadPlatformPositions[i].y +=
            BAD_PLATFORM_SPEED * gBadPlatformDirections[i] * deltaTime;

        if (gBadPlatformPositions[i].y < BAD_PLATFORM_TOP_LIMITS[limitIndex])
        {
            gBadPlatformPositions[i].y = BAD_PLATFORM_TOP_LIMITS[limitIndex];
            gBadPlatformDirections[i] = 1.0f;
        }
        else if (gBadPlatformPositions[i].y >
                 BAD_PLATFORM_BOTTOM_LIMITS[limitIndex])
        {
            gBadPlatformPositions[i].y =
                BAD_PLATFORM_BOTTOM_LIMITS[limitIndex];
            gBadPlatformDirections[i] = -1.0f;
        }
    }

    Vector2 previousPosition = gPlayer->getPosition();

    // Pass empty collision lists to the Entity update for collision detection
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

    // Touching any part of an asteroid fails the mission
    for (int i = 0; i < NUMBER_OF_PLATFORMS; i++)
    {
        if (isColliding(
                position,
                gPlayer->getColliderDimensions(),
                gBadPlatformPositions[i],
                BAD_PLATFORM_COLLIDER_SIZE))
        {
            endGame(false);
            break;
        }
    }

    // A successful landing happens when the ship lands on top of a platform
    if (!gGameOver && velocity.y > 0.0f)
    {
        float previousShipBottom = previousPosition.y + scale.y / 2.0f;
        float currentShipBottom = position.y + scale.y / 2.0f;

        for (int i = 0; i < NUMBER_OF_PLATFORMS; i++)
        {
            float platformTop =
                gGoodPlatformPositions[i].y - GOOD_PLATFORM_SIZE.y / 2.0f;
            bool overlapsPlatformHorizontally =
                fabs(position.x - gGoodPlatformPositions[i].x) <=
                (scale.x + GOOD_PLATFORM_SIZE.x) / 2.0f;
            bool crossedPlatformTop =
                previousShipBottom <= platformTop &&
                currentShipBottom >= platformTop;

            if (overlapsPlatformHorizontally && crossedPlatformTop)
            {
                endGame(true);
                break;
            }
        }
    }

    // End the game when the ship's center passes the bottom edge
    if (!gGameOver && position.y > SCREEN_HEIGHT)
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

    if (gGameOver)
    {
        const Texture2D *endScreenTexture = gMissionAccomplished
            ? &gMissionAccomplishedTexture
            : &gMissionFailedTexture;
        Vector2 endScreenPosition = {
            SCREEN_WIDTH / 2.0f,
            SCREEN_HEIGHT / 2.0f
        };

        drawTexture(
            endScreenTexture,
            &endScreenPosition,
            &END_SCREEN_SIZE
        );
        EndDrawing();
        return;
    }

    // Draw the platforms both good and bad
    for (int i = 0; i < NUMBER_OF_PLATFORMS; i++)
    {
        drawTexture(
            &gGoodPlatformTexture,
            &gGoodPlatformPositions[i],
            &GOOD_PLATFORM_SIZE
        );
        drawTexture(
            &gBadPlatformTexture,
            &gBadPlatformPositions[i],
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
    UnloadTexture(gMissionAccomplishedTexture);
    UnloadTexture(gMissionFailedTexture);
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

bool isColliding(Vector2 positionA, Vector2 scaleA,
                 Vector2 positionB, Vector2 scaleB)
{
    float xDistance = fabs(positionA.x - positionB.x) - (scaleA.x + scaleB.x) / 2.0f;
    float yDistance = fabs(positionA.y - positionB.y) - (scaleA.y + scaleB.y) / 2.0f;

    return xDistance <= 0.0f && yDistance <= 0.0f;
}

void endGame(bool missionAccomplished)
{
    gGameOver = true;
    gMissionAccomplished = missionAccomplished;
    gEndScreenTimer = END_SCREEN_DURATION;
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
