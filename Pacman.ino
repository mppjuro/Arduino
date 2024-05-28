#include <Usb.h>
#include <usbhub.h>
#include <hidboot.h>
#include <Adafruit_GFX.h>
#include <RGBmatrixPanel.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define buzzerPin 8 // Dodaję definicję zmiennej buzzerPin

LiquidCrystal_I2C lcd(0x27, 20, 4);
#define CLK 11
#define LAT A3
#define OE 9
#define A A0
#define B A1
#define C A2
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);

USB Usb;
USBHub Hub( & Usb);
HIDBoot < USB_HID_PROTOCOL_KEYBOARD > HidKeyboard( & Usb);

volatile bool gora = false, dol = false, lewo = false, prawo = false;
const int BOARD_WIDTH = 32,
          BOARD_HEIGHT = 16;
bool board[BOARD_WIDTH][BOARD_HEIGHT] = {0};
bool inGame = false;
bool inGame2 = false;
bool Pacmanstart = false;
bool rysuj = false;

class KbdRptParser : public KeyboardReportParser {
  void OnKeyDown(uint8_t mod, uint8_t key) override;
  void OnKeyUp(uint8_t mod, uint8_t key) override;
  void OnControlKeysChanged(uint8_t before, uint8_t after) override;
};

int PacmanLen = 1;
KbdRptParser Prs;
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

// Połozenie Enemy Pacmana
struct Enemy {
  int x;
  int y;
};

enum Direction { UP, DOWN, LEFT, RIGHT, NONE };
Direction currentDirection = NONE;

int foodX = 10, foodY = 10;  // Początkowa pozycja jedzenia
int liczba_przeciwnikow = 3;
// zmienne dla pacmana
PacmanSegment pacman[1]; //inicjalizacja połozenia pacmana
Enemy enemies[3]; // inicjalizacja połozenia  3 przeciwników

// Inicjalizacja gry Pacman

void initializePacmanGame() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   Pacman ver. 1.0   ");
  lcd.setCursor(0, 1);
  lcd.print("Score:  0           ");
  lcd.setCursor(0, 2);
  lcd.print("                    ");
  lcd.setCursor(0, 3);
  lcd.print("====  Pacman 1.0 ====");
  PacmanLen = 1;  // Początkowa długość Pacmana
  for (int i = 0; i < PacmanLen; i++) {
    pacman[i].x = 16 - i;
    pacman[i].y = 8;
  }
  spawnFood();
  spawnEnemies();
}

void spawnFood() {
  bool onPacman;
  do {
    onPacman = false;
    foodX = random(1, BOARD_WIDTH - 1);
    foodY = random(1, BOARD_HEIGHT - 1);
    for (int i = 0; i < PacmanLen; i++) {
      if (pacman[i].x == foodX && pacman[i].y == foodY) {
        onPacman = true;
        break;
      }
    }
  } while (onPacman);
}

void spawnEnemies(){
  enemies[0].x = random(1, BOARD_WIDTH - 1);
  enemies[0].y = random(1, BOARD_HEIGHT - 1);
  enemies[1].x = random(1, BOARD_WIDTH - 1);
  enemies[1].y = random(1, BOARD_HEIGHT - 1);
  enemies[2].x = random(1, BOARD_WIDTH - 1);
  enemies[2].y = random(1, BOARD_HEIGHT - 1);
}

void initializeEnemies() {
  spawnEnemies();
  for (int i = 0; i < liczba_przeciwnikow; i++)
    matrix.drawPixel(enemies[i].x, enemies[i].y, matrix.Color333(3, 0, 0));
}

void updateEnemies() {
  // Iterujemy przez wszystkich przeciwników
  for (int i = 0; i < liczba_przeciwnikow; i++) {
    // Generujemy losowy kierunek ruchu dla każdego przeciwnika
    int losowy_kierunek = random(4); // Losujemy liczby od 0 do 3

    // Aktualizujemy pozycję przeciwnika zgodnie z wylosowanym kierunkiem
    switch (losowy_kierunek) {
      case 0:
        // Ruch w górę
        if (enemies[i].y > 0) {
          enemies[i].y--;
        }
        break;
      case 1:
        // Ruch w dół
        if (enemies[i].y < BOARD_HEIGHT - 1) {
          enemies[i].y++;
        }
        break;
      case 2:
        // Ruch w lewo
        if (enemies[i].x > 0) {
          enemies[i].x--;
        }
        break;
      case 3:
        // Ruch w prawo
        if (enemies[i].x < BOARD_WIDTH - 1) {
          enemies[i].x++;
        }
        break;
    }
  }
    for (int i = 0; i < liczba_przeciwnikow; i++)
      matrix.drawPixel(enemies[i].x, enemies[i].y, matrix.Color333(3, 0, 0));
}

void checkEnemyCollision() {

  // Iterujemy przez wszystkich przeciwników
  for (int i = 0; i < liczba_przeciwnikow; i++) {
    // Sprawdzamy, czy przeciwnik znajduje się na tej samej pozycji co Pacman
    if (enemies[i].x == pacman[0].x && enemies[i].y == pacman[0].y) {
      Pacmanstart = false;
      break;
    }
  }
}

// Rysowanie stanu gry Pacman
void drawPacmanGame() {
  matrix.fillScreen(0); // Czyszczenie ekranu
  // Rysowanie Pacmana
  matrix.drawPixel(pacman[0].x, pacman[0].y, matrix.Color333(0, 3, 0));
  // Rysowanie jedzenia
  matrix.drawPixel(foodX, foodY, matrix.Color333(0, 0, 3));
}

// Aktualizacja stanu gry Pacman
void updatePacmanGame() {
  // Sprawdzenie kierunku i aktualizacja pozycji pacmana
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

  // Sprawdzenie czy Pacman zjadł jedzenie
  if (pacman[0].x == foodX && pacman[0].y == foodY) {
    //mozna zjesc enemy przez 10 sekund dodac trzeba
    spawnFood(); // Nowe jedzenie
  }
}

void setup() {
  pinMode(buzzerPin, OUTPUT);
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(" KIERUNEK = PLANSZA ");
  lcd.setCursor(0, 1);
  lcd.print(" LEWO - PACMAN  ");
  lcd.setCursor(0, 2);
  lcd.print(" PRAWO - ARKANOID 2 ");
  lcd.setCursor(0, 3);
  lcd.print(" DOL - SNAKE ver. 1 ");
  Serial.println("LCD STARTED");
  matrix.begin();
  if (Usb.Init() == -1) {
    Serial.println("OSC did not start.");
  }
  HidKeyboard.SetReportParser(0, &Prs);
  Serial.println("Start");
}

void loop() {
  Usb.Task();
  if (Pacmanstart) {
    noTone(buzzerPin);
    updatePacmanGame();
    drawPacmanGame();
    updateEnemies();
    checkEnemyCollision();
    initializeEnemies();
  }
  else{
    if (lewo) {
    Pacmanstart = true;
    matrix.fillScreen(0);
    initializePacmanGame();
    }
  }
  delay(100);
}
