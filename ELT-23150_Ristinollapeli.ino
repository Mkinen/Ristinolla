// Arttu Mäkinen
// arttu.makinen@student.tut.fi

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10
#define TFT_MOSI 11
#define TFT_CLK 13
#define TFT_RST 8
#define TFT_MISO 12
#define TFT_SDCS 4

#define changeButton 4
#define confirmButton 5
#define speakerOut 6

#define  c     3830    // 261 Hz 
#define  d     3400    // 294 Hz 
#define  e     3038    // 329 Hz 
#define  f     2864    // 349 Hz 
#define  g     2550    // 392 Hz 
#define  a     2272    // 440 Hz 
#define  b     2028    // 493 Hz 
#define  C     1912    // 523 Hz 
#define  R     0

int melody[] = {  c, R,  e, R,  g}; //  R,  C,  c,  g, a, C };
int beats[]  = { 32, 2, 32, 2, 64}; // 32, 16, 16, 16, 8, 8 }; 

int MAX_COUNT = sizeof(melody) / 2; // Melody length, for looping.

// Set overall tempo
long tempo = 10000;
// Set length of pause between notes
int pause = 1000;
// Loop variable to increase Rest length
int rest_count = 100;

// Initialize core variables
int tone_ = 0;
int beat = 0;
long duration  = 0;

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

bool pick = false;
uint16_t charSize = 12;
uint16_t y0 = 0;
uint16_t x0 = 0;
uint16_t height = tft.height();
uint16_t width = 10;
uint16_t radius = 4;
char currChar = 'X';
int currSQ = 0;
int currColor = ILI9341_GREEN;
int draw = 0;
bool gameEnds = false;

const int row1 = 5;
const int row2 = (width + tft.width()/3);
const int row3 = (width + 2*tft.width()/3);
const int column1 = 5; 
const int column2 = (width + tft.height()/3);
const int column3 = (width + 2*tft.height()/3);

const int squares[9][3] = {{row1, column1, 0},
                           {row2, column1, 1},
                           {row3, column1, 2},
                           {row1, column2, 3},
                           {row2, column2, 4},
                           {row3, column2, 5},
                           {row1, column3, 6},
                           {row2, column3, 7},
                           {row3, column3, 8}};

bool checkList[9] = {false, false, false, false, false, false, false, false, false};

const int winningLines[8][3] = {{ 0, 1, 2 }, { 3, 4, 5 }, { 6, 7, 8 }, { 0, 3, 6 }, { 1, 4, 7 }, { 2, 5, 8 }, { 0, 4, 8 }, { 2, 4, 6 }};

int placeX[5]; int placeXcount;
int placeO[5]; int placeOcount;


void setup() {
  Serial.begin(9600);
  Serial.println("ILI9341 Test!"); 
  tft.begin();
  
  pinMode(changeButton,INPUT);
  pinMode(confirmButton,INPUT);
  pinMode(speakerOut, OUTPUT);
}

void loop(void) {
  format();
  game();
}

void game(){
  while(gameEnds == false){
    if(currSQ > 9){
      currSQ = 0;
    }
    if(checkList[currSQ] == false){
      int row = squares[currSQ][0];
      int column = squares[currSQ][1];
          
      tft.drawChar(row,column, currChar, ILI9341_BLUE, ILI9341_BLUE, charSize);
      delay(50);
      while(pick == false){
        if(digitalRead(confirmButton) == HIGH){
          tone_ = f;
          beat = 6;
          duration = beat * tempo; // Set up timing    
          playTone(); 
          
          tft.drawChar(row,column, currChar, currColor, currColor, charSize);
          checkList[currSQ] = true;
          if(currChar == 'X'){
            placeX[placeXcount] = currSQ;
            ++placeXcount;
            if(placeXcount > 2){
              winCheck();
            }
            currChar = 'O';
            currColor = ILI9341_RED;
          } else {
            placeO[placeOcount] = currSQ;
            ++placeOcount;
            if(placeOcount > 2){
              winCheck();
            }
            currChar = 'X';
            currColor = ILI9341_GREEN;
          }
          ++draw;
          pick = true;
        }
        if(draw == 9 && gameEnds == false){
          drawScreen();
          gameEnds = true;
        }
        else if(digitalRead(changeButton) == HIGH){
          tone_ = c;
          beat = 4;
          duration = beat * tempo; // Set up timing    
          playTone(); 
          
          tft.drawChar(row,column, currChar, ILI9341_BLACK, ILI9341_BLACK, charSize);
          pick = true;
        }
      }
      pick = false;
    }
    ++currSQ;
  }
}

void drawScreen(){
  tft.setCursor(0, tft.height()/2.5);
  tft.setTextColor(ILI9341_BLUE);
  tft.setTextSize(10);
  tft.println("DRAW");
  delay(50);
  while(digitalRead(confirmButton) == LOW && digitalRead(changeButton) == LOW);
}

