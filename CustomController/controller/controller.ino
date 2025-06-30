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

#define STATE_STACK_MAX 10

enum States {
	STATE_NONE,
	STATE_HOME,
	STATE_MENU,
	STATE_CONFIG,
	STATE_GAIT,
	STATE_MODE,
	STATE_ANIMATION,
	STATE_LEG
	};

States stateStack[STATE_STACK_MAX];
int stackTop = -1;
States state;

struct page {
	char* item;
	const unsigned char* icon;
	States destination;
};

U8G2_SSD1309_128X64_NONAME0_1_HW_I2C u8g2(U8G2_R0);
//U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0);

const int MENU_ITEMS = 5;
page MENU[MENU_ITEMS] = {
	{"Config", epd_bitmap_cog_icon, STATE_CONFIG},
	{"Mode", epd_bitmap_controller_icon, STATE_MODE},
	{"Gait", epd_bitmap_paw_icon, STATE_GAIT},
  {"Home Screen", epd_bitmap_home_icon, STATE_HOME},
	{"Animation", epd_bitmap_film_icon, STATE_ANIMATION}
};

const int CONFIG_ITEMS = 6;
page CONFIG[CONFIG_ITEMS] = {
  {"Leg1", epd_bitmap_leg1_icon, STATE_LEG},
	{"Leg2", epd_bitmap_leg2_icon, STATE_LEG},
	{"Leg3", epd_bitmap_leg3_icon, STATE_LEG},
  {"Leg4", epd_bitmap_leg4_icon, STATE_LEG},
	{"Leg5", epd_bitmap_leg5_icon, STATE_LEG},
  {"Leg6", epd_bitmap_leg6_icon, STATE_LEG}
};

enum Joints {coxa, femur, tibia};
const int LEG_ITEMS = 3;
page LEG[LEG_ITEMS] = {
	{"Coxa", epd_bitmap_joint_selected_icon, STATE_NONE},
	{"Femur", epd_bitmap_joint_selected_icon, STATE_NONE},
	{"Tibia", epd_bitmap_joint_selected_icon, STATE_NONE}
};

enum Gaits {tripod, wave, ripple};
Gaits activeGait = tripod; 
const int GAIT_ITEMS = 3;
page GAIT[GAIT_ITEMS] = {
	{"Tripod", nullptr, STATE_NONE},
	{"Wave", nullptr, STATE_NONE},
	{"Ripple", nullptr, STATE_NONE}
};

enum Modes {strafe, normal, tilt};
Modes activeMode = normal;
const int MODE_ITEMS = 3;
page MODE[MODE_ITEMS] = {
  {"Strafe", epd_bitmap_strafe_mode_icon, STATE_NONE},
	{"Normal", epd_bitmap_normal_mode_icon, STATE_NONE},
	{"Tilt", epd_bitmap_tilt_mode_icon, STATE_NONE}	
};

int item_selected = 0;
int item_previous;
int item_next;

int encoderCounter = 0;
volatile bool lastA, lastB;

/*
void IRAM_ATTR handleEncoderInterrupt() {
  bool A = digitalRead(encoderA);
  bool B = digitalRead(encoderB);

  // Determine rotation direction
  if (A != lastA) {if (A == B) encoderCounter ++; if (A != B) encoderCounter --;} 

  lastA = A;
  lastB = B;
}
*/

//_______________________________________________________________________setup__________________________________________________________________
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
  //attachInterrupt(digitalPinToInterrupt(encoderA), handleEncoderInterrupt, CHANGE);
  //attachInterrupt(digitalPinToInterrupt(encoderB), handleEncoderInterrupt, CHANGE);

  u8g2.begin();
  u8g2.setFont(u8g2_font_5x8_mn);
  u8g2.setColorIndex(1);

  state = STATE_HOME;
}

