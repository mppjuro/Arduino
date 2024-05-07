#include <Usb.h>
#include <usbhub.h>
#include <hidboot.h>
#include <Adafruit_GFX.h>
#include <RGBmatrixPanel.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

#define NOTE_C4 262
#define NOTE_D4 294
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_G4 392
#define NOTE_A4 440
#define NOTE_B4 466
#define NOTE_H4 494
#define NOTE_ONE 240
#define NOTE_TWO 550
#define NOTE_THREE 700
#define NOTE_FOUR 1250

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
bool board[BOARD_WIDTH][BOARD_HEIGHT] = {
    0
};
bool inGame = false;
bool inGame2 = false;
bool rysuj = false;
int arkanoidx = 5, arkanoidy = 0;
int ballx = 0;
int bally = 0;
int balldx = 0;
int balldy = 0;
int pkt = 0;
int snakePkt = 0;
const int size = 32;
bool T[32][32];
const int buzzerPin = 8;
bool status_C = 0;

int game_length = 10; // Długość gry
int game[10]; // Tablica przechowująca sekwencję nut w grze
int currentStep; // Aktualny krok w grze
bool correctInput; // Flaga wskazująca na poprawne wprowadzenie użytkownika
int gameSetup[4] = {
    NOTE_ONE,
    NOTE_TWO,
    NOTE_THREE,
    NOTE_FOUR
};
int playerInput[10];

class KbdRptParser: public KeyboardReportParser {
    void OnKeyDown(uint8_t mod, uint8_t key) override;
    void OnKeyUp(uint8_t mod, uint8_t key) override;
    void OnControlKeysChanged(uint8_t before, uint8_t after) override;
};

KbdRptParser Prs;

// Struktura reprezentująca węża
struct SnakeSegment {
    int x;
    int y;
};

enum Direction { UP, DOWN, LEFT, RIGHT, NONE };
Direction currentDirection = NONE;

// Maksymalna liczba segmentów węża
const int maxSnakeLength = 512;
SnakeSegment snake[maxSnakeLength];
int snakeLength = 4;
int foodX = 10, foodY = 10;  // Początkowa pozycja jedzenia

// Inicjalizacja gry Snake
void initializeSnakeGame() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("   SNAKE ver. 1.0   ");
    lcd.setCursor(0, 1);
    lcd.print("Score:  0           ");
    lcd.setCursor(0, 2);
    lcd.print("                    ");
    lcd.setCursor(0, 3);
    lcd.print("====  SNAKE 1.0 ====");
    snakeLength = 4;  // Początkowa długość węża
    for(int i = 0; i < snakeLength; i++) {
        snake[i].x = 16 - i;
        snake[i].y = 8;
    }
    spawnFood();
}

// Funkcja spawnująca jedzenie w losowym miejscu
void spawnFood() {
    bool onSnake;
    do {
        onSnake = false;
        foodX = random(0, BOARD_WIDTH-1);
        foodY = random(0, BOARD_HEIGHT-1);
        for (int i = 0; i < snakeLength; i++) {
            if (snake[i].x == foodX && snake[i].y == foodY) {
                onSnake = true;
                break;
            }
        }
    } while(onSnake);
}

// Rysowanie stanu gry Snake
void drawSnakeGame() {
    matrix.fillScreen(0); // Czyszczenie ekranu
    // Rysowanie węża
    for(int i = 0; i < snakeLength; i++) {
        matrix.drawPixel(snake[i].x, snake[i].y, matrix.Color333(0, 2, 0));
    }
    // Rysowanie jedzenia
    matrix.drawPixel(foodX, foodY, matrix.Color333(2, 0, 0));
}