void victoryScreen(){
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(currColor);
  tft.setTextSize(13);
  tft.setCursor(5+tft.width()/3, tft.height()/5);
  tft.println(currChar);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(6, tft.height()/1.75);
  tft.println("WON");
  delay(50);
  while(digitalRead(confirmButton) == LOW && digitalRead(changeButton) == LOW);
}

void winCheck() {
  int winCheck = 0;
  for(int j = 0; j < 8; j++){
    for(int k = 0; k < 3; k++){
      if(currChar == 'X'){
        for(int p = 0; p < placeXcount; p++){
          if(winningLines[j][k] == placeX[p]){
            ++winCheck;
          }
        }
      } else {
        for(int p = 0; p < placeOcount; p++){
            if(winningLines[j][k] == placeO[p]){
              ++winCheck;
            }
        }
      }
      if(winCheck > 2){
        int places[3] = {winningLines[j][0], winningLines[j][1], winningLines[j][2]};
        int rows[3];
        int columns[3];
        for(int j = 0; j < 3; j++){
          for(int i = 0; i < 9; i++){
            if(squares[i][2] == places[j]){
              rows[j] = squares[i][0];
              columns[j] = squares[i][1];
              break;
            }
          }
        }
        victoryMusic();
        bool endLoop = false;
        while(digitalRead(confirmButton) == HIGH || digitalRead(changeButton) == HIGH){}
        while(endLoop == false){
          tft.drawChar(rows[0],columns[0], currChar, ILI9341_BLACK, ILI9341_BLACK, charSize);
          tft.drawChar(rows[0],columns[0], currChar, currColor, currColor, charSize);
          if(digitalRead(confirmButton) == HIGH || digitalRead(changeButton) == HIGH){
            endLoop = true;
          } delay(250);
          tft.drawChar(rows[1],columns[1], currChar, ILI9341_BLACK, ILI9341_BLACK, charSize);
          tft.drawChar(rows[1],columns[1], currChar, currColor, currColor, charSize);
          if(digitalRead(confirmButton) == HIGH || digitalRead(changeButton) == HIGH){
            endLoop = true;
          } delay(250);
          tft.drawChar(rows[2],columns[2], currChar, ILI9341_BLACK, ILI9341_BLACK, charSize);
          tft.drawChar(rows[2],columns[2], currChar, currColor, currColor, charSize);
          if(digitalRead(confirmButton) == HIGH || digitalRead(changeButton) == HIGH){
            endLoop = true;
          } delay(250);
        }
        
        victoryScreen();

        gameEnds = true;
        break;
      }
    }
    winCheck = 0;
  }
}

void format() {
  gameEnds = false;
  for(int i = 0; i < 9; i++){
    checkList[i] = false;
  }  
  for(int i = 0; i < 5; i++){
    placeX[i] = 9;
  }
  for(int i = 0; i < 5; i++){
    placeO[i] = 9;
  }
  
  tempo = 10000;
  placeXcount = 0;
  placeOcount = 0;

  currSQ = 0;
  draw = 0;
  
  tft.fillScreen(ILI9341_BLACK);

  tft.fillRoundRect(tft.width()/3 - width/2, y0, width,  height,  radius, ILI9341_WHITE);
  tft.fillRoundRect(2*tft.width()/3 - width/2, y0, width,  height,  radius, ILI9341_WHITE);

  tft.fillRoundRect( x0, tft.height()/3 - width/2, height,  width,  radius, ILI9341_WHITE);
  tft.fillRoundRect( x0, 2*tft.height()/3 - width/2, height,  width,  radius, ILI9341_WHITE);
}

void victoryMusic() {
  for (int i=0; i<MAX_COUNT; i++) {
    tone_ = melody[i];
    beat = beats[i];

    duration = beat * tempo; // Set up timing

    playTone(); 
    // A pause between notes...
    delayMicroseconds(pause);
  }
}

void playTone() {
  long elapsed_time = 0;
  if (tone_ > 0) { // if this isn't a Rest beat, while the tone has 
    //  played less long than 'duration', pulse speaker HIGH and LOW
    while (elapsed_time < duration) {

      digitalWrite(speakerOut,HIGH);
      delayMicroseconds(tone_ / 2);

      // DOWN
      digitalWrite(speakerOut, LOW);
      delayMicroseconds(tone_ / 2);

      // Keep track of how long we pulsed
      elapsed_time += (tone_);
    } 
  }
  else { // Rest beat; loop times delay
    for (int j = 0; j < rest_count; j++) { // See NOTE on rest_count
      delayMicroseconds(duration);  
    }                                
  }                                 
}



