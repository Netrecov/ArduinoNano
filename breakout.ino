#include <Wire.h>
#include "U8glib.h"

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);

// Button pins
#define LEFT_BUTTON 2
#define RIGHT_BUTTON 3
#define FIRE_BUTTON 4

// Game constants
#define WIDTH 128
#define HEIGHT 64
const unsigned int FRAME_RATE = 60;
const unsigned int COLUMNS = 13;
const unsigned int ROWS = 4;

// Game variables
int dx = -2, dy = -2;
int xb, yb;
byte xPaddle = 54;
boolean released = false;
boolean isHit[ROWS][COLUMNS];
byte lives = 3;
byte level = 1;
unsigned int score = 0;
unsigned int brickCount = 0;
boolean start = false;
char text_buffer[16];
unsigned long lastFrameTime = 0;

void setup() {
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  pinMode(FIRE_BUTTON, INPUT_PULLUP);
  randomSeed(analogRead(0));
  
  u8g.setFont(u8g_font_6x10);
  u8g.setColorIndex(1);
}

void loop() {
  unsigned long currentTime = millis();
  if (currentTime - lastFrameTime < (1000 / FRAME_RATE)) return;
  lastFrameTime = currentTime;

  if (!start) {
    showTitleScreen();
    return;
  }

  if (lives > 0) {
    updateGame();
  } else {
    showGameOver();
  }
}

void showTitleScreen() {
  static boolean showPressFire = true;
  static unsigned long lastToggle = 0;
  
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_10x20);
    u8g.drawStr(16, 30, "BREAKOUT");
    u8g.setFont(u8g_font_6x10);
    if (showPressFire) u8g.drawStr(31, 50, "PRESS FIRE!");
  } while (u8g.nextPage());

  if (millis() - lastToggle > 500) {
    showPressFire = !showPressFire;
    lastToggle = millis();
  }

  if (digitalRead(FIRE_BUTTON) == LOW) {
    delay(200);
    start = true;
    newLevel();
  }
}

void updateGame() {
  // Update game state first
  movePaddle();
  moveBall();

  // Then draw complete frame
  u8g.firstPage();
  do {
    // Clear screen
    u8g.setColorIndex(0);
    u8g.drawBox(0, 0, WIDTH, HEIGHT);
    
    // Draw game elements
    u8g.setColorIndex(1);
    
    // Draw bricks
    for (byte row = 0; row < ROWS; row++) {
      for (byte column = 0; column < COLUMNS; column++) {
        if (!isHit[row][column]) {
          u8g.drawBox(10 * column, 6 * row + 1, 8, 4);
        }
      }
    }
    
    // Draw paddle
    u8g.drawBox(xPaddle, HEIGHT - 1, 11, 1);
    
    // Draw ball
    u8g.drawPixel(xb, yb);
    u8g.drawPixel(xb + 1, yb);
    u8g.drawPixel(xb, yb + 1);
    u8g.drawPixel(xb + 1, yb + 1);
    
    // Draw score
    sprintf(text_buffer, "Score: %u", score);
    u8g.drawStr(0, 0, text_buffer);
  } while (u8g.nextPage());

  if (brickCount == ROWS * COLUMNS) {
    level++;
    newLevel();
  }
}

void movePaddle() {
  if (digitalRead(LEFT_BUTTON) == LOW && xPaddle > 0) {
    xPaddle -= 2;
  }
  if (digitalRead(RIGHT_BUTTON) == LOW && xPaddle < WIDTH - 12) {
    xPaddle += 2;
  }
}

void moveBall() {
  if (!released) {
    xb = xPaddle + 5;
    yb = HEIGHT - 4;
    if (digitalRead(FIRE_BUTTON) == LOW) {
      released = true;
      dx = random(0, 2) ? 1 : -1;
      dy = -1;
      delay(200);
    }
    return;
  }

  xb += dx;
  yb += dy;

  // Wall collisions
  if (xb <= 0 || xb >= WIDTH - 2) dx = -dx;
  if (yb <= 0) dy = -dy;
  
  // Bottom collision
  if (yb >= HEIGHT) {
    released = false;
    lives--;
    delay(1000);
    return;
  }

  // Paddle collision - FIXED THE MISSING PARENTHESIS HERE
  if (yb >= HEIGHT - 4 && xb >= xPaddle && xb <= xPaddle + 11) {
    dy = -dy;
    dx = ((xb - (xPaddle + 5)) / 2); // Added the missing parenthesis
  }

  // Brick collisions
  for (byte row = 0; row < ROWS; row++) {
    for (byte column = 0; column < COLUMNS; column++) {
      if (!isHit[row][column]) {
        byte brickLeft = 10 * column;
        byte brickRight = brickLeft + 10;
        byte brickTop = 6 * row + 1;
        byte brickBottom = brickTop + 6;

        if (xb >= brickLeft && xb <= brickRight && 
            yb >= brickTop && yb <= brickBottom) {
          isHit[row][column] = true;
          brickCount++;
          score += level * 10;
          
          if (xb < brickLeft || xb > brickRight) dx = -dx;
          if (yb < brickTop || yb > brickBottom) dy = -dy;
        }
      }
    }
  }
}

void newLevel() {
  xb = xPaddle + 5;
  yb = HEIGHT - 4;
  released = false;
  brickCount = 0;
  
  // Reset bricks
  for (byte row = 0; row < ROWS; row++) {
    for (byte column = 0; column < COLUMNS; column++) {
      isHit[row][column] = false;
    }
  }
}

void showGameOver() {
  u8g.firstPage();
  do {
    u8g.setColorIndex(1);
    u8g.setFont(u8g_font_6x10);
    u8g.drawStr(37, 30, "GAME OVER");
    sprintf(text_buffer, "Score: %u", score);
    u8g.drawStr(31, 45, text_buffer);
  } while (u8g.nextPage());
  
  delay(3000);
  resetGame();
}

void resetGame() {
  lives = 3;
  level = 1;
  score = 0;
  start = false;
}