// Aktualizacja stanu gry Snake
void updateSnakeGame() {
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
      snake[0].y--;
    } else if (currentDirection == LEFT) {
      snake[0].y++;
    } else if (currentDirection == UP) {
      snake[0].x--;
    } else if (currentDirection == DOWN) {
      snake[0].x++;
    }

    // Sprawdzenie kolizji z krawędziami
    if (snake[0].x < 0 || snake[0].x >= BOARD_WIDTH || snake[0].y < 0 || snake[0].y >= BOARD_HEIGHT) {
        snakePkt = 0;
        inGame = false;
        currentDirection = NONE;
        playMelodyEnd();
        lcd.setCursor(0, 0);
    lcd.print(" KIERUNEK = PLANSZA ");
    lcd.setCursor(0, 1);
    lcd.print(" LEWO - ARKANOID 1  ");
    lcd.setCursor(0, 2);
    lcd.print(" PRAWO - ARKANOID 2 ");
    lcd.setCursor(0, 3);
    lcd.print(" DOL - SNAKE ver. 1 ");
    }

    // Sprawdzenie kolizji z samym sobą
    for (int i = 1; i < snakeLength; i++) {
        if (snake[i].x == snake[0].x && snake[i].y == snake[0].y && currentDirection != NONE) {
            inGame = false;
            currentDirection = NONE;
            playMelodyEnd();
            lcd.setCursor(0, 0);
    lcd.print(" KIERUNEK = PLANSZA ");
    lcd.setCursor(0, 1);
    lcd.print(" LEWO - ARKANOID 1  ");
    lcd.setCursor(0, 2);
    lcd.print(" PRAWO - ARKANOID 2 ");
    lcd.setCursor(0, 3);
    lcd.print(" DOL - SNAKE ver. 1 ");
    snakePkt = 0;
        }
    }

// Sprawdzenie czy wąż zjadł jedzenie
if (snake[0].x == foodX && snake[0].y == foodY) {
    if ((snakeLength + 2) <= maxSnakeLength) { // Sprawdzenie, czy jest miejsce na dodanie dwóch segmentów
        // Zapamiętaj pozycję ostatniego segmentu
        int last_segment_x = snake[snakeLength-1].x;
        int last_segment_y = snake[snakeLength-1].y;

        // Wydłużenie węża o jeden segment
        snakeLength++;
        snake[snakeLength-1].x = last_segment_x;
        snake[snakeLength-1].y = last_segment_y;

        // Wydłużenie węża o drugi segment
        if (snakeLength < maxSnakeLength) {
            snakeLength++;
            snake[snakeLength-1].x = last_segment_x; // Dodaj kolejny segment w tej samej pozycji
            snake[snakeLength-1].y = last_segment_y;
        }

        // Aktualizacja wyniku
        snakePkt++;
        lcd.setCursor(0, 1);
        lcd.print("Score:              ");
        lcd.setCursor(6, 1);
        lcd.print(String(snakePkt));
    }

    // Generowanie nowego jedzenia
    spawnFood();
}

