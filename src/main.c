/* Project Team:
-Sean Power A00027507
-Angelina Brovtsyna A00038794 
-Yaroslav Shulhin A00042614 

project description:
This is a space invaders game where you can play multiple games and try beat your score where you have 3 lives before you lose
we have 3 leds on our board, a buzzer, and 5 buttons. in this project we have used random numbers and i/O serial port, sound, and 
we have learned much from doing this project including the building of a game.
*/


#include <stm32f031x6.h>
#include "display.h"
#include "serial.h"
#include "sound.h"

/* ================================================================
   Hardware function prototypes
   ================================================================ */
void initClock(void);
void initSysTick(void);
void SysTick_Handler(void);
void delay(volatile uint32_t dly);
void setupIO(void);
void enablePullUp(GPIO_TypeDef *Port, uint32_t BitNumber);
void pinMode(GPIO_TypeDef *Port, uint32_t BitNumber, uint32_t Mode);



/* LED life indicators (PB0, PB1, PB3) */
void updateLEDs(uint8_t lives);

volatile uint32_t milliseconds;

/* ================================================================
   Colour definitions
   Custom RGB packing: bits[15:13]=G[2:0], [12:8]=R[7:3], [7:3]=B[7:3], [2:0]=G[7:5]
   ================================================================ */
#define COL_BLACK 0
#define COL_WHITE 65535
#define COL_GREEN 57351
#define COL_RED 7936
#define COL_YELLOW 65287
#define COL_CYAN 57599
#define COL_MAGENTA 8184

/* ================================================================
   Sprites
   ================================================================ */

/* Alien sprite: 12 wide x 10 tall (from alien1.bmp) */
const uint16_t alien_sprite[12 * 10] = {
       0, 1815,    0,    0,    0,    0,    0,    0,    0,    0, 1815,    0, /* row 0 */
       0,    0, 1815,    0,    0,    0,    0,    0,    0, 1815,    0,    0, /* row 1 */
       0,    0,    0, 1815, 1815, 1815, 1815, 1815, 1815,    0,    0,    0, /* row 2 */
       0,    0, 1815, 1815, 1815, 1815, 1815, 1815, 1815, 1815,    0,    0, /* row 3 */
       0, 1815, 1815, 1815,    0, 1815, 1815,    0, 1815, 1815, 1815,    0, /* row 4 */
       0, 1815,    0, 1815, 1815, 1815, 1815, 1815, 1815,    0, 1815,    0, /* row 5 */
       0, 1815,    0, 1815,    0,    0,    0,    0, 1815,    0, 1815,    0, /* row 6 */
    1815,    0,    0,    0, 1815,    0,    0, 1815,    0,    0,    0, 1815, /* row 7 */
       0,    0, 1815, 1815,    0,    0,    0,    0, 1815, 1815,    0,    0, /* row 8 */
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, /* row 9 */
};

/* Pre-explosion sprite: 11 wide x 11 tall (from PreBoom.bmp) */
const uint16_t preboom_sprite[11 * 11] = {
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, /* row  0 */
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, /* row  1 */
        0,     0,     0,     0,     0, 64005,     0,     0,     0,     0,     0, /* row  2 */
        0,     0,     0, 56075, 56075, 32016, 56075, 64005,     0,     0,     0, /* row  3 */
        0,     0,     0,     0, 32016, 56075, 32016, 56075,     0,     0,     0, /* row  4 */
        0,     0,     0, 64005, 56075, 32016, 56075, 32016,     0,     0,     0, /* row  5 */
        0,     0,     0, 32016,     0, 56075, 64005,     0,     0,     0,     0, /* row  6 */
        0,     0,     0,     0, 64005, 64005,     0,     0,     0,     0,     0, /* row  7 */
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, /* row  8 */
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, /* row  9 */
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, /* row 10 */
};


