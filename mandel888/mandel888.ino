//Include

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>


//LCD pins
#define TFT_CS        A0
#define TFT_RST       10 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         2

//1.8" TFT with ST7735 use:
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

//Keys
#define UP_KEY  3
#define DOWN_KEY 5
#define LEFT_KEY 4
#define RIGHT_KEY 6
#define A_KEY 7
#define B_KEY 8
#define BAT_PIN A3
#define SPEAKER_OUTPUT_PIN  9 //buzzer to arduino pin 9

//Battery 
bool batFirtPowerOn = true;
int oldPercInt = 100;
auto batTextColor = ST77XX_WHITE;
char printout2[5];
long interval = 10000;           // interval at which to update
long previousMillis = 0;


//MS fractals vars

float WIDTH = 160;
float HEIGHT = 128;
//scaled to lie in the Mandelbrot X scale (-2.5, 1))
double minRe = -2, maxRe = 1;
//scaled to lie in the Mandelbrot X scale (-1, 1))
double minIm = -1, maxIm = 1;
//Num of iterations
int maxIter = 16;
double zoom = 1.0;

//move
double w = (maxRe - minRe);
double h = (maxIm - minIm);

int x = 160/2 - (WIDTH/4)/2;
int y = 128/2 - (HEIGHT/4)/2;
int Oldx = x;
int Oldy = y;

//display iter
char iterChar[5];

void setup() {
  Serial.begin(9600);

  //Init screen
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(1);

  // buzzer init
  pinMode(SPEAKER_OUTPUT_PIN, OUTPUT); // Set buzzer - pin 9 as an output
  
  //Buttons init
  analogReference(INTERNAL);
  pinMode(UP_KEY,INPUT_PULLUP);
  pinMode(DOWN_KEY,INPUT_PULLUP);
  pinMode(LEFT_KEY,INPUT_PULLUP);
  pinMode(RIGHT_KEY,INPUT_PULLUP);
  pinMode(A_KEY,INPUT_PULLUP);
  pinMode(B_KEY,INPUT_PULLUP);

  //Battery init
  pinMode(BAT_PIN,INPUT);
  
  drawBat();
  drawMendl();
  
}


auto zoom_x = [&](double z)
{
  
  
  double cr = minRe + (maxRe - minRe) * (x + (WIDTH/4)/2)  / WIDTH;
  double ci = minIm + (maxIm - minIm) * (y + (HEIGHT/4)/2) / HEIGHT;

  //zoom
  double tminr = cr - (maxRe - minRe) / 2 / z;
  maxRe = cr + (maxRe - minRe) / 2 / z;
  minRe = tminr;

  double tmini = ci - (maxIm - minIm) / 2 / z;
  maxIm = ci + (maxIm - minIm) / 2 / z;
  minIm = tmini;
};
  
void drawLoading(){

  tft.setTextSize(3);
  tft.setTextColor(batTextColor);
  tft.setCursor(20 , HEIGHT/2 - 10);
  tft.print("LOADING");
  
}
void drawRect(){

  //tft.fillScreen(ST77XX_BLACK);
  tft.drawRect(x,y,WIDTH/4,HEIGHT/4,ST77XX_WHITE);
  
  if(Oldx != x || Oldy != y){
    tft.drawRect(Oldx,Oldy,WIDTH/4,HEIGHT/4,ST77XX_BLACK);  
  }
  
  Oldx = x;
  Oldy = y;
  
  delay(200);

}

void loop() {
  
  drawBat();
  
  if(digitalRead(A_KEY) == LOW){
    maxIter += 16;
    zoom_x(3.5);
    drawMendl();
  }
  if(digitalRead(B_KEY) == LOW){
    
    maxIter = 16;
    minRe = -2; 
    maxRe = 1;
    minIm = -1; 
    maxIm = 1;
    
    drawMendl();
  }

  if(digitalRead(UP_KEY) == LOW){
    y-=HEIGHT/4;
    drawRect();
  }
  if(digitalRead(DOWN_KEY) == LOW){
    y+=HEIGHT/4;
    drawRect();
  }
  if(digitalRead(LEFT_KEY) == LOW){
    x-=WIDTH/4;
    drawRect();
  }
  if(digitalRead(RIGHT_KEY) == LOW){
    x+=WIDTH/4;
    drawRect();
  }
 
}