// Przesunięcie segmentów węża
for (int i = snakeLength - 1; i > 0; i--) {
    snake[i] = snake[i - 1];
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
    if (inGame2) {
        noTone(buzzerPin);
        if (gora && dol && lewo && prawo) {
            pkt = 0;
            arkanoidGameOver();
            delay(1500);
        } else {
            if (gora && ((ballx + arkanoidy) >= 30)) {
                matrix.drawPixel(ballx, bally, matrix.Color333(0, 0, 0));
                ballx--;
                matrix.drawPixel(ballx, bally, matrix.Color333(0, 2, 2));
            }
            if (lewo && gora) {
                if (arkanoidx != 10) {
                    arkanoidx++;
                }
                if (arkanoidy < 6) {
                    arkanoidy++;
                }
                arkanoidLinia(arkanoidx, arkanoidy);
            } else if (lewo & dol) {
                if (arkanoidx != 10) {
                    arkanoidx++;
                }
                if (arkanoidy != 0) {
                    arkanoidy--;
                }
                arkanoidLinia(arkanoidx, arkanoidy);
            } else if (prawo && gora) {
                if (arkanoidx != 0) {
                    arkanoidx--;
                }
                if (arkanoidy < 6) {
                    arkanoidy++;
                }
                arkanoidLinia(arkanoidx, arkanoidy);
            } else if (prawo & dol) {
                if (arkanoidx != 0) {
                    arkanoidx--;
                }
                if (arkanoidy != 0) {
                    arkanoidy--;
                }
                arkanoidLinia(arkanoidx, arkanoidy);
            } else if (lewo) {
                if (arkanoidx != 10) {
                    arkanoidx++;
                    arkanoidLinia(arkanoidx, arkanoidy);
                }
            } else if (prawo) {
                if (arkanoidx != 0) {
                    arkanoidx--;
                    arkanoidLinia(arkanoidx, arkanoidy);
                }
            } else if (gora) {
                if (arkanoidy < 6) {
                    arkanoidy++;
                    arkanoidLinia(arkanoidx, arkanoidy);
                }
            } else if (dol) {
                if (arkanoidy != 0) {
                    arkanoidy--;
                    arkanoidLinia(arkanoidx, arkanoidy);
                }
            }
            if ((bally + balldy) < 0 || (bally + balldy) > 15) {
                balldy *= -1;
            }
            if ((ballx + balldx) < 0 || ((ballx + balldx + arkanoidy) == 31 && ((bally + balldy) >= arkanoidx) && ((bally + balldy) <= (arkanoidx + 5)))) {
                balldx *= -1;
                if (lewo) {
                    balldy = 1;
                } else if (prawo) {
                    balldy = -1;
                }
            }
            if (((ballx + balldx) == (31 - arkanoidy)) && gora) {
                ballx--;
                balldx *= -1;
                if (lewo) {
                    balldy = 1;
                } else if (prawo) {
                    balldy = -1;
                }
            }
            if (T[ballx + balldx][bally + balldy] && (ballx + balldx) >= 0 && (ballx + balldx) <= 31 && (bally + balldy) >= 0 && (bally + balldy) <= 15) {
                T[ballx + balldx][bally + balldy] = false;
                matrix.drawPixel(ballx + balldx, bally + balldy, matrix.Color333(0, 0, 0));
                balldx *= -1;
                balldy *= -1;
                pkt++;
                tone(buzzerPin, NOTE_THREE, 150);
            }
            arkanoidPlansza();
            matrix.drawPixel(ballx, bally, matrix.Color333(0, 0, 0));
            ballx = ballx + balldx;
            bally = bally + balldy;
            for (int i = 16; i < (31 - arkanoidy - 1); i++) {
              for (int j = 0; j < 16; j++) {
                matrix.drawPixel(i, j, matrix.Color333(0, 0, 0));
              }
            }
            if (ballx >= 31) {
                pkt = 0;
                playMelodyEnd();
                arkanoidGameOver();
            } else {
                matrix.drawPixel(ballx, bally, matrix.Color333(0, 2, 2));
                lcd.setCursor(6, 1);
                lcd.print((String(pkt)));
            }
        }
    } else if (inGame) {
      updateSnakeGame();
      drawSnakeGame();
    } else {
        updateAndDrawPoints();
        if (lewo) {
            inGame2 = true;
            matrix.fillScreen(0);
            arkanoidInit(0);
            arkanoidPlansza();
            arkanoidLinia(arkanoidx, arkanoidy);
        }
        if (prawo) {
            inGame2 = true;
            matrix.fillScreen(0);
            arkanoidInit(1);
            arkanoidPlansza();
            arkanoidLinia(arkanoidx, arkanoidy);
        }
        if (dol) {
            inGame = true;
            initializeSnakeGame();
        }
    }
    delay(100);
}


void displaySequence() {
  delay(2000);
  for (int i = 0; i <= 4; ++i) {
    int note = gameSetup[i];
    tone(buzzerPin, note, 600);
    delay(800);
    noTone(buzzerPin);
    delay(400);
  }
  delay(2000);
}

