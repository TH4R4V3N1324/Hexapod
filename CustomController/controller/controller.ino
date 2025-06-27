#include <U8g2lib.h>
#include "bitmaps.h"
#include <Wire.h>
#include <SPI.h>

//#define SDAPin 33
//#define SCLPin 32

#define stick1X 17
#define stick1Y 18

#define stick1X 19
#define stick1Y 20

#define upButton 2
bool upButtonZ0 = false;
bool upButtonZ1 = false;

#define selectButton 3
bool selectButtonZ0 = false;
bool selectButtonZ1 = false;

#define downButton 4
bool downButtonZ0 = false;
bool downButtonZ1 = false;

#define button1 5
bool button1Z0 = false;
bool button1Z1 = false;

#define button2 6
bool button2Z0 = false;
bool button2Z1 = false;

#define button3 7
bool button3Z0 = false;
bool button3Z1 = false;

#define button4 8
bool button4Z0 = false;
bool button4Z1 = false;

#define button5 9
bool button5Z0 = false;
bool button5Z1 = false;

#define switch1 10
#define switch2 11
#define switch3 12
#define switch4 13

#define encoderA 14
#define encoderB 15
#define encoderButton 16

U8G2_SSD1309_128X64_NONAME0_1_HW_I2C u8g2(U8G2_R0);
//U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0);

const int NUM_ITEMS = 5;

// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 144)
const unsigned char* icons[NUM_ITEMS] = {
	epd_bitmap_cog_icon,
	epd_bitmap_controller_icon,
	epd_bitmap_paw_icon,
  epd_bitmap_home_icon,
	epd_bitmap_film_icon
};

char menu_item[] [20] = {
  {"Config"},
	{"Mode"},
	{"Gait"},
  {"Home Screen"},
	{"Animation"}
};

int item_selected = 0;
int item_previous;
int item_next;

int encoderCounter = 0;
volatile bool lastA, lastB;

enum States {home, menu, config, gait, mode, animation};
States state;

void IRAM_ATTR handleEncoderInterrupt() {
  bool A = digitalRead(encoderA);
  bool B = digitalRead(encoderB);

  // Determine rotation direction
  if (A != lastA) {if (A == B) encoderCounter ++; if (A != B) encoderCounter --;} 

  lastA = A;
  lastB = B;
}

void setup() {
  //Wire.begin();
  pinMode(upButton, INPUT_PULLUP);
  pinMode(selectButton, INPUT_PULLUP);
  pinMode(downButton, INPUT_PULLUP);
  pinMode(encoderButton, INPUT_PULLUP);
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(button4, INPUT_PULLUP);
  pinMode(button5, INPUT_PULLUP);
  pinMode(switch1, INPUT_PULLUP);
  pinMode(switch2, INPUT_PULLUP);
  pinMode(switch3, INPUT_PULLUP);
  pinMode(switch4, INPUT_PULLUP);

  lastA = digitalRead(encoderA);
  lastB = digitalRead(encoderB);
  attachInterrupt(digitalPinToInterrupt(encoderA), handleEncoderInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderB), handleEncoderInterrupt, CHANGE);

  u8g2.begin();
  u8g2.setFont(u8g2_font_5x8_mn);
  u8g2.setColorIndex(1);

  state = home;
}

void loop() {
  readInputData();
  mainFSM();

  u8g2.firstPage();
  do {
    if (state == menu) menuPage();
    if (state == home) homePage();
  } while ( u8g2.nextPage() );
}

void mainFSM() {
  switch (state) {
    case home:
    if((selectButtonZ1 != selectButtonZ0) && (!selectButtonZ0)) {state = menu;}
      //if button 1
      //if button 2
      //if button 3
      //if button 4
      break;
    case menu:
      if((upButtonZ1 != upButtonZ0) && (!upButtonZ0)) {item_selected --; if (item_selected < 0) item_selected = NUM_ITEMS - 1;}
      if((downButtonZ1 != downButtonZ0) && (!downButtonZ0)) {item_selected ++; if (item_selected >= NUM_ITEMS) item_selected = 0;}
      if((selectButtonZ1 != selectButtonZ0) && (!selectButtonZ0)) {if (item_selected == 3 && state == menu) state = home;}

      item_previous = item_selected - 1;
      if (item_previous < 0) item_previous = NUM_ITEMS - 1;
      item_next = item_selected + 1;
      if (item_next >= NUM_ITEMS) item_next = 0;
      //if button 1
      //if button 2
      //if button 3
      //if button 4
      break;
    case config:
      //if button 1
      //if button 2
      //if button 3
      //if button 4
      break;
    case gait:
      //if button 1
      //if button 2
      //if button 3
      //if button 4
      break;
    case mode:
      //if button 1
      //if button 2
      //if button 3
      //if button 4
      break;
    case animation:
      //if button 1
      //if button 2
      //if button 3
      //if button 4
      break;
    default:
      break;
  }
}

