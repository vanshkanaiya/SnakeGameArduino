#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 oled(128, 64, &Wire, -1);

#define RIGHT 0
#define LEFT  1
#define UP    2
#define DOWN  3

// STICKBOX Pins
#define pinX  A0
#define pinY  A1
#define pinSW A2  // Joystick Button Pin

int valueX = 0;
int valueY = 0;

// Snake Values
const uint8_t block[] PROGMEM = {
  0xf0, //B1111000
  0xb0, //B1011000
  0xd0, //B1101000
  0xf0, //B1111000
};

uint8_t snake_head_x;
uint8_t snake_head_y;
uint8_t x[100];
uint8_t y[100];
uint8_t snake_len;
uint8_t snake_dir;

uint8_t food_x;
uint8_t food_y;
bool food_eaten;

bool game_started = false; // Tracks if we are in the main menu
bool game_over = false;    // Tracks game over state
int score = 0;
int level = 1;
int snake_speed = 150;
int i;

void reset_game(void);
void check_snake_die(void);

// Initialize or Reset all game state variables
void reset_game(void)
{
  snake_head_x = 24;  
  snake_head_y = 24;
  snake_len = 3;      
  snake_dir = RIGHT;
  
  for(int i = 0; i < 100; i++) {
    x[i] = 0;
    y[i] = 0;
  }
  x[0] = snake_head_x;
  y[0] = snake_head_y;
  
  food_eaten = true;
  game_over = false;
  score = 0;
  level = 1;
  snake_speed = 150;
}

// Directional control scanner
void keyScan(void)
{
  static unsigned char keyUp = 1;

  valueX = analogRead(pinX);
  valueY = analogRead(pinY);

  if (keyUp && ((valueX <= 10) || (valueX >= 1010) || (valueY <= 10) || (valueY >= 1010)))
  {
    delay(10); 
    keyUp = 0;
    if (valueX <= 10)
    {
      if (snake_dir != UP)   snake_dir = DOWN;
    }
    else if (valueX >= 1010)
    {
      if (snake_dir != DOWN) snake_dir = UP;
    }
    else if (valueY <= 10)
    {
      if (snake_dir != RIGHT) snake_dir = LEFT;
    }
    else if (valueY >= 1010)
    {
      if (snake_dir != LEFT)  snake_dir = RIGHT;
    }
  } 
  else if ((valueX > 10) && (valueX < 1010) && (valueY > 10) && (valueY < 1010)) {
    keyUp = 1;
  }
}

// Button Scanner for Start / Restart
void checkButton(void)
{
  if (digitalRead(pinSW) == LOW)
  {
    delay(200); // Debounce delay to prevent double-clicks
    if (!game_started) 
    {
      reset_game();
      game_started = true;
    } 
    else if (game_over) 
    {
      reset_game();
    }
  }
}

void draw_snake(int x, int y)
{
  oled.drawBitmap(x, y, block, 4, 4, 1);
}

void show_score(int x, int y, int data)
{
  oled.setCursor(x, y);
  oled.println(data);
}

// Main Menu Drawing
void draw_main_menu()
{
  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setCursor(32, 10);
  oled.println("SNAKE");
  
  oled.setTextSize(1);
  oled.setCursor(20, 42);
  oled.println("Press Joystick Button");
  oled.setCursor(44, 54);
  oled.println("To Start");
  oled.display();
}

// Gameplay Screen Drawing
void screen(void)
{
  oled.clearDisplay();
  oled.setTextSize(1); 
  oled.drawRect(0, 1, 102, 62, 1);
  oled.drawRect(0, 0, 102, 64, 1);
  oled.setCursor(104, 12);
  oled.println("LEVE");
  oled.setCursor(104, 40);
  oled.println("SCOR");

  show_score(110, 25, level);
  show_score(110, 53, score);

  for (i = 0; i < snake_len; i++)
  {
    if(x[i] > 0 && y[i] > 0) { 
      draw_snake(x[i], y[i]);
    }
  }

  draw_snake(food_x, food_y);
  oled.display();
}

void draw_food(void)
{
  int food_out = 0;

  if (food_eaten)
  {
    while (food_out == 0)
    {
      food_out = 1;
      food_x = (uint8_t)(random(4, 96) / 4) * 4;
      food_y = (uint8_t)(random(4, 56) / 4) * 4;

      for (int i = snake_len - 1; i > 0; i--) 
      {
        if (food_x == x[i] && food_y == y[i])
        {
          food_out = 0;
        }
      }
    }
    food_eaten = false;
  }
}

void snake_move(void)
{
  switch (snake_dir) {
    case RIGHT: snake_head_x += 4; break;
    case UP:    snake_head_y -= 4; break;
    case LEFT:  snake_head_x -= 4; break;
    case DOWN:  snake_head_y += 4; break;
  }

  if ((snake_head_x == food_x) && (snake_head_y == food_y))
  {
    food_eaten = true; 
    snake_len++;
    score++;
    level = score / 5 + 1;
    
    if (snake_speed > 40) {
      snake_speed -= 10;
    }
  }

  for (i = snake_len - 1; i > 0; i--)
  {
    x[i] = x[i - 1];
    y[i] = y[i - 1];
  }
  x[0] = snake_head_x;
  y[0] = snake_head_y;

  check_snake_die();
}

void draw_game_over()
{
  oled.clearDisplay();
  oled.setTextSize(2); 
  oled.setCursor(10, 10);
  oled.println("GAME OVER");
  
  oled.setTextSize(1); 
  oled.setCursor(20, 32);
  oled.println("Press Click Button");
  oled.setCursor(35, 42);
  oled.println("To Restart");
  
  oled.setCursor(10, 55);
  oled.print("LVL:"); oled.print(level);
  oled.setCursor(70, 55);
  oled.print("SCR:"); oled.print(score);

  oled.display();
}

void check_snake_die(void)
{
  // Border Collision Check
  if (snake_head_x < 1 || snake_head_x > 98 || snake_head_y < 1 || snake_head_y > 58)
  {
    game_over = true;
  }

  // Self Collision Check
  if (snake_len > 3)
  {
    for (int i = 1; i < snake_len; i++)
    {
      if (snake_head_x == x[i] && snake_head_y == y[i])
      {
        game_over = true;
      }
    }
  }
}

void setup()
{
  pinMode(pinSW, INPUT_PULLUP);

  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  oled.setTextColor(WHITE);
  randomSeed(analogRead(3));
  
  reset_game();
}

void loop()
{
  checkButton(); 

  if (!game_started) 
  {
    draw_main_menu();
  } 
  else if (game_over) 
  {
    draw_game_over();
  } 
  else 
  {
    keyScan();
    snake_move();
    draw_food();
    screen();
  }
  
  delay(snake_speed);
}