void waitForInput() {
  for(int i = 0; i < 10; ++i) {
    playerInput[i] == 0;
  }
  int i = 0;
        gora = false;
        dol = false;
        lewo = false;
        prawo = false;
  while(i < currentStep) {
    if (gora) {
      tone(buzzerPin, gameSetup[0], 2000);
      playerInput[i] = gameSetup[0];
      i++;
    }
    if (lewo) {
      tone(buzzerPin, gameSetup[1], 2000);
      playerInput[i] = gameSetup[1];
      i++;
    }
    if (dol) {
      tone(buzzerPin, gameSetup[2], 2000);
      playerInput[i] = gameSetup[2];
      i++;
    }
    if (prawo) {
      tone(buzzerPin, gameSetup[3], 2000);
      playerInput[i] = gameSetup[3];
      i++;
    }
    String s1 = String(currentStep);
    lcd.setCursor(0,0);
    lcd.print(s1);
    String s2 = String(i);
    lcd.setCursor(0,1);
    lcd.print(s2);
  }
}

// Funkcja generująca nową sekwencję nut
void generateSequence() {
    int randomIndex = random(0, 4);
    game[currentStep] = gameSetup[randomIndex];
    currentStep++;
}

// Funkcja odtwarzająca melodię końcową
void playMelodyEnd() {
    int melody[] = {
        NOTE_C4,
        NOTE_H4,
        NOTE_A4,
        NOTE_G4,
        NOTE_F4,
        NOTE_E4,
        NOTE_B4
    };
    int noteDurations[] = {
        400,
        200,
        200,
        200,
        200,
        200,
        500
    };

    for (int i = 0; i < 7; i++) {
        tone(buzzerPin, melody[i], noteDurations[i]);
        delay(noteDurations[i] + 50);
        noTone(buzzerPin);
    }
    lcd.setCursor(0, 0);
    lcd.print(" KIERUNEK = PLANSZA ");
    lcd.setCursor(0, 1);
    lcd.print(" LEWO - ARKANOID 1  ");
    lcd.setCursor(0, 2);
    lcd.print(" PRAWO - ARKANOID 2 ");
    lcd.setCursor(0, 3);
    lcd.print(" DOL - SNAKE ver. 1 ");
}

void playMelodyWin() {
    int melody[] = {
        NOTE_C4,
        NOTE_D4,
        NOTE_E4,
        NOTE_F4,
        NOTE_G4,
        NOTE_A4,
        NOTE_B4,
        NOTE_H4,
        NOTE_H4,
        NOTE_B4,
        NOTE_A4,
        NOTE_G4,
        NOTE_F4,
        NOTE_E4,
        NOTE_D4,
        NOTE_C4,
    };

    int noteDurations[] = {
        100,
        450,
        350,
        250,
        150,
        100,
        300,
        200,
        400,
        100,
        400,
        300,
        200,
        100,
        100,
        300,
    };

    for (int i = 0; i < sizeof(melody) / sizeof(melody[0]); i++) {
        tone(buzzerPin, melody[i], noteDurations[i]);
        delay(noteDurations[i] + 50); // Dodatkowy czas, aby uniknąć zaniku dźwięku
        noTone(buzzerPin); // Wyłącz dźwięk
    }
}

void arkanoidInit(int plansza) {
    if (plansza == 0) {
        for (int i = 0; i < 32; i++)
            for (int j = 0; j < 32; j++)
                T[i][j] = false;
        for (int i = 0; i <= 15; i++)
            for (int j = 0; j <= 15; j++)
                T[i][j] = true;
        for (int i = 0; i <= 6; i++)
            for (int j = 0; j <= 6; j++)
                T[i][j] = false;
        for (int i = 9; i <= 15; i++)
            for (int j = 0; j <= 6; j++)
                T[i][j] = false;
        for (int i = 0; i <= 6; i++)
            for (int j = 9; j <= 15; j++)
                T[i][j] = false;
        for (int i = 9; i <= 15; i++)
            for (int j = 9; j <= 15; j++)
                T[i][j] = false;
    } else {
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                T[i][j] = false;
            }
        }

        for (int i = 1; i <= 14; i++) {
            for (int j = 1; j <= 14; j++) {
                T[i][1] = true;
                T[i][14] = true;
                T[1][j] = true;
                T[14][j] = true;
            }
        }
    }
    ballx = 15;
    bally = 4;
    balldx = random(2) * 2 - 1;
    balldy = random(2) * 2 - 1;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" ARKANOID ver. 1.0  ");
    lcd.setCursor(0, 1);
    lcd.print("Score:  0");
}

