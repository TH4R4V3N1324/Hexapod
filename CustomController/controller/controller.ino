#include "bitmaps.h"
#include <U8g2lib.h> 
#include <SPI.h>

#define stick1X 16  // Joystick 1 - X axis
#define stick1Y 17  // Joystick 1 - Y axis
int joy1X;
int joy1Y;
int centerX = 0;
int centerY = 0;

#define stick2X 18  // Joystick 2 - X axis
#define stick2Y 19  // Joystick 2 - Y axis

#define button1 4
bool button1Z0 = false;
bool button1Z1 = false;

#define button2 5
bool button2Z0 = false;
bool button2Z1 = false;

#define button3 6
bool button3Z0 = false;
bool button3Z1 = false;

#define button4 7
bool button4Z0 = false;
bool button4Z1 = false;

#define switch1 8
#define switch2 9
#define switch3 10
#define switch4 11

#define encoderA 20
#define encoderB 21
#define encoderButton 33
bool encoderButtonZ0 = false;
bool encoderButtonZ1 = false;

#define STATE_STACK_MAX 10

enum States {
	STATE_NONE,
	STATE_HOME,
	STATE_MENU,
	STATE_CONFIG,
	STATE_GAIT,
	STATE_MODE,
	STATE_ANIMATION,
	STATE_LEG,
	STATE_JOINT
	};

struct StateStack {
	States state;
	int item_selected;

	StateStack(int s = 0, int i = 0) : state(static_cast<States>(s)), item_selected(i) {}
};

StateStack stateStack[STATE_STACK_MAX];
int stackTop = -1;
int stackIndex = 0;
States state;

struct page {
	char* item;
	const unsigned char* icon;
	States destination;
};

int LEG_OFFSET[6][3];
int jointOffset = 0;
int leg_selected;
int joint_selected;

U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI u8g2(
  U8G2_R0,       // rotation
  /* cs=*/ 0,   // GPIO0
  /* dc=*/ 1,   // GPIO1
  /* reset=*/ 2 // GPIO2
);

const int MENU_ITEMS = 4;
page MENU[MENU_ITEMS] = {
	{"Config", epd_bitmap_cog_icon, STATE_CONFIG},
	{"Mode", epd_bitmap_controller_icon, STATE_MODE},
	{"Gait", epd_bitmap_paw_icon, STATE_GAIT},
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
	{"Coxa", epd_bitmap_leg_icon, STATE_JOINT},
	{"Femur", epd_bitmap_leg_icon, STATE_JOINT},
	{"Tibia", epd_bitmap_leg_icon, STATE_JOINT}
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

enum EncoderStates {AB, Ab, aB, ab};
EncoderStates encoderState;
volatile int encoderCount = 0;
int lastEncoderCount = 0;
int encoderDelta = 0;
int encoderCountPerIndent = 4;

int currentPhase = 0;
int currentHeight = 100;

float scrollPosition = 0.0f; // Accumulates encoderDelta
const float SCROLL_THRESHOLD = 1.0f; // Change item when this is exceeded
float visualScrollIndex = 0.0f; // For smooth scrolling

const int SCREEN_HEIGHT = 64;
const int NAVBAR_HEIGHT = 10;
const int ITEM_HEIGHT = 18;
const float CENTER_Y = 42.0f;
const float ySpacing = 19.0f;

//_______________________________________________________________________setup__________________________________________________________________
void setup() {
	Serial.begin(115200);
  pinMode(encoderButton, INPUT_PULLUP);
	pinMode(encoderA, INPUT_PULLUP);
	pinMode(encoderB, INPUT_PULLUP);
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(button4, INPUT_PULLUP);
  pinMode(switch1, INPUT_PULLUP);
  pinMode(switch2, INPUT_PULLUP);
  pinMode(switch3, INPUT_PULLUP);
  pinMode(switch4, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(encoderA), doEncoderFSM, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderB), doEncoderFSM, CHANGE);

	// Read joystick at rest to find center
  centerX = calibrateCenter(stick1X);
  centerY = calibrateCenter(stick1Y);

  u8g2.begin();
  u8g2.setFont(u8g2_font_5x8_mn);
  u8g2.setColorIndex(1);

	initializeEncoder();

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
		if (state == STATE_JOINT) jointPage();
  } while ( u8g2.nextPage() );
}