/* Explosion sprite: 11 wide x 11 tall (from boom.bmp) */
const uint16_t boom_sprite[11 * 11] = {
        0,  7694,     0,     0,  7694,     0, 15887,     0, 15887,     0,     0, /* row  0 */
        0,  7694, 40212, 15887,  7694,  7694,  7694, 15887, 40212, 15887, 15887, /* row  1 */
     7694,     0,  7701, 40212, 23571, 40212,  7694, 40468, 40212,  7694,     0, /* row  2 */
    15887, 40212,  7694, 40212,  7701, 40722,  7694, 40212, 23571, 15887,     0, /* row  3 */
        0, 15887,  7694, 40722, 40722, 40722, 40468,  7701, 40212,  7694, 15887, /* row  4 */
    40212, 40212,  7701, 40722, 40722, 40722, 40722, 40722, 40212, 23571,     0, /* row  5 */
    15887, 40212, 40212, 40212,  7701, 40722,  7701, 40212, 40468, 15887,     0, /* row  6 */
        0,  7694, 23571, 40212, 23571, 40212,  7694, 40212,  7701, 40468, 15887, /* row  7 */
    15887, 15887, 40468, 40212, 40212,  7701, 40468, 40212, 40212,  7694,     0, /* row  8 */
        0, 15887,  7694,  7694, 15887,  7694,  7694,  7694,  7694,     0,  7694, /* row  9 */
    15887,     0, 15887,     0,     0, 15887,     0,  7694,     0,  7694,     0, /* row 10 */
};

/* Player sprite: 7 wide x 10 tall (from ship1.bmp) */
const uint16_t player_sprite[7 * 10] = {
        0,     0,     0, 32767,     0,     0,     0, /* row 0 */
        0,     0, 32767, 65287, 32767,     0,     0, /* row 1 */
        0,     0, 32767, 32767, 32767,     0,     0, /* row 2 */
        0, 32767, 32767,     0, 32767, 32767,     0, /* row 3 */
        0, 32767,     0,     0,     0, 32767,     0, /* row 4 */
    32767, 32767, 32767,     0, 32767, 32767, 32767, /* row 5 */
        0, 32767, 32767, 32767, 32767, 32767,     0, /* row 6 */
        0, 32767, 32767, 32767, 32767, 32767,     0, /* row 7 */
    32767, 32767,     0, 32767,     0, 32767, 32767, /* row 8 */
    32767,     0,     0,     0,     0,     0, 32767, /* row 9 */
};


/* ================================================================
   Difficulty level — selected on menu
   0 = Easy, 1 = Medium, 2 = Hard
   ================================================================ */
static uint8_t difficulty = 1; /* default Medium */

/* ================================================================
   Pause state
   ================================================================ */

static uint8_t paused = 0;
static uint8_t pause_prev = 0;  /* for edge detection on PA11 */
static uint8_t reset_prev = 0;  /* for edge detection on PB6 (restart) */

/* ================================================================
   Game constants
   ================================================================ */
#define SCREEN_W 128
#define SCREEN_H 160
#define UI_Y 2       /* y position of score text */
#define UI_LINE_Y 13 /* separator line below UI bar */


#define MAX_ALIENS 12 /* max on screen at once */
#define ALIEN_W 12
#define ALIEN_H 10
#define ALIEN_SPAWN_Y 18 /* y where new aliens appear */

#define PLAYER_W 7
#define PLAYER_H 10
#define PLAYER_Y 148
#define PLAYER_SPEED 4

#define BOOM_W 11 // preboom/boom sprites
#define BOOM_H 11

#define BULLET_W 2
#define BULLET_H 5
#define BULLET_SPEED_P 6 /* player bullet: moves up */
#define BULLET_SPEED_A 2 /* alien bullet:  moves down */
#define MAX_ALIEN_BULLETS 5

#define MAX_STARS 30
#define STAR_REDRAW_PERIOD 40 /* frames between star redraws */

/* ================================================================
   Alien struct – each alien is independent
   ================================================================ */
typedef struct
{
    int16_t x, y;
    int8_t dx; /* horizontal drift: -1, 0, or +1 */
    uint8_t alive;
    uint16_t color;
} Alien;

/* Palette: blue, green, yellow, red, orange, purple */
static const uint16_t alien_colors[6] = { 248, 57351, 65287, 7936, 48901, 4816 };

/* ================================================================
   Game state
   ================================================================ */
static Alien aliens[MAX_ALIENS];
static int16_t player_x;
static uint16_t score;
static uint16_t high_score; /* persists across games within a session */
static uint8_t lives;

/* Player bullet */
static int16_t pbullet_x, pbullet_y;
static uint8_t pbullet_active;

/* Alien bullets */
static int16_t abullet_x[MAX_ALIEN_BULLETS];
static int16_t abullet_y[MAX_ALIEN_BULLETS];
static uint8_t abullet_active[MAX_ALIEN_BULLETS];

static uint32_t frame_count;
static uint8_t fire_prev;

/* Stars */
static uint8_t  star_x[MAX_STARS];
static uint8_t  star_y[MAX_STARS];
static uint16_t star_color[MAX_STARS];

/* ================================================================
   LCG pseudo-random number generator
   ================================================================ */
