#include <Usb.h>
#include <usbhub.h>
#include <hidboot.h>
#include <Adafruit_GFX.h>
#include <RGBmatrixPanel.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);
#define CLK 11
#define LAT A3
#define OE 9
#define A A0
#define B A1
#define C A2
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);

volatile bool gora = false, dol = false, lewo = false, prawo = false;
const int BOARD_WIDTH = 32,
    BOARD_HEIGHT = 16;
bool board[BOARD_WIDTH][BOARD_HEIGHT] = {
    0
};
bool inGame = false;
bool inGame2 = false;
bool Pacmanstart = false;
bool rysuj = false;

int PacmanLen = 1;
KbdRptParser Prs;

class KbdRptParser: public KeyboardReportParser {
    void OnKeyDown(uint8_t mod, uint8_t key) override;
    void OnKeyUp(uint8_t mod, uint8_t key) override;
    void OnControlKeysChanged(uint8_t before, uint8_t after) override;
};
void KbdRptParser::OnControlKeysChanged(uint8_t before, uint8_t after) {}

void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t key) {
    Serial.print("Down: Mod: ");
    Serial.print(mod, HEX);
    Serial.print(" Key: ");
    Serial.println(key, HEX);
    switch (key) {
    case 0x52:
        gora = true;
        break;
    case 0x51:
        dol = true;
        break;
    case 0x50:
        lewo = true;
        break;
    case 0x4F:
        prawo = true;
        break;
    default:
        break;
    }
}

  void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t key) {
    Serial.print("Up: Mod: ");
    Serial.print(mod, HEX);
    Serial.print(" Key: ");
    Serial.println(key, HEX);
    switch (key) {
    case 0x52:
        gora = false;
        break;
    case 0x51:
        dol = false;
        break;
    case 0x50:
        lewo = false;
        break;
    case 0x4F:
        prawo = false;
        break;
    default:
        break;
    }
}

// Połozenie Pacmana
struct PacmanSegment {
    int x;
    int y;
};

enum Direction { UP, DOWN, LEFT, RIGHT, NONE };
Direction currentDirection = NONE;

int foodX = 10, foodY = 10;  // Początkowa pozycja jedzenia

//zmienne dla pacmana
PacmanSegment pacman[1];

//Inicjalizacja gry Pacman

void initializePacmanGame(){
      lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("   Pacman ver. 1.0   ");
    lcd.setCursor(0, 1);
    lcd.print("Score:  0           ");
    lcd.setCursor(0, 2);
    lcd.print("                    ");
    lcd.setCursor(0, 3);
    lcd.print("====  Pacman 1.0 ====");
    PacmanLength = 1;  // Początkowa długość Pacmana
    for(int i = 0; i < PacmanLength; i++) {
        pacman[i].x = 16 - i;
        pacman[i].y = 8;
    }
    spawnFood();
}

void spawnFood() {
    bool onSnake;
    do {
        onSnake = false;
        foodX = random(1, BOARD_WIDTH-1);
        foodY = random(1, BOARD_HEIGHT-1);
        for (int i = 0; i < snakeLength; i++) {
            if (snake[i].x == foodX && snake[i].y == foodY) {
                onSnake = true;
                break;
            }
        }
    } while(onSnake);
}

// Rysowanie stanu gry Pacman
void drawPacmanGame() {
    matrix.fillScreen(0); // Czyszczenie ekranu
    // Rysowanie Pacmana
        matrix.drawPixel(pacman[i].x, pacman[i].y, matrix.Color333(0, 3, 0));
    // Rysowanie jedzenia
    matrix.drawPixel(foodX, foodY, matrix.Color333(3, 0, 0));
}

// Aktualizacja stanu gry Pacman
void updatePacmanGame() {
    // Sprawdzenie kierunku i aktualizacja pozycji głowy węża
    if (prawo && (currentDirection != LEFT)) {
      currentDirection = RIGHT;
      
    } else if (lewo && (currentDirection != RIGHT)) {
        currentDirection = LEFT;
    } else if (gora && (currentDirection != DOWN)) {
        currentDirection = UP;
    } else if (dol && (currentDirection != UP)) {
        currentDirection = DOWN;
    }
    if (currentDirection == RIGHT) {
      pacman[0].y--;
    } else if (currentDirection == LEFT) {
      pacman[0].y++;
    } else if (currentDirection == UP) {
      pacman[0].x--;
    } else if (currentDirection == DOWN) {
      pacman[0].x++;
    }
}


    // Sprawdzenie czy Pacman zjadł jedzenie
    if (pacman[0].x == foodX && pacman[0].y == foodY) {
      //mozna zjesc enemy przez 10 sekund 
        spawnFood(); // Nowe jedzenie
    }
void setup() {
    pinMode(buzzerPin, OUTPUT);
    Serial.begin(9600);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print(" KIERUNEK = PLANSZA ");
    lcd.setCursor(0, 1);
    lcd.print(" LEWO - ARKANOID 1  ");
    lcd.setCursor(0, 2);
    lcd.print(" PRAWO - ARKANOID 2 ");
    lcd.setCursor(0, 3);
    lcd.print(" DOL - SNAKE ver. 1 ");
    Serial.println("LCD STARTED");
    matrix.begin();
    if (Usb.Init() == -1) {
        Serial.println("OSC did not start.");
    }
    HidKeyboard.SetReportParser(0, & Prs);
    initializePoints();
    Serial.println("Start");
}
void loop() {
  Usb.Task();
  if(Pacmanstart){
      noTone(buzzerPin);
      updatePacmanGame();
      drawPacmanGame();
    }
    else{ 
      
    }
            if (lewo) {
            Pacmanstart = true;
            matrix.fillScreen(0);
            arkanoidInit(0);
            arkanoidPlansza();
            arkanoidLinia(arkanoidx, arkanoidy);
        }
        delay(100);
    }
  }

}    