//_______________________________________________________________________pushState__________________________________________________________________
void pushState(int currentState, int selectedItem) {
  if (stackIndex < STATE_STACK_MAX) {stateStack[stackIndex++] = { currentState, selectedItem };}
}

//_______________________________________________________________________popState__________________________________________________________________
StateStack popState() {
  if (stackIndex > 0) {return stateStack[--stackIndex];}
  return { 0, 0 };  // Default fallback
}

//_______________________________________________________________________backPage__________________________________________________________________
void backPage() {
	StateStack restored = popState();
	item_selected = restored.item_selected;
	state = restored.state;
}

//______________________________________________________________________handleScrollAndSelect_________________________________________________________
void handleScrollAndSelect(page* pages, int itemCount, bool destination = true) {
	if ((button1Z1 != button1Z0) && (!button1Z0)) {backPage(); return;}

	int delta = encoderCount - lastEncoderCount;

	if (abs(delta) >= encoderCountPerIndent) {
		lastEncoderCount = encoderCount;
		if (delta > 0) {item_selected = (item_selected + itemCount - 1) % itemCount;} 
		else {item_selected = (item_selected + 1) % itemCount;}
	}

	if (!destination) return;

	if ((encoderButtonZ1 != encoderButtonZ0) && (!encoderButtonZ0)) {
		pushState(state, item_selected);
		state = pages[item_selected].destination;
		visualScrollIndex = 0.00f;
		item_selected = 0; 
	}
}

//______________________________________________________________________mainFSM_____________________________________________________________________
void mainFSM() {
	switch (state) {
		case STATE_HOME:
			if (encoderDelta >= encoderCountPerIndent) {currentHeight --; lastEncoderCount += encoderCountPerIndent;}
  		if (encoderDelta <= -encoderCountPerIndent) {currentHeight ++; lastEncoderCount -= encoderCountPerIndent;}
			if ((button2Z1 != button2Z0) && (!button2Z0)) {activeGait = static_cast<Gaits>((activeGait + 1) % GAIT_ITEMS);}
			if ((button3Z1 != button3Z0) && (!button3Z0)) {activeMode = static_cast<Modes>((activeMode + 1) % MODE_ITEMS);}
			if ((encoderButtonZ1 != encoderButtonZ0) && (!encoderButtonZ0)) {pushState(state, item_selected); state = STATE_MENU; lastEncoderCount = encoderCount;}
			break;
		case STATE_MENU:
			handleScrollAndSelect(MENU, MENU_ITEMS);
			break;
		case STATE_CONFIG:
			if ((encoderButtonZ1 != encoderButtonZ0) && (!encoderButtonZ0)) {leg_selected = item_selected;}
			handleScrollAndSelect(CONFIG, CONFIG_ITEMS);
			break;
		case STATE_LEG:
			if ((encoderButtonZ1 != encoderButtonZ0) && (!encoderButtonZ0)) {
				joint_selected = item_selected; 
				jointOffset = LEG_OFFSET[leg_selected][joint_selected];
			}
			handleScrollAndSelect(LEG, LEG_ITEMS);
			break;
		case STATE_GAIT:
			handleScrollAndSelect(GAIT, GAIT_ITEMS, false);
			if((encoderButtonZ1 != encoderButtonZ0) && (!encoderButtonZ0)) {activeGait = static_cast<Gaits>(item_selected);}
			break;
		case STATE_MODE:
			handleScrollAndSelect(MODE, MODE_ITEMS, false);
			if((encoderButtonZ1 != encoderButtonZ0) && (!encoderButtonZ0)) {activeMode = static_cast<Modes>(item_selected);}
			break;
		case STATE_JOINT:
			if ((button1Z1 != button1Z0) && (!button1Z0)) {backPage();}
			if((encoderButtonZ1 != encoderButtonZ0) && (!encoderButtonZ0)) {
				LEG_OFFSET[leg_selected][joint_selected] = jointOffset;
				jointOffset = 0;
				backPage();
			}
			if(encoderDelta >= encoderCountPerIndent) {jointOffset --; lastEncoderCount += encoderCountPerIndent;}
  		if(encoderDelta <= -encoderCountPerIndent) {jointOffset ++; lastEncoderCount -= encoderCountPerIndent;}
			if (jointOffset > 60) jointOffset = 60;
			if (jointOffset < -60) jointOffset = -60;
		default:
			break;
	}
}

