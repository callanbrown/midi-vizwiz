#include <U8g2lib.h>
#include <U8x8lib.h>
#include <MIDI.h>
#include <SPI.h>

byte pcBuffer[16];
int gfxGraph[16];
int lastMap;
double lastTime;
double gfxTime;
boolean lastButton;
char *messages[]={"MIDI VizWiz!","GM Mode","GS Mode","MT-32 Mode","SC-55","SC-88","SC-88Pro","SC-8820","CM-64 C"};
char *notesSharps[]={"A","A#","B","C","C#","D","D#","E","F","F#","G","G#"};
char *notesFlats[]={"A","Bb","B","C","Db","D","Eb","E","F","Gb","G","Ab"};
int msgCoords[]={0,4,44,83,2,29,58,97,97};
int singleBuffer[8]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
boolean logEn=false;
boolean gfxEn=true;
int mode;
int bufferLast = 0;

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);
//d70 buddy U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, 31, 30, 29);
U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, 9,8,7);
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

const int bSC55=47;
const int bSC88=46;
const int bSC88p=45;
const int b8820=44;
const int bGM=43;
const int bGS=42;
const int bMT32=41;
void setup() {
  Serial.begin(115200);
  while(!Serial){}
  Serial.println("mode set to single");

  pinMode(bGM, INPUT_PULLUP);
  pinMode(bGS, INPUT_PULLUP);
  pinMode(bMT32, INPUT_PULLUP); 
  pinMode(bSC55, INPUT_PULLUP);
  pinMode(bSC88, INPUT_PULLUP);
  pinMode(bSC88p, INPUT_PULLUP);
  pinMode(b8820, INPUT_PULLUP);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOn();
  MIDI.setThruFilterMode(midi::Thru::Full);
  lastTime=millis();
  gfxTime=millis();
  u8g2.setBusClock(8000000);
  u8g2.begin();
  u8g2.clearBuffer();
  gfxSingleChannelInit();
  mode=1; //single channel
}

void loop() {
  if(millis()-lastTime>1000){  //allows new button presses after 1 sec
      lastButton=false;
      lastTime=millis();
  }  
  switch(mode){
    case 0:
      if (MIDI.read()){     
        if (MIDI.getType()==midi::NoteOn){
            gfxGraph[MIDI.getChannel()-1]=MIDI.getData2()/8; //21 pixels high
            gfxSetChannel(MIDI.getChannel()-1,1);
            gfxSetGraph(MIDI.getChannel()-1,gfxGraph[MIDI.getChannel()-1]);
        }
        if (MIDI.getType()==midi::NoteOff){
            gfxSetChannel(MIDI.getChannel()-1,0);
        }
      }  
      if(millis()-gfxTime>50){    //update screen each 50ms
        for(int i=0;i<16;i++){
          if(gfxGraph[i]){
            gfxGraph[i]=gfxGraph[i]-1;
            gfxSetGraph(i,gfxGraph[i]);
          }
        }
        u8g2.sendBuffer();
        gfxTime=millis();
      }
      break;
    case 1: //Single Mode
      if(MIDI.read()){
        if (MIDI.getType()==midi::NoteOn){
          singleAddNote();
          //When a NoteOn occurs, show the note.
          //If another NoteOn occurs, add the note by shuffling
          
          //gfxGraph[MIDI.getChannel()-1]=MIDI.getData2()/8; //21 pixels high
          //gfxSetChannel(MIDI.getChannel()-1,1);
          //gfxSetGraph(MIDI.getChannel()-1,gfxGraph[MIDI.getChannel()-1]);
        }
        if (MIDI.getType()==midi::NoteOff){
            //gfxSetChannel(MIDI.getChannel()-1,0);
        }
      }
    break;
  }
}    

void singleAddNote(){
  //Sort note into buffer
  //Redraw notes
  singleSortNotes();
  
}

