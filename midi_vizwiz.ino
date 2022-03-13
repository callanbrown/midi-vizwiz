#include <U8g2lib.h>
#include <SPI.h>
#include <MIDI.h>
U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, 9, 8, 7);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);


void setup() {
  // put your setup code here, to run once:
  u8g2.setBusClock(8000000);
  u8g2.begin();
  u8g2.setDrawColor(1);
  u8g2.drawRFrame(0,55,128,9,2);
      u8g2.sendBuffer();
MIDI.begin(MIDI_CHANNEL_OMNI);
}

void loop() {
  // put your main code here, to run repeatedly:
if (MIDI.read()){
  if (MIDI.getType()==midi::NoteOn){
  u8g2.drawRFrame(10,55,128,9,2);
    u8g2.sendBuffer();
  }
  
}
}