void arkanoidPlansza() {
    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 16; j++)
            if (T[i][j] == true)
                matrix.drawPixel(i, j, matrix.Color333(0, 2, 0));
}

void arkanoidLinia(int x, int y) {
    if (y > 0) {
        matrix.drawLine(31 - y + 1, 0, 31 - y + 1, 15, matrix.Color333(0, 0, 0));
    }
    matrix.drawLine(31 - y, 0, 31 - y, 15, matrix.Color333(0, 0, 0));
    matrix.drawLine(31 - y - 1, 0, 31 - y - 1, 15, matrix.Color333(0, 0, 0));
    matrix.drawLine(31 - y, x, 31 - y, x + 5, matrix.Color333(2, 0, 0));
}

void arkanoidGameOver() {
    matrix.fillScreen(0);
    inGame2 = false;
    lcd.setCursor(0, 0);
    lcd.print(" Nacisnij przycisk: ");
    lcd.setCursor(0, 1);
    lcd.print("GORA/PRAWO-PLANSZA 1");
    lcd.setCursor(0, 2);
    lcd.print("LEWO/DOL - PLANSZA 2");
    lcd.setCursor(0, 3);
    lcd.print("===   ARKANOID   ===");
}

const int numPoints = 7; // Liczba punktów
struct Point {
    int x, y; // Pozycja punktu
    int dx, dy; // Kierunek ruchu punktu
    uint16_t color; // Kolor punktu
}
points[numPoints];

void initializePoints() {
    randomSeed(analogRead(0)); // Inicjalizacja generatora liczb losowych
    for (int i = 0; i < numPoints; i++) {
        points[i].x = random(32);
        points[i].y = random(16);
        points[i].dx = random(2) * 2 - 1; // -1 lub 1
        points[i].dy = random(2) * 2 - 1; // -1 lub 1
    }
    points[0].color = matrix.Color333(1, 0, 0);
    points[1].color = matrix.Color333(1, 1, 0);
    points[2].color = matrix.Color333(1, 1, 1);
    points[3].color = matrix.Color333(1, 0, 1);
    points[4].color = matrix.Color333(0, 1, 0);
    points[5].color = matrix.Color333(0, 0, 1);
    points[6].color = matrix.Color333(0, 1, 1);
}

void updateAndDrawPoints() {
    matrix.fillScreen(0); // Wyczyszczenie ekranu
    for (int i = 0; i < numPoints; i++) {
        // Aktualizacja pozycji
        points[i].x += points[i].dx;
        points[i].y += points[i].dy;

        // Odbicie od krawędzi ekranu
        if (points[i].x <= 0 || points[i].x >= matrix.width() - 1) {
            points[i].dx *= -1;
        }
        if (points[i].y <= 0 || points[i].y >= matrix.height() - 1) {
            points[i].dy *= -1;
        }

        // Rysowanie punktu
        matrix.drawPixel(points[i].x, points[i].y, points[i].color);
    }
}

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

void KbdRptParser::OnControlKeysChanged(uint8_t before, uint8_t after) {}

void playMelodyStart() {
    int melody[] = {
        NOTE_C4,
        NOTE_D4,
        NOTE_E4,
        NOTE_F4,
        NOTE_G4,
        NOTE_A4,
        NOTE_B4,
        NOTE_H4,
        NOTE_H4,
        NOTE_B4,
        NOTE_A4,
        NOTE_G4,
        NOTE_F4,
        NOTE_E4,
        NOTE_D4,
        NOTE_C4,
    };

    int noteDurations[] = {
        100,
        450,
        350,
        250,
        150,
        100,
        300,
        200,
        400,
        100,
        400,
        300,
        200,
        100,
        100,
        300,
    };

    for (int i = 0; i < sizeof(melody) / sizeof(melody[0]); i++) {
        tone(buzzerPin, melody[i], noteDurations[i]);
        delay(noteDurations[i] + 50); // Dodatkowy czas, aby uniknąć zaniku dźwięku
        noTone(buzzerPin); // Wyłącz dźwięk
    }
}