//_____________________________________________________doEncoderFSM__________________________________________________
void doEncoderFSM() {
  switch (encoderState) {
    case AB:
      if (!digitalRead(encoderA)) {encoderState = aB; encoderCount++;}
      if (!digitalRead(encoderB)) {encoderState = Ab; encoderCount--;}
      break;
    case aB:
      if (!digitalRead(encoderB)) {encoderState = ab; encoderCount++;}
      if (digitalRead(encoderA)) {encoderState = AB; encoderCount--;}
      break;
    case Ab:
      if (digitalRead(encoderB)) {encoderState = AB; encoderCount++;}
      if (!digitalRead(encoderA)) {encoderState = ab; encoderCount--;}
      break;
    case ab:
      if (digitalRead(encoderA)) {encoderState = Ab; encoderCount++;}
      if (digitalRead(encoderB)) {encoderState = aB; encoderCount--;}
      break;
    default:
      printf("Invalid state");
      break;
  }
}

//_____________________________________________________initializeEncoder__________________________________________________
void initializeEncoder() {
  if (digitalRead(encoderA) && digitalRead(encoderB)) encoderState = AB;
  if (!digitalRead(encoderA) && digitalRead(encoderB)) encoderState = aB;
  if (digitalRead(encoderA) && !digitalRead(encoderB)) encoderState = Ab;
  if (!digitalRead(encoderA) && !digitalRead(encoderB)) encoderState = ab;
}

//_______________________________________________________________________circularDelta__________________________________________________________________
float circularDelta(float from, float to, int size) {
    float delta = fmodf((to - from + size), size);
    if (delta > size / 2.0f) delta -= size;
    return delta;
}

//_______________________________________________________________________setupNav__________________________________________________________________
void setupNav(const char* heading) {
	// Back button and nav
	u8g2.setFont(u8g2_font_4x6_mf);
  u8g2.drawXBMP(0, 0, 26, 10, epd_bitmap_button_boarder);
  u8g2.drawStr(5, 7, "Back");
	u8g2.drawStr(29, 7, heading);
}

//_______________________________________________________________________drawActiveItem__________________________________________________________________
void drawActiveItem(int NUM_ITEMS, int activeItem) {
	// Number of items
  const int itemCount = NUM_ITEMS;

  // Calculate offset from currently selected item to active item, handling wrap-around
  int delta = activeItem - item_selected;
  if (delta > itemCount / 2) delta -= itemCount;
  if (delta < -itemCount / 2) delta += itemCount;

  // Define Y positions for items relative to selected item at 33
  // If your item_selected is at Y=33, and items are spaced by 18px:
  int yBase = 33;        // Y pos of selected item
  int ySpacing = 18;     // vertical spacing between items
  int yPos = yBase + delta * ySpacing;

  // Only draw if visible within your scrolling window (e.g., y between 15 and 51)
  if (yPos >= 15 && yPos <= 51) {u8g2.drawXBMP(5, yPos, 7, 7, epd_bitmap_selected_icon);}
}