static uint32_t rand_state = 12345u;

static uint32_t simple_rand(void)
{
    rand_state = rand_state * 1664525u + 1013904223u;
    return rand_state;
}

/* ================================================================
   Difficulty – scales with both selected level AND score
   Easy:   slow fall, fewer aliens, relaxed spawning
   Medium: balanced (original behaviour)
   Hard:   fast fall, more aliens, aggressive spawning
   Score thresholds make each level progressively harder
   ================================================================ */

/* Frames between each 1-pixel downward step (lower = faster) */
static int fallPeriod(void)
{
    /* Base periods per difficulty: Easy=6, Medium=4, Hard=3 */
    int base = (difficulty == 0) ? 6 : (difficulty == 2) ? 3 : 4;

    /* Progressive speed-up with score */
    if (score >= 260) return (base > 2) ? base - 3 : 1;
    if (score >= 130) return (base > 1) ? base - 2 : 1;
    if (score >= 50)  return (base > 0) ? base - 1 : 1;
    return base;
}

/* Frames between spawn attempts */
static int spawnPeriod(void)
{
    int base = (difficulty == 0) ? 85 : (difficulty == 2) ? 45 : 65;
    /* Speed up spawning as score rises */
    if (score >= 200) return (base > 15) ? base - 20 : 15;
    if (score >= 100) return (base > 10) ? base - 10 : 10;
    return base;
}


/* Maximum aliens allowed on screen at once */
static int maxAlive(void)
{
    int base = (difficulty == 0) ? 6 : (difficulty == 2) ? 12 : 9;
    if (score >= 200) return base + 2;
    if (score >= 100) return base + 1;
    return base;
}

/* ================================================================
   Helper: count alive aliens
   ================================================================ */
static int countAlive(void)
{
    int i, n = 0;
    for (i = 0; i < MAX_ALIENS; i++)
        if (aliens[i].alive)
            n++;
    return n;
}

/* ================================================================
   Drawing helpers
   ================================================================ */
static void drawAlien(int i)
{
    int r, c;
    for (r = 0; r < ALIEN_H; r++)
        for (c = 0; c < ALIEN_W; c++)
            putPixel((uint16_t)(aliens[i].x + c), (uint16_t)(aliens[i].y + r),
                     alien_sprite[r * ALIEN_W + c] ? aliens[i].color : COL_BLACK);
}


static void eraseAlien(int i)
{
    fillRectangle((uint16_t)aliens[i].x, (uint16_t)aliens[i].y,
                  ALIEN_W, ALIEN_H, COL_BLACK);
}

static void drawPlayer(int16_t x)
{
    putImage((uint16_t)x, PLAYER_Y, PLAYER_W, PLAYER_H, player_sprite, 0, 0);
}

static void erasePlayer(int16_t x)
{
    fillRectangle((uint16_t)x, PLAYER_Y, PLAYER_W, PLAYER_H, COL_BLACK);
}

/* Two-frame explosion centred on the player: PreBoom → Boom → clear */
static void playerExplosion(int16_t x)
{
    int16_t bx = x + (PLAYER_W - BOOM_W) / 2;
    int16_t by = PLAYER_Y + (PLAYER_H - BOOM_H) / 2;
    if (bx < 0) bx = 0;
    if (by < 0) by = 0;
    putImage((uint16_t)bx, (uint16_t)by, BOOM_W, BOOM_H, preboom_sprite, 0, 0);
    delay(150);
    putImage((uint16_t)bx, (uint16_t)by, BOOM_W, BOOM_H, boom_sprite, 0, 0);
    delay(350);
    fillRectangle((uint16_t)bx, (uint16_t)by, BOOM_W, BOOM_H, COL_BLACK);
}