//_______________________________________________________________________loop__________________________________________________________________
void loop() {
  readInputData();
	mainFSM();
  u8g2.firstPage();
  do {
		if (state == STATE_HOME) homePage();
		if (state == STATE_MENU) menuPage();
		if (state == STATE_CONFIG) configPage();
		if (state == STATE_LEG) legPage();
		if (state == STATE_GAIT) gaitPage();
		if (state == STATE_MODE) modePage();
		//if (state == STATE_ANIMATION) animationPage();
  } while ( u8g2.nextPage() );
}

//_______________________________________________________________________pushState__________________________________________________________________
void pushState(States s) {
	if (stackTop < STATE_STACK_MAX - 1) {stateStack[++stackTop] = s;}
}

//_______________________________________________________________________popState__________________________________________________________________
States popState() {
	if (stackTop >= 0) {return stateStack[stackTop--];}
	return STATE_HOME; // fallback if stack is empty
}

//______________________________________________________________________handleScrollAndSelect_________________________________________________________
void handleScrollAndSelect(page* pages, int itemCount, bool destination = true) {
	if((button1Z1 != button1Z0) && (!button1Z0)) {state = popState();}
	if((upButtonZ1 != upButtonZ0) && (!upButtonZ0)) {item_selected --; if (item_selected < 0) item_selected = itemCount - 1;}
  if((downButtonZ1 != downButtonZ0) && (!downButtonZ0)) {item_selected ++; if (item_selected >= itemCount) item_selected = 0;}
	if(!destination) {return;}
	if((selectButtonZ1 != selectButtonZ0) && (!selectButtonZ0)) {pushState(state); state = pages[item_selected].destination;}
}

//_______________________________________________________________________mainFSM__________________________________________________________________
void mainFSM() {
	switch (state) {
		case STATE_HOME:
      if((button2Z1 != button2Z0) && (!button2Z0)) {activeGait = (activeGait + 1) % GAIT_ITEMS;}
			if((button3Z1 != button3Z0) && (!button3Z0)) {activeMode = (activeMode + 1) % MODE_ITEMS;}
			if((selectButtonZ1 != selectButtonZ0) && (!selectButtonZ0)) {pushState(state);; state = STATE_MENU;}
			break;
		case STATE_MENU:
			handleScrollAndSelect(MENU, MENU_ITEMS);
			break;
		case STATE_CONFIG:
			handleScrollAndSelect(CONFIG, CONFIG_ITEMS);
			break;
		case STATE_LEG:
			handleScrollAndSelect(LEG, LEG_ITEMS);
		case STATE_GAIT:
			handleScrollAndSelect(GAIT, GAIT_ITEMS, false);
			if((selectButtonZ1 != selectButtonZ0) && (!selectButtonZ0)) {activeGait = static_cast<Gaits>(item_selected);}
			break;
		case STATE_MODE:
			handleScrollAndSelect(MODE, MODE_ITEMS, false);
			if((selectButtonZ1 != selectButtonZ0) && (!selectButtonZ0)) {activeMode = static_cast<Modes>(item_selected);}
			break;
		default:
			break;
	}
}


//_______________________________________________________________________menuPage__________________________________________________________________
void menuPage() {
	item_previous = item_selected - 1;
  if (item_previous < 0) item_previous = MENU_ITEMS - 1;

  item_next = item_selected + 1;
  if (item_next >= MENU_ITEMS) item_next = 0;

	// Back button and nav
	u8g2.setFont(u8g2_font_4x6_mf);
  u8g2.drawXBMP(0, 0, 26, 10, epd_bitmap_button_boarder);
  u8g2.drawStr(5, 7, "Back");
	u8g2.drawStr(29, 7, "Menu");

  // Previous
  u8g2.setFont(u8g_font_7x14);
  u8g2.drawStr(26, 23, MENU[item_previous].item);
  u8g2.drawXBMP(4, 10, 16, 16, MENU[item_previous].icon);

  // Current
  u8g2.setFont(u8g_font_7x14B); 
	u8g2.drawXBMP(1, 27, 126, 20, epd_bitmap_selection_boarder);
  u8g2.drawStr(26, 41, MENU[item_selected].item);
  u8g2.drawXBMP(4, 28, 16, 16, MENU[item_selected].icon);

  // Next
  u8g2.setFont(u8g_font_7x14);
  u8g2.drawStr(26, 60, MENU[item_next].item);
  u8g2.drawXBMP(4, 47, 16, 16, MENU[item_next].icon);
}