double randomDouble(double minf, double maxf)
{
  return minf + random(1UL << 31) * (maxf - minf) / (1UL << 31);  // use 1ULL<<63 for max double values)
}

void drawMendl(){
    //Mapping
   drawLoading();
   
   for (unsigned int x = 0; x < WIDTH; x++) for (unsigned y = 0; y < HEIGHT; y++){
     
    double cr = mapD(x, 0, WIDTH, minRe, maxRe);
    double ci = mapD(y, 0, HEIGHT, minIm, maxIm);
    double re = 0, im = 0;

    int iter;
    for (iter = 0; iter <= maxIter; iter++)
    {
        double tr = re * re - im * im + cr;
        im = 2 * re * im + ci;
        re = tr;

        if (re * re + im * im > 2) break;
    }
    if (iter >= maxIter) iter = -1;
    tft.drawPixel(x, y, getColor(iter));
   }
    
    //Draw stats
    tft.fillRoundRect(130, 14, 40, 10, 2 ,ST77XX_BLACK);
    tft.setTextSize(1);
    
    if(maxIter < 100){
      tft.setCursor(140 , 16);
    }else{
      tft.setCursor(135 , 16);
     }
     
    String iters = String(maxIter);
    iters.toCharArray(iterChar,5);
    tft.print(iterChar);
    
    tone(SPEAKER_OUTPUT_PIN, 800);
    delay(150);
    noTone(SPEAKER_OUTPUT_PIN);
    tone(SPEAKER_OUTPUT_PIN, 1400);
    delay(150);
    noTone(SPEAKER_OUTPUT_PIN);
}
unsigned long getColor(int iterations){
    int r = 0, g = 0, b = 0;

    if (iterations == -1) {
        // nothing to do

    } else if (iterations == 0) {
        r = 255;
        // g = 0;
        // b = 0;
    } else {
        // colour gradient:      Red -> Blue -> Green -> Red -> Black
        // corresponding values:  0  ->  16  ->  32   -> 64  ->  127 (or -1)
        if (iterations < 16) {
            r = 16 * (16 - iterations);
            // g = 0;
            b = 16 * iterations - 1;
        } else if (iterations < 32) {
            // r = 0;
            g = 16 * (iterations - 16);
            b = 16 * (32 - iterations) - 1;
        } else if (iterations < 64) {
            r = 8 * (iterations - 32);
            g = 8 * (64 - iterations) - 1;
            // b = 0;
        } else { // range is 64 - 127
            r = 255 - (iterations - 64) * 4;
            // g = 0;
            // b = 0;
        }
    } 
    // 888 RGB to 565 RGB
    return ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);
}
void drawBat(){

    unsigned long currentMillis = millis();

    String percs = String(oldPercInt);
    percs.toCharArray(printout2,5);
    tft.setTextSize(1);
    tft.setTextColor(batTextColor);
    tft.setCursor(135 , 4);
    tft.print(printout2);
    tft.print("%");
      
    if(currentMillis - previousMillis > interval || batFirtPowerOn) {
      previousMillis = currentMillis;  
       tft.fillRoundRect(130, 2, 40, 10, 2 ,ST77XX_BLACK);
      int sensorValue = analogRead(BAT_PIN); //read the pin value
      float voltage = sensorValue * (2.3 / 1023.00) * 2; //convert the value to a true voltage.
      float perc = mapfloat(voltage, 3.5, 4.1, 1, 100);

      if(perc>100){
        perc = 100;
      }

      int percInt = (int)perc;
      if(percInt < oldPercInt){
        //tft.fillRect(130, 1, 30,12, ST77XX_BLACK);
        oldPercInt = percInt;
        if(oldPercInt < 10) batTextColor = ST77XX_RED;
      }
      
      batFirtPowerOn = false;
    }
}
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max){
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
double mapD(double x, double in_min, double in_max, double out_min, double out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