static void drawUI(void)
{
    printText("SC:", 0, UI_Y, COL_YELLOW, COL_BLACK);
    printNumber(score, 22, UI_Y, COL_YELLOW, COL_BLACK);

    /* Lives as small ship icons on the right */
    fillRectangle(113, UI_Y, 15, 9, COL_BLACK);
    if (lives >= 1)
    {
        putPixel(120, (uint16_t)(UI_Y), COL_RED);
        putPixel(119, (uint16_t)(UI_Y + 1), COL_RED);
        putPixel(120, (uint16_t)(UI_Y + 1), COL_RED);
        putPixel(121, (uint16_t)(UI_Y + 1), COL_RED);
        fillRectangle(118, (uint16_t)(UI_Y + 2), 5, 2, COL_RED);
    }
    if (lives >= 2)
    {
        putPixel(125, (uint16_t)(UI_Y), COL_YELLOW);
        putPixel(124, (uint16_t)(UI_Y + 1), COL_YELLOW);
        putPixel(125, (uint16_t)(UI_Y + 1), COL_YELLOW);
        putPixel(126, (uint16_t)(UI_Y + 1), COL_YELLOW);
        fillRectangle(123, (uint16_t)(UI_Y + 2), 5, 2, COL_YELLOW);
    }
    if (lives >= 3)
    {
        putPixel(115, (uint16_t)(UI_Y), COL_GREEN);
        putPixel(114, (uint16_t)(UI_Y + 1), COL_GREEN);
        putPixel(115, (uint16_t)(UI_Y + 1), COL_GREEN);
        putPixel(116, (uint16_t)(UI_Y + 1), COL_GREEN);
        fillRectangle(113, (uint16_t)(UI_Y + 2), 5, 2, COL_GREEN);
    }
}

/* ================================================================
   Spawn one alien into a free slot at a random x position
   ================================================================ */
static void spawnAlien(void)
{
    int i;
    for (i = 0; i < MAX_ALIENS; i++)
    {
        if (!aliens[i].alive)
        {
            aliens[i].x = (int16_t)(simple_rand() % (uint32_t)(SCREEN_W - ALIEN_W));
            aliens[i].y = ALIEN_SPAWN_Y;
            /* Weighted towards 0 (straight down): -1, 0, 0, +1 */
            {
                uint32_t r = simple_rand() % 4u;
                aliens[i].dx = (r == 0) ? -1 : (r == 3) ? 1
                                                        : 0;
            }
            aliens[i].color = alien_colors[simple_rand() % 6u];
            aliens[i].alive = 1;
            drawAlien(i);
            return;
        }

    }
}

/* Spread `count` aliens evenly across the top at game start */
static void spawnInitial(int count)
{
    int i;
    int16_t step = (int16_t)((SCREEN_W - ALIEN_W) / (count + 1));
    for (i = 0; i < count && i < MAX_ALIENS; i++)
    {
        aliens[i].x = (int16_t)(step * (i + 1));
        aliens[i].y = ALIEN_SPAWN_Y;
        aliens[i].dx = (int8_t)((int)(simple_rand() % 3u) - 1);
        aliens[i].color = alien_colors[simple_rand() % 6u];
        aliens[i].alive = 1;
    }
}

/* ================================================================
   Draw the difficulty label on the menu
   ================================================================ */
static const char *diffName(void)
{
    if (difficulty == 0) return "Easy  ";
    if (difficulty == 2) return "Hard  ";
    return "Medium";
}

static uint16_t diffColour(void)
{
    if (difficulty == 0) return COL_GREEN;
    if (difficulty == 2) return COL_RED;
    return COL_YELLOW;
}

static void drawDiffLabel(void)
{
    printText("< ", 10, 140, COL_WHITE, COL_BLACK);
    printText(diffName(), 25, 140, diffColour(), COL_BLACK);
    printText(" >", 65, 140, COL_WHITE, COL_BLACK);
}

/* ================================================================
   Main menu — left/right to change difficulty, fire to start
   ================================================================ */