//______________________________________________________________________drawPageItems_________________________________________________________
void drawPageItems(page* pages, int NUM_ITEMS, bool itemIcons = false, bool hexIcon = false) {
  float delta = circularDelta(visualScrollIndex, (float)item_selected, NUM_ITEMS);
  visualScrollIndex += 0.2f * delta;

  // Clamp within [0, MENU_ITEMS)
  if (visualScrollIndex < 0) visualScrollIndex += NUM_ITEMS;
  if (visualScrollIndex >= NUM_ITEMS) visualScrollIndex -= NUM_ITEMS;

	// Wrap scroll index to keep in [0, NUM_ITEMS)
	float wrappedScroll = fmodf(visualScrollIndex + NUM_ITEMS, NUM_ITEMS);
	int centerIndex = (int)wrappedScroll;
	float fractionalOffset = wrappedScroll - (float)centerIndex;

  // Draw the fixed selection border at CENTER_Y
	if (hexIcon) {u8g2.drawXBMP(1, (int)(CENTER_Y - 14), 77, 20, epd_bitmap_selection_boarder_hex);} 
	else {u8g2.drawXBMP(1, (int)(CENTER_Y - 14), 126, 20, epd_bitmap_selection_boarder);}
  
  // Draw visible items
  for (int i = -2; i <= 2; i++) {
    int index = (centerIndex + i + NUM_ITEMS) % NUM_ITEMS;
    float y = CENTER_Y + ySpacing * (i - fractionalOffset);

    if (y < NAVBAR_HEIGHT + 1 || y > SCREEN_HEIGHT - 1) continue;

    if (index == item_selected) {
			// This is the selected item — draw bold
			u8g2.setFont(u8g_font_7x14B);
			u8g2.drawStr(26, (int)roundf(y), pages[index].item);
			if (itemIcons && !hexIcon) {u8g2.drawXBMP(4, (int)roundf(y - 13), 16, 16, pages[index].icon);}
			if (hexIcon) {
				u8g2.drawXBMP(79, 12, 48, 48, epd_bitmap_hex_boarder);
				u8g2.drawXBMP(82, 15, 42, 42, pages[index].icon);
			}
		} else {
			// Non-selected
			u8g2.setFont(u8g_font_7x14);
			u8g2.drawStr(26, (int)roundf(y), pages[index].item);
			if (itemIcons) {u8g2.drawXBMP(4, (int)roundf(y - 13), 16, 16, pages[index].icon);}
		}
  }
}

//_______________________________________________________________________homePage__________________________________________________________________
void homePage() {
	// Switch 1
  u8g2.setFont(u8g2_font_4x6_mf);
  u8g2.drawXBMP(2, 1, 5, 7, (digitalRead(switch1) ? epd_bitmap_switch_up_icon : epd_bitmap_switch_down_icon));
  u8g2.drawStr(9, 7, "SW1");

	// Switch 2
  u8g2.setFont(u8g2_font_4x6_mf);
  u8g2.drawXBMP(35, 1, 5, 7, (digitalRead(switch2) ? epd_bitmap_switch_up_icon : epd_bitmap_switch_down_icon));
  u8g2.drawStr(42, 7, "SW2");

	// Switch 3
  u8g2.setFont(u8g2_font_4x6_mf);
  u8g2.drawXBMP(67, 1, 5, 7, (digitalRead(switch3) ? epd_bitmap_switch_up_icon : epd_bitmap_switch_down_icon));
  u8g2.drawStr(74, 7, "SW3");

	// Switch 4
  u8g2.setFont(u8g2_font_4x6_mf);
  u8g2.drawXBMP(98, 1, 5, 7, (digitalRead(switch4) ? epd_bitmap_switch_up_icon : epd_bitmap_switch_down_icon));
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
	char currentPhaseStr[10];
	sprintf(currentPhaseStr, "%d", currentPhase);
	u8g2.drawStr(36, 40, currentPhaseStr);
	
	// Height
	u8g2.setFont(u8g2_font_4x6_mf);
  u8g2.drawStr(3, 49, "Height");
	char currentHeightStr[10];
	sprintf(currentHeightStr, "%d", currentHeight);
	u8g2.drawStr(36, 49, currentHeightStr);

	// Menu Button
	u8g2.setFont(u8g2_font_4x6_mf);
  u8g2.drawXBMP(1, 53, 26, 10, epd_bitmap_button_boarder);
  u8g2.drawStr(5, 60, "Menu");

	// Hexapod
	u8g2.drawXBMP(72, 12, 48, 48, epd_bitmap_hex_boarder);
	u8g2.drawXBMP(75, 15, 42, 42, MODE[activeMode].icon);
}

//_______________________________________________________________________menuPage__________________________________________________________________
void menuPage() {
	setupNav("Menu");
	drawPageItems(MENU, MENU_ITEMS, true);
}

//_______________________________________________________________________configPage__________________________________________________________________
void configPage() {
	setupNav("Menu>Config");
	drawPageItems(CONFIG, CONFIG_ITEMS, false, true);
}