void menuPage() {
	item_previous = item_selected - 1;
  if (item_previous < 0) item_previous = NUM_ITEMS - 1;

  item_next = item_selected + 1;
  if (item_next >= NUM_ITEMS) item_next = 0;

	// Back button and nav
	u8g2.setFont(u8g2_font_4x6_mf);
  u8g2.drawXBMP(0, 0, 26, 10, epd_bitmap_button_boarder);
  u8g2.drawStr(5, 7, "Back");
	u8g2.drawStr(29, 7, "Menu");

  // Previous
  u8g2.setFont(u8g_font_7x14);
  u8g2.drawStr(26, 23, menu_item[item_previous]);
  u8g2.drawXBMP(4, 10, 16, 16, icons[item_previous]);

  // Current
  u8g2.setFont(u8g_font_7x14B); 
	u8g2.drawXBMP(1, 27, 126, 20, epd_bitmap_selection_boarder);
  u8g2.drawStr(26, 41, menu_item[item_selected]);
  u8g2.drawXBMP(4, 28, 16, 16, icons[item_selected]);

  // Next
  u8g2.setFont(u8g_font_7x14);
  u8g2.drawStr(26, 60, menu_item[item_next]);
  u8g2.drawXBMP(4, 47, 16, 16, icons[item_next]);
}

void homePage() {
	// Switch 1
  u8g2.setFont(u8g2_font_4x6_mf);
  u8g2.drawXBMP(2, 1, 5, 7, epd_bitmap_switch_up_icon);
  u8g2.drawStr(9, 7, "SW1");

	// Switch 2
  u8g2.setFont(u8g2_font_4x6_mf);
  u8g2.drawXBMP(35, 1, 5, 7, epd_bitmap_switch_up_icon);
  u8g2.drawStr(42, 7, "SW2");

	// Switch 3
  u8g2.setFont(u8g2_font_4x6_mf);
  u8g2.drawXBMP(67, 1, 5, 7, epd_bitmap_switch_up_icon);
  u8g2.drawStr(74, 7, "SW3");

	// Switch 4
  u8g2.setFont(u8g2_font_4x6_mf);
  u8g2.drawXBMP(98, 1, 5, 7, epd_bitmap_switch_up_icon);
  u8g2.drawStr(105, 7, "SW4");

	// Gait Button
	u8g2.setFont(u8g2_font_4x6_mf);
  u8g2.drawXBMP(1, 11, 26, 10, epd_bitmap_button_boarder);
  u8g2.drawStr(5, 18, "Gait");
	//u8g2.drawStr(36, 18, GaitValue);

	// Mode Button
	u8g2.setFont(u8g2_font_4x6_mf);
  u8g2.drawXBMP(1, 22, 26, 10, epd_bitmap_button_boarder);
  u8g2.drawStr(5, 29, "Mode");
	//u8g2.drawStr(36, 29, ModeValue);

	// Phase
	u8g2.setFont(u8g2_font_4x6_mf);
  u8g2.drawStr(3, 40, "Phase");
	//u8g2.drawStr(36, 40, PhaseValue);
	
	// Height
	u8g2.setFont(u8g2_font_4x6_mf);
  u8g2.drawStr(3, 49, "Height");
	//u8g2.drawStr(36, 49, "HeightValue");

	// Menu Button
	u8g2.setFont(u8g2_font_4x6_mf);
  u8g2.drawXBMP(1, 53, 26, 10, epd_bitmap_button_boarder);
  u8g2.drawStr(5, 60, "Menu");

	// Hexapod
	u8g2.drawXBMP(72, 12, 48, 48, epd_bitmap_hex_boarder);
}

void configPage() {
  item_previous = item_selected - 1;
  if (item_previous < 0) item_previous = NUM_ITEMS - 1;

  item_next = item_selected + 1;
  if (item_next >= NUM_ITEMS) item_next = 0;

	// Back button and nav
	u8g2.setFont(u8g2_font_4x6_mf);
  u8g2.drawXBMP(0, 0, 26, 10, epd_bitmap_button_boarder);
  u8g2.drawStr(5, 7, "Back");
	u8g2.drawStr(29, 7, "Menu>Config");

  // Previous
  u8g2.setFont(u8g_font_7x14);
  u8g2.drawStr(26, 23, menu_item[item_previous]);
  u8g2.drawXBMP(4, 10, 16, 16, icons[item_previous]);

  // Current
  u8g2.setFont(u8g_font_7x14B); 
	u8g2.drawXBMP(1, 27, 77, 20, epd_bitmap_selection_boarder_hex);
  u8g2.drawStr(26, 41, menu_item[item_selected]);
  u8g2.drawXBMP(4, 28, 16, 16, icons[item_selected]);

  // Next
  u8g2.setFont(u8g_font_7x14);
  u8g2.drawStr(26, 60, menu_item[item_next]);
  u8g2.drawXBMP(4, 47, 16, 16, icons[item_next]);

  // Hexapod
	u8g2.drawXBMP(79, 12, 48, 48, epd_bitmap_hex_boarder);
}

void readInputData() {
  readButtonData();
  readStickData();
}

void readButtonData() {
  upButtonZ1 = upButtonZ0; upButtonZ0 = digitalRead(upButton);
  downButtonZ1 = downButtonZ0; downButtonZ0 = digitalRead(downButton);
  selectButtonZ1 = selectButtonZ0; selectButtonZ0 = digitalRead(selectButton);
  button1Z1 = button1Z0; button1Z0 = digitalRead(button1);
  button2Z1 = button2Z0; button2Z0 = digitalRead(button2);
  button3Z1 = button3Z0; button3Z0 = digitalRead(button3);
  button4Z1 = button4Z0; button4Z0 = digitalRead(button4);
  button5Z1 = button5Z0; button5Z0 = digitalRead(button5);
}

void readStickData() {
  // Read raw analog values (range 0–4095)
  int xRaw = analogRead(stick1X);
  int yRaw = analogRead(stick1Y);

  // Normalize to range -100 to 100 with deadzone
  int x = map(xRaw, 0, 4095, -127, 128);
  int y = map(yRaw, 0, 4095, -127, 128);
}