static void showMenu(void)
{
    uint8_t left_prev = 1, right_prev = 1;

    fillRectangle(0, 0, SCREEN_W, SCREEN_H, COL_BLACK);

    printTextX2("Alien",    34, 12, COL_CYAN,  COL_BLACK);
    printTextX2("INVADERS", 16, 36, COL_GREEN, COL_BLACK);

    /* Decorative alien rows */
    putImage(14,  76, ALIEN_W, ALIEN_H, alien_sprite, 0, 0);
    putImage(42,  76, ALIEN_W, ALIEN_H, alien_sprite, 0, 0);
    putImage(70,  76, ALIEN_W, ALIEN_H, alien_sprite, 0, 0);
    putImage(98,  76, ALIEN_W, ALIEN_H, alien_sprite, 0, 0);
    putImage(28,  92, ALIEN_W, ALIEN_H, alien_sprite, 0, 0);
    putImage(56,  92, ALIEN_W, ALIEN_H, alien_sprite, 0, 0);
    putImage(84,  92, ALIEN_W, ALIEN_H, alien_sprite, 0, 0);


    printText("PRESS FIRE", 25, 115, COL_YELLOW, COL_BLACK);
    printText("TO START",   30, 125, COL_YELLOW, COL_BLACK);
    drawDiffLabel();

    /* High-score line if a previous game was played */
    if (high_score > 0)
    {
        printText("HI:", 0, 152, COL_CYAN, COL_BLACK);
        printNumber(high_score, 22, 152, COL_CYAN, COL_BLACK);
    }

    serialWriteLine("=== ALIEN INVADERS ===");
    serialWriteLine("Use LEFT/RIGHT to pick difficulty, FIRE to start");

    /* Loop: left/right change difficulty, fire starts game */
    while (1)
    {
        uint8_t left_now  = ((GPIOB->IDR & (1u << 5)) == 0) ? 0u : 1u;
        uint8_t right_now = ((GPIOB->IDR & (1u << 4)) == 0) ? 0u : 1u;
        uint8_t fire_now  = ((GPIOA->IDR & (1u << 8)) == 0) ? 0u : 1u;

        /* Left pressed (falling edge) → decrease difficulty */
        if (left_prev == 1 && left_now == 0)
        {
            if (difficulty > 0) difficulty--;
            drawDiffLabel();
        }
        /* Right pressed (falling edge) → increase difficulty */
        if (right_prev == 1 && right_now == 0)
        {
            if (difficulty < 2) difficulty++;
            drawDiffLabel();
        }
        /* Fire pressed → start game */
        if (fire_now == 0)
        {
            delay(50);
            while ((GPIOA->IDR & (1u << 8)) == 0);
            delay(80);
            serialWrite("Difficulty: ");
            serialWriteLine(diffName());
            return;
        }

        left_prev  = left_now;
        right_prev = right_now;
        delay(30);
    }
}

/* ================================================================
   background Stars
   ================================================================ */
static const uint16_t star_palette[5] = {
    65535, /* white  */
    65287, /* yellow */
    57599, /* cyan   */
    7936,  /* red    */
    248,   /* blue   */
};

static void initStars(void)
{
    int i;
    for (i = 0; i < MAX_STARS; i++)
    {
        star_x[i] = (uint8_t)(simple_rand() % SCREEN_W);
        star_y[i] = (uint8_t)(UI_LINE_Y + 2 + simple_rand() % (uint32_t)(PLAYER_Y - UI_LINE_Y - 4));
        star_color[i] = star_palette[simple_rand() % 5u];
    }
}

static void drawStars(void)
{
    int i;
    for (i = 0; i < MAX_STARS; i++)
        putPixel(star_x[i], star_y[i], star_color[i]);
}

/* ================================================================
   Initialise a new game
   ================================================================ */
static void initGame(void)
{
    int i;

    for (i = 0; i < MAX_ALIENS; i++)
        aliens[i].alive = 0;

    for (i = 0; i < MAX_ALIEN_BULLETS; i++)
        abullet_active[i] = 0;

    player_x = (SCREEN_W - PLAYER_W) / 2;
    pbullet_active = 0;
    frame_count = 0;
    fire_prev = 1;
    paused = 0;
    pause_prev = 0;
    reset_prev = 0;
    rand_state = milliseconds;


    fillRectangle(0, 0, SCREEN_W, SCREEN_H, COL_BLACK);
    drawUI();
    updateLEDs(lives);
    drawLine(0, UI_LINE_Y, SCREEN_W - 1, UI_LINE_Y, COL_CYAN);
    initStars();
    drawStars();

    /* Start with 3 aliens spread across the top */
    spawnInitial(3);
    for (i = 0; i < MAX_ALIENS; i++)
        if (aliens[i].alive)
            drawAlien(i);


    drawPlayer(player_x);
    serialWriteLine("Game started!");
}

/* ================================================================
   Gameplay loop
   ================================================================ */


