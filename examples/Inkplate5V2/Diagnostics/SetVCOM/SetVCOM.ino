/**
 **************************************************
 *
 * @file        SetVCOM.ino
 * @brief       WARNING! - VCOM voltage is written in EEPROM, which means it can be set a limited number of times,
 *              so don't run this sketch repeateadly! VCOM should be set once and then left as is.
 *
 * For info on how to quickly get started with Inkplate 5 V2 visit
 * https://soldered.com/documentation/inkplate/5/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/


#include <EEPROM.h>
#include <Inkplate-LVGL.h>
#include <Wire.h>

Inkplate inkplate(INKPLATE_3BIT); // Create an object on Inkplate library and also set the grayscale to 3bit.

double currentVCOM;         // Stores the current VCOM value stored on EEPROM
const int EEPROMAddress=0;  // Should leave the address as it is for correct EEPROM reading later
double vcomVoltage;

double readPanelVCOM();
double getVCOMFromSerial(double *_vcom);
uint8_t writeVCOMToEEPROM(double v);
static inline int display_width();
static inline int display_height();
void displayTestImage();
void writeReg(uint8_t reg, float data);
uint8_t readReg(uint8_t reg);

void setup() 
{
  Serial.begin(115200);     //Start serial at 115200 baud
  EEPROM.begin(512);        //Initialize EEPROM
  Wire.begin();             //Initialize I2C buss
  inkplate.begin();          //Initialize the Inkplate 
  Serial.println("The optimal VCOM Voltage for your Inkplate's panel can sometimes be found written on the flat cable connector");
  Serial.println("Write VCOM voltage from epaper panel. \r\nDon't forget negative (-) sign!\r\nUse dot as the decimal point.\r\nFor example -1.23\n");
  displayTestImage();
}

void loop() 
{  
  if (Serial.available()){
    //Serial.println("Enter VCOM value, it must be [-5, 0]");
    do{
      getVCOMFromSerial(&vcomVoltage);
      Serial.println(vcomVoltage, 2);
      if(vcomVoltage < -5.0 || vcomVoltage > 0.0){
        Serial.println("VCOM out of range! [-5, 0]");
      }
    }while(vcomVoltage <-5.0 || vcomVoltage > 0.0);

    //Program the panel EEPROM
    inkplate.internalIO.pinMode(6, INPUT_PULLUP);
    if(writeVCOMToEEPROM(vcomVoltage)){
      EEPROM.put(EEPROMAddress, vcomVoltage);
      EEPROM.commit();
    }
    lv_obj_clean(lv_scr_act());
    displayTestImage();
  }
}

double readPanelVCOM()
{
  delay(10); //Wake up TPS65186 so registers respond
  uint8_t vcomL=readReg(0x03); // Read low 8 bits from register 0x03
  uint8_t vcomH = readReg(0x04) & 0x01; // Read full byte, mask off all but bit 0 (MSB)
  delay(10); //Power down driver
  int raw=(vcomH << 8) | vcomL; //Value between 0 - 511
  return -(raw/100.0);
}

double getVCOMFromSerial(double *_vcom)
{
  double vcom=0;
  char buff[32];
  unsigned long start;
  while (!Serial.available());
  start=millis();
  int idx=0;
  while ((millis()-start)<500 && idx<sizeof(buff)-1){
    if(Serial.available()){
      char c=Serial.read();
      buff[idx++]=c;
      start=millis();
    }
  }
  buff[idx]='\0';
  sscanf(buff, "%lf", &vcom);
  *_vcom=vcom;
  return vcom;
}

uint8_t writeVCOMToEEPROM(double v)
{
  //Build a 9-bit raw value (0 - 511)
  int raw=int(abs(v)*100)&0x1FF;
  uint8_t lsb=raw & 0xFF;
  uint8_t msb=(raw >> 8)&0x01;
  
  inkplate.einkOn();
  delay(10);

  writeReg(0x03, lsb);
  uint8_t r4=readReg(0x04)&~0x01;
  writeReg(0x04, r4 | msb);
  writeReg(0x04, (r4 | msb) | (1 << 6));
  while ( inkplate.internalIO.digitalRead(6) ) {
    delay(1);
  }
  readReg(0x07);  // clear interrupt flag
  writeReg(0x03, 0);
  writeReg(0x04, 0);
  inkplate.einkOff(); // WAKEUP low
  delay(10);
  inkplate.einkOn();  // WAKEUP high
  delay(10);
  uint8_t vL = readReg(0x03);
  uint8_t vH = readReg(0x04) & 0x01;
  int check = (vH << 8) | vL;
  if (check != raw) {
    Serial.printf("Verification failed: got %d, want %d\n", check, raw);
    return 0;
  }
  Serial.println("VCOM EEPROM PROGRAMMING OK");
  return 1;
}

void writeReg(uint8_t reg, float data)
{
  Wire.beginTransmission(0x48);
  Wire.write(reg);
  Wire.write((uint8_t)data);
  Wire.endTransmission();
}

uint8_t readReg(uint8_t reg)
{
  Wire.beginTransmission(0x48);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)0x48, (uint8_t)1);
  return Wire.read();
}

static inline int display_width()
{
  lv_display_t *disp = lv_disp_get_default();
  return lv_display_get_horizontal_resolution(disp);
}

static inline int display_height()
{
  lv_display_t *disp = lv_disp_get_default();
  return lv_display_get_vertical_resolution(disp);
}

void displayTestImage() 
{
  inkplate.clearDisplay();
  currentVCOM = readPanelVCOM();

  /* Show text on the screen */
  lv_obj_t *label = lv_label_create(lv_screen_active());
  lv_label_set_text(label, "Current VCOM: ");
  lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_text_font(label,  &lv_font_montserrat_28, 0);
  lv_obj_align(label, LV_ALIGN_TOP_LEFT, 5, 5);
  
  /* Display value */
  lv_obj_t *label2 = lv_label_create(lv_scr_act());
  lv_label_set_text(label2, String(currentVCOM, 2).c_str());
  lv_obj_set_style_text_color(label2, lv_color_hex(0x000000), 0);
  lv_obj_set_style_text_font(label2, &lv_font_montserrat_28, 0);
  // Position label2 just to the right of label1
  lv_obj_align_to(label2, label, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

  for (int i = 0; i < 8; i++) 
  {
    int x = (display_width() / 8) * i;

    uint8_t v = (i * 255) / 7;
    lv_color_t color = lv_color_make(v, v, v);

    lv_obj_t *rect = lv_obj_create(lv_scr_act());
    lv_obj_set_size(rect, display_width() / 8, display_height());
    lv_obj_set_pos(rect, x, 40);
    lv_obj_set_style_bg_color(rect, color, 0);
    lv_obj_set_style_border_width(rect, 0, 0);  // Remove 'padding'
    lv_obj_set_style_radius(rect, 0, 0);        // No round corners
  }
  // Update the display
  lv_tick_inc(50);
  lv_timer_handler();
  inkplate.display();
}