//_______________________________________________________________________homePage__________________________________________________________________
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
	u8g2.drawStr(36, 18, GAIT[static_cast<int>(activeGait)].item);

	// Mode Button
	u8g2.setFont(u8g2_font_4x6_mf);
  u8g2.drawXBMP(1, 22, 26, 10, epd_bitmap_button_boarder);
  u8g2.drawStr(5, 29, "Mode");
	u8g2.drawStr(36, 29, MODE[static_cast<int>(activeMode)].item);

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

//_______________________________________________________________________configPage__________________________________________________________________
void configPage() {
  item_previous = item_selected - 1;
  if (item_previous < 0) item_previous = CONFIG_ITEMS - 1;

  item_next = item_selected + 1;
  if (item_next >= CONFIG_ITEMS) item_next = 0;

	// Back button and nav
	u8g2.setFont(u8g2_font_4x6_mf);
  u8g2.drawXBMP(0, 0, 26, 10, epd_bitmap_button_boarder);
  u8g2.drawStr(5, 7, "Back");
	u8g2.drawStr(29, 7, "Menu>Config");

  // Hexapod
	u8g2.drawXBMP(79, 12, 48, 48, epd_bitmap_hex_boarder);

  // Previous
  u8g2.setFont(u8g_font_7x14);
  u8g2.drawStr(26, 23, CONFIG[item_previous].item);

  // Current
  u8g2.setFont(u8g_font_7x14B); 
	u8g2.drawXBMP(1, 27, 77, 20, epd_bitmap_selection_boarder_hex);
  u8g2.drawStr(26, 41, CONFIG[item_selected].item);
  u8g2.drawXBMP(88, 18, 30, 37, CONFIG[item_selected].icon);

  // Next
  u8g2.setFont(u8g_font_7x14);
  u8g2.drawStr(26, 60, CONFIG[item_next].item);
}

//_______________________________________________________________________legPage__________________________________________________________________
void legPage() {
  item_previous = item_selected - 1;
  if (item_previous < 0) item_previous = LEG_ITEMS - 1;

  item_next = item_selected + 1;
  if (item_next >= LEG_ITEMS) item_next = 0;

  // Hexapod
	u8g2.drawXBMP(79, 12, 48, 48, epd_bitmap_hex_boarder);

	// Back button and nav
	u8g2.setFont(u8g2_font_4x6_mf);
  u8g2.drawXBMP(0, 0, 26, 10, epd_bitmap_button_boarder);
  u8g2.drawStr(5, 7, "Back");
	u8g2.drawStr(29, 7, "Menu>Config>Leg?");

  // Previous
  u8g2.setFont(u8g_font_7x14);
  u8g2.drawStr(26, 23, LEG[item_previous].item);

  // Current
  u8g2.setFont(u8g_font_7x14B); 
	u8g2.drawXBMP(1, 27, 77, 20, epd_bitmap_selection_boarder_hex);
  u8g2.drawXBMP(81, 24, 40, 24, epd_bitmap_leg_icon);
  u8g2.drawStr(26, 41, LEG[item_selected].item);

  if (item_selected == coxa) u8g2.drawXBMP(85, 37, 3, 3, LEG[item_selected].icon);
  if (item_selected == femur) u8g2.drawXBMP(96, 37, 3, 3, LEG[item_selected].icon);
  if (item_selected == tibia) u8g2.drawXBMP(107, 26, 3, 3, LEG[item_selected].icon);

  // Next
  u8g2.setFont(u8g_font_7x14);
  u8g2.drawStr(26, 60, LEG[item_next].item);
}