static void runGame(void)
{
    uint8_t game_over = 0;
    int i;

    while (!game_over)
    {
        /* ---- Pause toggle — PA11 falling-edge detect ---- */
        {
            uint8_t pause_now = ((GPIOA->IDR & (1u << 11)) == 0) ? 0u : 1u;
            if (pause_prev == 1 && pause_now == 0)
            {
                paused = !paused;
                if (paused)
                {
                    printTextX2("PAUSED", 20, 75, COL_YELLOW, COL_BLACK);
                    serialWriteLine("Game paused");
                }
                else
                {
                    fillRectangle(20, 75, 90, 16, COL_BLACK);
                    serialWriteLine("Game resumed");
                    /* Redraw anything the pause text may have covered */
                    for (i = 0; i < MAX_ALIENS; i++)
                        if (aliens[i].alive) drawAlien(i);
                    drawPlayer(player_x);
                }
            }
            pause_prev = pause_now;

        }

        /* ---- Restart — PB6 falling-edge detect ---- */
        {
            uint8_t reset_now = ((GPIOB->IDR & (1u << 6)) == 0) ? 0u : 1u;
            if (reset_prev == 1 && reset_now == 0)
            {
                serialWriteLine("Game restarted");
                game_over = 1;
            }
            reset_prev = reset_now;
        }

        /* If paused, also check for serial 'p' to unpause */
        if (paused)
        {
            char sc = serialRead();
            if (sc == 'p' || sc == 'P')
            {
                paused = 0;
                fillRectangle(20, 75, 90, 16, COL_BLACK);
                serialWriteLine("Game resumed (serial)");
                for (i = 0; i < MAX_ALIENS; i++)
                    if (aliens[i].alive) drawAlien(i);
                drawPlayer(player_x);
            }
            delay(50);
            continue;
        }

        frame_count++;

        /* ---- Check serial input for commands ---- */
        {
            char sc = serialRead();
            if (sc == 'p' || sc == 'P')
            {
                paused = 1;
                printTextX2("PAUSED", 20, 75, COL_YELLOW, COL_BLACK);
                serialWriteLine("Game paused (serial)");
                continue;
            }
        }

        /* ---- Player movement ---- */
        {
            int16_t old_px = player_x;
            if ((GPIOB->IDR & (1u << 4)) == 0) /* right */
            {
                if (player_x + PLAYER_W < SCREEN_W)
                    player_x += PLAYER_SPEED;
            }

            if ((GPIOB->IDR & (1u << 5)) == 0) /* left */
            {
                if (player_x > 0)
                    player_x -= PLAYER_SPEED;
            }
            if (player_x != old_px)
            {
                erasePlayer(old_px);
                drawPlayer(player_x);
            }
        }

        /* ---- Fire – rising-edge detect on PA8 ---- */
        {
            uint8_t fire_now = ((GPIOA->IDR & (1u << 8)) == 0) ? 0u : 1u;
            if (!pbullet_active && fire_prev == 1 && fire_now == 0)
            {
                pbullet_active = 1;
                pbullet_x = player_x + PLAYER_W / 2 - BULLET_W / 2;
                pbullet_y = PLAYER_Y - BULLET_H - 1;
            }
            fire_prev = fire_now;
        }

        /* ---- Move player bullet (upward) ---- */
        if (pbullet_active)
        {
            fillRectangle((uint16_t)pbullet_x, (uint16_t)pbullet_y,
                          BULLET_W, BULLET_H, COL_BLACK);
            pbullet_y -= BULLET_SPEED_P;
            if (pbullet_y < UI_LINE_Y + 2)
            {
                pbullet_active = 0;
            }
            else
            {
                fillRectangle((uint16_t)pbullet_x, (uint16_t)pbullet_y,
                              BULLET_W, BULLET_H, COL_RED);
            }

        }

        /* ---- Move alien bullets (downward) ---- */
        for (i = 0; i < MAX_ALIEN_BULLETS; i++)
        {
            if (!abullet_active[i])
                continue;

            fillRectangle((uint16_t)abullet_x[i], (uint16_t)abullet_y[i],
                          BULLET_W, BULLET_H, COL_BLACK);
            abullet_y[i] += BULLET_SPEED_A;

            if (abullet_y[i] > SCREEN_H)
            {
                abullet_active[i] = 0;
            }
            else
            {
                fillRectangle((uint16_t)abullet_x[i], (uint16_t)abullet_y[i],
                              BULLET_W, BULLET_H, COL_MAGENTA);
            }
        }

        /* ---- Alien fall (every fallPeriod frames) ---- */
        if (frame_count % (uint32_t)fallPeriod() == 0)
        {
            for (i = 0; i < MAX_ALIENS; i++)
            {
                if (!aliens[i].alive)
                    continue;

                eraseAlien(i);

                /* Move down */
                aliens[i].y += 1;

                /* Horizontal drift (3x slower than vertical) */
                if (frame_count % ((uint32_t)fallPeriod() * 3u) == 0)
                {
                    aliens[i].x += aliens[i].dx;
                    if (aliens[i].x < 0)
                    {
                        aliens[i].x = 0;
                        aliens[i].dx = 1;
                    }

                    if (aliens[i].x + ALIEN_W > SCREEN_W)
                    {
                        aliens[i].x = (int16_t)(SCREEN_W - ALIEN_W);
                        aliens[i].dx = -1;
                    }
                }

                /* Reached player line → lose a life */
                if (aliens[i].y + ALIEN_H >= PLAYER_Y)
                {
                    aliens[i].alive = 0;
                    if (lives > 0) lives--;
                    drawUI();
                    updateLEDs(lives);
                    soundLifeLost();
                    serialWrite("Life lost! Lives: ");
                    serialWriteNum(lives);
                    serialWriteLine("");

                    erasePlayer(player_x);
                    playerExplosion(player_x);

                    if (lives == 0)
                    {
                        game_over = 1;
                    }
                    else
                    {
                        drawPlayer(player_x);
                    }
                }
                else
                {
                    drawAlien(i);
                }
            }
        }

        /* ---- Redraw stars erased by sprites/bullets ---- */
        if (frame_count % STAR_REDRAW_PERIOD == 0)
            drawStars();

        /* ---- Spawn new aliens ---- */
        if (frame_count % (uint32_t)spawnPeriod() == 3u)
        {
            if (countAlive() < maxAlive())
                spawnAlien();
        }

        /* ---- Alien shooting (every 55 frames on Medium, scaled) ---- */
        {
            uint32_t shoot_rate = (difficulty == 0) ? 70u : (difficulty == 2) ? 40u : 55u;
            if (frame_count % shoot_rate == 7u)
            {
                int slot = -1;
                for (i = 0; i < MAX_ALIEN_BULLETS; i++)
                    if (!abullet_active[i])
                    {
                        slot = i;
                        break;
                    }

                if (slot >= 0)
                {
                    int cands[MAX_ALIENS], nc = 0;
                    for (i = 0; i < MAX_ALIENS; i++)
                        if (aliens[i].alive)
                            cands[nc++] = i;

                    if (nc > 0)
                    {
                        int ai = cands[(int)(simple_rand() % (uint32_t)nc)];
                        abullet_x[slot] = (int16_t)(aliens[ai].x + ALIEN_W / 2);
                        abullet_y[slot] = (int16_t)(aliens[ai].y + ALIEN_H);
                        abullet_active[slot] = 1;
                    }
                }
            }
        }

        /* ---- Collision: player bullet vs aliens ---- */
        if (pbullet_active)
        {
            for (i = 0; i < MAX_ALIENS && pbullet_active; i++)
            {
                if (!aliens[i].alive)
                    continue;

                if (pbullet_x + BULLET_W > aliens[i].x &&
                    pbullet_x < aliens[i].x + ALIEN_W &&
                    pbullet_y + BULLET_H > aliens[i].y &&
                    pbullet_y < aliens[i].y + ALIEN_H)
                {
                    eraseAlien(i);
                    aliens[i].alive = 0;

                    fillRectangle((uint16_t)pbullet_x, (uint16_t)pbullet_y,
                                  BULLET_W, BULLET_H, COL_BLACK);
                    pbullet_active = 0;

                    score += 10;
                    if (score > high_score)
                        high_score = score;

                    drawUI();
                    serialWrite("Score: ");
                    serialWriteNum(score);
                    serialWriteLine("");
                }
            }
        }

        /* ---- Collision: alien bullet vs player ---- */
        for (i = 0; i < MAX_ALIEN_BULLETS; i++)
        {
            if (!abullet_active[i])
                continue;

            if (abullet_x[i] + BULLET_W > player_x &&
                abullet_x[i] < player_x + PLAYER_W &&
                abullet_y[i] + BULLET_H > PLAYER_Y &&
                abullet_y[i] < PLAYER_Y + PLAYER_H)
            {
                fillRectangle((uint16_t)abullet_x[i], (uint16_t)abullet_y[i],
                              BULLET_W, BULLET_H, COL_BLACK);
                abullet_active[i] = 0;

                erasePlayer(player_x);
                playerExplosion(player_x);

                if (lives > 0) lives--;
                drawUI();
                updateLEDs(lives);
                soundLifeLost();
                serialWrite("Hit! Lives: ");
                serialWriteNum(lives);
                serialWriteLine("");

                if (lives == 0)
                {
                    game_over = 1;
                }
                else{
                    drawPlayer(player_x);
                }
            }
        }

        delay(50);
    }

    /* ---- Game over ---- */
    soundGameOver();
    updateLEDs(0);

    serialWriteLine("=== GAME OVER ===");
    serialWrite("Final score: ");
    serialWriteNum(score);
    serialWriteLine("");
    serialWrite("High score: ");
    serialWriteNum(high_score);
    serialWriteLine("");


    fillRectangle(0, 0, SCREEN_W, SCREEN_H, COL_BLACK);
    printTextX2("GAME OVER", 10, 50, COL_RED, COL_BLACK);
    printText("Score:", 30, 80, COL_WHITE, COL_BLACK);
    printNumberX2(score, 30, 92, COL_YELLOW, COL_BLACK);
    printText("High:", 30, 115, COL_WHITE, COL_BLACK);
    printNumberX2(high_score, 30, 127, COL_CYAN, COL_BLACK);
    printText("Press fire", 25, 150, COL_WHITE, COL_BLACK);

    /* Wait for fire button */
    while ((GPIOA->IDR & (1u << 8)) == 0);
    delay(50);
    while ((GPIOA->IDR & (1u << 8)) != 0);
    delay(80);
}
   