//_______________________________________________________________________legPage__________________________________________________________________
void legPage() {
	char navHeading[64]; // or large enough buffer
	snprintf(navHeading, sizeof(navHeading), "Menu>Config>%s", CONFIG[leg_selected].item);
	setupNav(navHeading);
	drawPageItems(LEG, LEG_ITEMS, false, true);

	u8g2.setFont(u8g_font_7x14);
	char currentOffsetStr[3];
	sprintf(currentOffsetStr, "%d", abs(LEG_OFFSET[leg_selected][item_selected]));
	u8g2.drawStr(94, 56, (LEG_OFFSET[leg_selected][item_selected] < 0) ? "-" : "+");
	u8g2.drawStr(102, 56, currentOffsetStr);

  if (item_selected == coxa) u8g2.drawXBMP(86, 37, 3, 3, epd_bitmap_joint_selected_icon);
  if (item_selected == femur) u8g2.drawXBMP(97, 37, 3, 3, epd_bitmap_joint_selected_icon);
  if (item_selected == tibia) u8g2.drawXBMP(108, 26, 3, 3, epd_bitmap_joint_selected_icon);
}

//_______________________________________________________________________gaitPage__________________________________________________________________
void gaitPage() {
	setupNav("Menu>Gait");
	drawPageItems(GAIT, GAIT_ITEMS, false, false);
	drawActiveItem(GAIT_ITEMS, activeGait);
}

//_______________________________________________________________________modePage__________________________________________________________________
void modePage() {
	setupNav("Menu>Mode");
	drawPageItems(MODE, MODE_ITEMS, false, true);
	drawActiveItem(MODE_ITEMS, activeMode);
}

//_______________________________________________________________________jointPage__________________________________________________________________
void jointPage() {
	char navHeading[64];  // Make sure buffer is big enough
	snprintf(navHeading, sizeof(navHeading), "Menu>Config>%s>%s", CONFIG[leg_selected].item, LEG[joint_selected].item);
	setupNav(navHeading);

	u8g2.drawXBMP(1, 22, 126, 20, epd_bitmap_selection_boarder);
	u8g2.drawBox(64 + ((jointOffset < 0) ? jointOffset : 0), 24, abs(jointOffset), 15);

	u8g2.setFont(u8g_font_7x14);
	char jointOffsetStr[4];
	sprintf(jointOffsetStr, "%d", abs(jointOffset));
	u8g2.drawStr(53, 56, (jointOffset < 0) ? "-" : "+");
	u8g2.drawStr(61, 56, jointOffsetStr);

	u8g2.setFont(u8g2_font_4x6_mf);
  u8g2.drawXBMP(0, 53, 26, 10, epd_bitmap_button_boarder);
  u8g2.drawStr(5, 60, "Save");
}

//_______________________________________________________________________readInputData__________________________________________________________________
void readInputData() {
	encoderDelta = encoderCount - lastEncoderCount;
  readButtonData();
  readStickData();
}

//_______________________________________________________________________readButtonData__________________________________________________________________
void readButtonData() {
  button1Z1 = button1Z0; button1Z0 = digitalRead(button1);
  button2Z1 = button2Z0; button2Z0 = digitalRead(button2);
  button3Z1 = button3Z0; button3Z0 = digitalRead(button3);
  button4Z1 = button4Z0; button4Z0 = digitalRead(button4);
	encoderButtonZ1 = encoderButtonZ0; encoderButtonZ0 = digitalRead(encoderButton);
}

//_______________________________________________________________________calibrateCenter__________________________________________________________________
int calibrateCenter(int pin) {
  long total = 0;
  const int samples = 20;

  for (int i = 0; i < samples; i++) {
    total += analogRead(pin);
    delay(10); // Small delay between samples
  }

  return total / samples;
}

//_______________________________________________________________________readStickData__________________________________________________________________
void readStickData() {
  int xRaw = analogRead(stick1X);
  int yRaw = analogRead(stick1Y);

  // Subtract center
  int xCentered = xRaw - centerX;
  int yCentered = yRaw - centerY;

  // Optional axis flip
  xCentered = -xCentered;

  // Scale to -128 to 127
  int x = (xCentered * 128L) / 2048;
  int y = (yCentered * 128L) / 2048;

  // Deadzone
  if (abs(x) < 8) x = 0;
  if (abs(y) < 8) y = 0;

  joy1X = constrain(x, -128, 127);
  joy1Y = constrain(y, -128, 127);
}