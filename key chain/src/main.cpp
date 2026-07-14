#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "eyes_anim.h"
#include "renard_anim.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>


#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_RESET -1
// seuil danim de mouvemnet
#define SEUIL_REPOS 200
#define SEUIL_ACTIF 400


bool modeRenard = false;

unsigned long debutRenard = 0;

#define DUREE_RENARD 15000   // 10 secondes
#define SEUIL_RENARD 500
// pour limiter que les anim et quelles reviennent à 0 
#define FOX_FRAMES 45
#define EYES_FRAMES 76

unsigned long dernier_clignement = 0;
bool clignement = false;
int blink_frame = 66;

#define TEMPS_CLIGNEMENT 5000

//variables d'anim
int fox_frames = 0;
bool fox_reveille = false ;
#define fox_reveille_frames 6
#define fox_walk_frames 38

int eyes_frames = 0;
bool eyes_reveille = false;
#define eyes_reveille_frames 39

int idle_frames = 40;
#define eyes_stare_lenframes 10

#define eyes_cling_frames 31
#define eyes_cling_lenframes 9

#define eyes_whistle_frames 51
#define eyes_whistle_lenframes 14

#define eyes_close_frames 66
#define eyes_close_lenframes 10

bool isAnimating = false;
int anim_start = 0;
int animFrames = 0;
int anim_len = 0;

int idle_count = 0;
bool isIdle = true;

unsigned long lastFrames = 0;
#define FRAMES_DELAY 60

//arbres 
float scrollFond = 0;
float scrollDevant = 0;

bool mpu_ok = false;

//variable accelerométre
float ax , ay, az;
float mouvement = 0;
float mouvement_filtre = 0;

unsigned long dernier_mouvement = 0;

#define TEMPS_REACTION 1000
#define TEMPS_RETOUR 8000


Adafruit_MPU6050 mpu;
Adafruit_SSD1306 ecran(SCREEN_WIDTH,SCREEN_HEIGHT,&Wire, OLED_RESET);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
Serial.println("XIAO ESP32-S3 OK");

Wire.begin(5,6);

if (mpu.begin(0x68, &Wire)) {
    mpu_ok = true;
    Serial.println("MPU6050 OK !");
} else {
    mpu_ok = false;
    Serial.println("MPU6050 non trouvé !");
}

if (!ecran.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED NON TROUVE !");
    while (1);
} else {
    Serial.println("OLED OK !");
}

ecran.clearDisplay();
ecran.display();

Serial.println("Scanning I2C...");
for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
        Serial.print("Device found at 0x");
        Serial.println(addr, HEX);
    }
}
}
void drawSapin(int x, int y, int h, int couleur) {
  // Tronc
  ecran.drawRect(x, y, 3, h/3, couleur);
  // Couche basse
  ecran.fillTriangle(x-h/2, y + h/3, x+h/2, y, x, y-h/2, couleur);
  // Couche haute
  ecran.fillTriangle(x-h/3, y-h/3, x+h/3, y-h/3, x, y-h, couleur);
}

void drawBambou(int x, int y, int h) {
  // Tige
  ecran.drawRect(x, y-h, 3, h, WHITE);
  // Noeuds
  ecran.drawRect(x-2, y-h/3, 7, 2, WHITE);
  ecran.drawRect(x-2, y-h*2/3, 7, 2, WHITE);
  // Feuille
  ecran.drawRect(x+3, y-h/2, 8, 2, WHITE);
}