/* ================================================================
   Application entry point
   ================================================================ */
int main(void)
{
    initClock();
    initSysTick();
    setupIO();

    while (1)
    {
        score = 0;
        lives = 3;
        showMenu();
        initGame();
        runGame();
    }

    return 0;
}

/* ================================================================
   Hardware / peripheral setup
   ================================================================ */
void initSysTick(void)
{
    SysTick->LOAD = 48000;
    SysTick->CTRL = 7;
    SysTick->VAL = 10;
    __asm(" cpsie i ");
}

void SysTick_Handler(void)
{
    milliseconds++;
}

void initClock(void)
{
    RCC->CR &= ~(1u << 24);
    while (RCC->CR & (1u << 25))
        ;

    FLASH->ACR |= (1u << 0);
    FLASH->ACR &= ~((1u << 2) | (1u << 1));
    FLASH->ACR |= (1u << 4);

    RCC->CFGR &= ~((1u << 21) | (1u << 20) | (1u << 19) | (1u << 18));
    RCC->CFGR |= ((1u << 21) | (1u << 19));
    RCC->CFGR |= (1u << 14);

    RCC->CR |= (1u << 24);
    RCC->CFGR |= (1u << 1);
}

void delay(volatile uint32_t dly)
{
    uint32_t end_time = dly + milliseconds;
    while (milliseconds != end_time)
        __asm(" wfi ");
}