//_______________________________________________________________________gaitPage__________________________________________________________________
void gaitPage() {
  item_previous = item_selected - 1;
  if (item_previous < 0) item_previous = GAIT_ITEMS - 1;

  item_next = item_selected + 1;
  if (item_next >= GAIT_ITEMS) item_next = 0;

  // Hexapod
	u8g2.drawXBMP(79, 12, 48, 48, epd_bitmap_hex_boarder);

	// Back button and nav
	u8g2.setFont(u8g2_font_4x6_mf);
  u8g2.drawXBMP(0, 0, 26, 10, epd_bitmap_button_boarder);
  u8g2.drawStr(5, 7, "Back");
	u8g2.drawStr(29, 7, "Menu>Gait");

  // Previous
  u8g2.setFont(u8g_font_7x14);
  u8g2.drawStr(18, 23, GAIT[item_previous].item);
  if (item_previous == activeGait) u8g2.drawXBMP(5, 15, 7, 7, epd_bitmap_selected_icon);

  // Current
  u8g2.setFont(u8g_font_7x14B); 
	u8g2.drawXBMP(1, 27, 77, 20, epd_bitmap_selection_boarder_hex);
  u8g2.drawStr(18, 41, GAIT[item_selected].item);
  if (item_selected == activeGait) u8g2.drawXBMP(5, 33, 7, 7, epd_bitmap_selected_icon);

  // Next
  u8g2.setFont(u8g_font_7x14);
  u8g2.drawStr(18, 60, GAIT[item_next].item);
  if (item_next == activeGait) u8g2.drawXBMP(5, 51, 7, 7, epd_bitmap_selected_icon);
}

//_______________________________________________________________________modePage__________________________________________________________________
void modePage() {
  item_previous = item_selected - 1;
  if (item_previous < 0) item_previous = MODE_ITEMS - 1;

  item_next = item_selected + 1;
  if (item_next >= MODE_ITEMS) item_next = 0;

  // Hexapod
	u8g2.drawXBMP(79, 12, 48, 48, epd_bitmap_hex_boarder);

	// Back button and nav
	u8g2.setFont(u8g2_font_4x6_mf);
  u8g2.drawXBMP(0, 0, 26, 10, epd_bitmap_button_boarder);
  u8g2.drawStr(5, 7, "Back");
	u8g2.drawStr(29, 7, "Menu>Mode");

  // Previous
  u8g2.setFont(u8g_font_7x14);
  u8g2.drawStr(18, 23, MODE[item_previous].item);
  if (item_previous == activeMode) u8g2.drawXBMP(5, 15, 7, 7, epd_bitmap_selected_icon);

  // Current
  u8g2.setFont(u8g_font_7x14B); 
	u8g2.drawXBMP(1, 27, 77, 20, epd_bitmap_selection_boarder_hex);
  u8g2.drawStr(18, 41, MODE[item_selected].item);
  u8g2.drawXBMP(82, 15, 42, 42, MODE[item_selected].icon);
  if (item_selected == activeMode) u8g2.drawXBMP(5, 33, 7, 7, epd_bitmap_selected_icon);

  // Next
  u8g2.setFont(u8g_font_7x14);
  u8g2.drawStr(18, 60, MODE[item_next].item);
  if (item_next == activeMode) u8g2.drawXBMP(5, 51, 7, 7, epd_bitmap_selected_icon);
}

//_______________________________________________________________________readInputData__________________________________________________________________
void readInputData() {
  readButtonData();
  readStickData();
}

//_______________________________________________________________________readButtonData__________________________________________________________________
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

//_______________________________________________________________________readStickData__________________________________________________________________
void readStickData() {
  // Read raw analog values (range 0–4095)
  int xRaw = analogRead(stick1X);
  int yRaw = analogRead(stick1Y);

  // Normalize to range -100 to 100 with deadzone
  int x = map(xRaw, 0, 4095, -127, 128);
  int y = map(yRaw, 0, 4095, -127, 128);
}