void loop() {
  
  // put your main code here, to run repeatedly:
 
 sensors_event_t a, g, temp;


if(mpu_ok && mpu.getEvent(&a,&g,&temp))
{
    mouvement = abs(g.gyro.x) + abs(g.gyro.y) + abs(g.gyro.z);
    mouvement *= 100;


    //  eviter les tremblements
    mouvement_filtre = mouvement_filtre * 0.85 + mouvement * 0.15;


    // secousse reveil du renard
    if(mouvement_filtre > SEUIL_RENARD && !modeRenard)
    {
      modeRenard = true;
      debutRenard = millis();

      fox_frames = 0;
      fox_reveille = false;
    }
}
else
{
    mouvement_filtre = 0;
}

  bool nouvelleFrame = (millis() - lastFrames > FRAMES_DELAY);
  if(modeRenard && millis() - debutRenard > DUREE_RENARD)
  {
    modeRenard = false;

    fox_frames = 0;
    fox_reveille = false;
    
    eyes_reveille = false;
    animFrames = 0;
    isIdle = true;
  }
  if(nouvelleFrame)
  {
    lastFrames = millis();
  }

  if (modeRenard){

    ecran.clearDisplay();


      if (!fox_reveille)  // premier tour pour le reveil
      {
        ecran.drawBitmap(8,6, fox[fox_frames],FOX_W,FOX_H,WHITE); 

        if( nouvelleFrame)
        {
          fox_frames++;
        }
        if (fox_frames >= FOX_FRAMES)
        {
          fox_reveille = true;
          fox_frames = 0;
        }
      }
      else
      {

        ecran.drawBitmap(8,6, fox[fox_reveille_frames + fox_frames],FOX_W,FOX_H,WHITE);
        //le fond il passet derrier le renard donc dessiner avant
        drawSapin((int)scrollFond , 45, 30,BLACK);
        drawSapin((int)scrollFond + 50,45,20,BLACK);
        drawSapin((int)scrollFond  - 64, 45,17,BLACK);
        if( nouvelleFrame) 
        {
          fox_frames++;
          scrollFond -= 0.5;
          scrollDevant -=0.8;
          if (scrollFond < 0) scrollFond =128;
          if (scrollDevant < 0) scrollDevant =128;
        }
      
      
        //la forme nan je decone le devant
        drawBambou((int)scrollDevant + 10, 30, 30 );
        drawBambou((int)scrollDevant + 100, 40, 40);
        drawBambou((int)scrollDevant + 120, 35, 30 );
        drawBambou((int)scrollDevant + 80, 30, 20);
        drawBambou((int)scrollDevant - 20, 35, 30 );
        drawBambou((int)scrollDevant - 50, 20, 20);
        
        if (fox_frames > fox_walk_frames) fox_frames = 0;
      }
      ecran.display();
    
    }
    else
    {

      fox_reveille = false;
      ecran.clearDisplay();
      if (!eyes_reveille) // premier tour pour le reveil
      {
        
        ecran.drawBitmap(32,0, eyes[animFrames],64,64,WHITE); 

        if( nouvelleFrame) animFrames++;
        if (animFrames >= EYES_FRAMES)
        {
          eyes_reveille = true;
          animFrames = 0;
        }
      }
      else if(eyes_reveille && isIdle)
      {
      
        ecran.drawBitmap(32,0,eyes[idle_frames],64,64,WHITE);

        if(nouvelleFrame)
        {
          idle_frames++;
          idle_count++;
          if (idle_frames > anim_start + anim_len) idle_frames = anim_start;
        
          if (idle_count > anim_len * 2) isIdle = false;
        }
        ecran.display();

      }
      else if (eyes_reveille && !isIdle)
      { //puis si reveil on lace un anim au hasard
        if (!isAnimating) 
        {
          int animNum = random(10);
          if (animNum < 5 )
          {
            eyes_frames = eyes_cling_frames;
            anim_len = eyes_cling_lenframes;
            anim_start = eyes_cling_frames;
            isAnimating = true;
          }
          else if (animNum < 6 )
          {
            eyes_frames = eyes_whistle_frames;
            anim_len = eyes_whistle_lenframes;
            anim_start = eyes_whistle_frames;
            isAnimating = true;
          }
          else  {
            eyes_frames = eyes_close_frames;
            anim_len = eyes_close_lenframes;
            anim_start = eyes_close_frames;
            isAnimating = true;
          } 
        }
        ecran.drawBitmap(32,0, eyes[eyes_frames],64,64,WHITE);
        if (nouvelleFrame)
        {
          eyes_frames++;
        }

        if (eyes_frames >= anim_start + anim_len){
          eyes_frames = anim_start;
          isAnimating = false;
          isIdle = true;
          idle_count = 0;
          idle_frames = 40;
          anim_len = eyes_stare_lenframes;
          anim_start = 40;
        }

      }
      ecran.display();
    }
  }

  
  