void enablePullUp(GPIO_TypeDef *Port, uint32_t BitNumber)
{
    Port->PUPDR = Port->PUPDR & ~(3u << (BitNumber * 2));
    Port->PUPDR = Port->PUPDR | (1u << (BitNumber * 2));
}

void pinMode(GPIO_TypeDef *Port, uint32_t BitNumber, uint32_t Mode)
{
    uint32_t mv = Port->MODER;
    mv &= ~(3u << (BitNumber * 2));
    mv |= (Mode << (BitNumber * 2));
    Port->MODER = mv;
}

void setupIO(void)
{
    RCC->AHBENR |= (1u << 18) | (1u << 17); /* Enable GPIOB, GPIOA clocks */
    display_begin();

    /* Buttons: PB4 (right), PB5 (left), PA8 (fire), PA11 (pause), PB6 (restart) */
    pinMode(GPIOB, 4, 0);
    enablePullUp(GPIOB, 4);
    pinMode(GPIOB, 5, 0);
    enablePullUp(GPIOB, 5);
    pinMode(GPIOA, 8, 0);
    enablePullUp(GPIOA, 8);
    pinMode(GPIOA, 11, 0);
    enablePullUp(GPIOA, 11);
    pinMode(GPIOB, 6, 0);
    enablePullUp(GPIOB, 6);

    /* LEDs: PB0, PB1, PB3 as outputs (life indicators) */
    pinMode(GPIOB, 0, 1);
    pinMode(GPIOB, 1, 1);
    pinMode(GPIOB, 3, 1);
    updateLEDs(3); /* all on at start */

    /* Buzzer: TIM2_CH1 on PA0 (PWM via sound.c) */
    initSound();

    /* Serial: USART1 on PA2 (TX) and PA15 (RX) */
    initSerial();
}



/* ================================================================
   LED life indicators — PB1=red, PB0=yellow, PB3=green
   3 lives = all on, 2 = red+yellow, 1 = red only, 0 = all off
   ================================================================ */
void updateLEDs(uint8_t l){
    if (l >= 1) GPIOB->ODR |=  (1u << 1); else GPIOB->ODR &= ~(1u << 1); /* red    */
    if (l >= 2) GPIOB->ODR |=  (1u << 0); else GPIOB->ODR &= ~(1u << 0); /* yellow */
    if (l >= 3) GPIOB->ODR |=  (1u << 3); else GPIOB->ODR &= ~(1u << 3); /* green  */
}