void singleSortNotes(){
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_micro_tr);
  u8g2.drawStr(10,10,"TEST");
  u8g2.sendBuffer();
  char *logMessage = "singleBuffer: ";
  
  for(int i=0; i<bufferLast+1; i++){
    if (MIDI.getData1() > singleBuffer[i]){
      singleBuffer[i] = MIDI.getData1();
      bufferLast++;
      break;
    }
    if(bufferLast == 8){
      bufferLast = 7;
    }
  }
  
  Serial.print(logMessage);
  for(int i=0; i<8; i++){
    Serial.print(notesSharps[(singleBuffer[i] - 0x15) % 12]);
    Serial.print(" ");
  }
  Serial.print("bf: ");
  Serial.print(bufferLast);
  Serial.print('\n');
}

void gfxOmniInit(){
  u8g2.clearBuffer();
  gfxMapInit();
  gfxModeInit();
}

void gfxSingleChannelInit(){
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.drawRFrame(0,11,128,33,2);
  gfxMessage("Single Mode");
  u8g2.sendBuffer(); 
  gfxMessage("Another");
  u8g2.sendBuffer();
}

void gfxMapInit(){
  u8g2.setDrawColor(0);
  u8g2.drawBox(0,55,128,9);
  u8g2.setDrawColor(1);
  u8g2.drawRFrame(0,55,128,9,2);
  u8g2.setFont(u8g2_font_micro_tr);
  for(int i=0;i<4;i++){
    u8g2.drawStr(msgCoords[i+4]+1,62,messages[i+4]);
  }
}

void gfxModeInit(){
  u8g2.setDrawColor(0);
  u8g2.drawBox(0,45,128,9);
  u8g2.setDrawColor(1);
  u8g2.drawRFrame(0,45,128,9,2);
  u8g2.setFont(u8g2_font_micro_tr);
  u8g2.drawStr(5,52,"GM Mode   GS Mode");
  u8g2.drawStr(84,52,"MT-32 Mode");
  u8g2.drawStr(3,42,"1 2 3 4 5 6 7 8 9 A B C D E F G");
  u8g2.drawRFrame(0,11,128,33,2);
}

void gfxMessage(char *message){
  //u8g2.clearBuffer();
  u8g2.setDrawColor(0);
  u8g2.drawBox(0,0,128,10);
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_t0_11b_tf);
  u8g2.drawStr(0,8,message);
}

void gfxSetGraph(int channel, int vel){ //sets height of channel bar based on vel
  if(gfxEn){
    int cOffset=3+((channel)*8);
    u8g2.setDrawColor(0);
    u8g2.drawBox(cOffset-1,12,5,22);
    u8g2.setDrawColor(1);
    u8g2.drawBox(cOffset-1,34-vel,5,vel);
  }
}

void gfxSetChannel(int channel, int state){ //highlights or not channel number
  if(gfxEn){
    int cOffset=3+((channel)*8);
    u8g2.setDrawColor(state);
    u8g2.drawBox(cOffset-1,36,5,7);
    u8g2.setDrawColor(!state);
    const char strChannel[]="123456789ABCDEFG";
    char p[]="";
    strncpy(p,&strChannel[channel],1);
    u8g2.setFont(u8g2_font_micro_tr);
    u8g2.drawStr(cOffset,42,p);
  }
}

void gfxSetMap(int map){
  gfxMapInit();
  u8g2.setDrawColor(1);
  u8g2.drawBox(msgCoords[map+4],56,strlen(messages[map+4])*4+1,7);
  u8g2.setDrawColor(0);
  u8g2.setFont(u8g2_font_micro_tr);
  u8g2.drawStr(msgCoords[map+4]+1,62,messages[map+4]);
}

void gfxSetMode(int mode){
  gfxModeInit();
  u8g2.setDrawColor(1);
  u8g2.drawBox(msgCoords[mode+1],46,strlen(messages[mode+1])*4+1,7);
  u8g2.setDrawColor(0);
  u8g2.setFont(u8g2_font_micro_tr);
  u8g2.drawStr(msgCoords[mode+1]+1,52,messages[mode+1]);
}
