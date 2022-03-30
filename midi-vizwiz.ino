#include <U8g2lib.h>
#include <U8x8lib.h>
#include <MIDI.h>
#include <SPI.h>

byte pcBuffer[16];
byte sysEx[9]={0x41,0x10,0x42,0x12,0x40}; //excludes boundaries. array space for channel, upper byte, lower byte, checksum.
const byte gsResetSysEx[9]={0x41,0x10,0x42,0x12,0x40,0x00,0x7F,0x00,0x41};
byte gmResetSysEx[4]={0x7E,0x7F,0x09,0x01};
byte mt32GM[128]={0x0,0x1,0x2,0x4,0x5,0x4,0x5,0x3,0x10,0x11,0x12,0x12,0x13,0x13,0x14,0x15,0x6,0x6,0x6,0x7,0x7,0x7,0x8,0x8,0x3E,0x3F,0x3E,0x3F,0x26,0x27,0x26,0x27,0x5D,0x5E,0x36,0x62,0x61,0x63,0x59,0x65,0x66,0x60,0x44,0x50,0x51,0x53,0x55,0x50,0x30,0x31,0x30,0x2D,0x28,0x29,0x2A,0x2A,0x2B,0x2E,0x2E,0x18,0x19,0x1A,0x1B,0x68,0x20,0x20,0x21,0x22,0x24,0x25,0x23,0x23,0x49,0x49,0x48,0x48,0x4A,0x4B,0x41,0x40,0x42,0x43,0x47,0x47,0x44,0x45,0x46,0x16,0x38,0x38,0x39,0x39,0x3C,0x3C,0x3A,0x3D,0x3D,0xB,0xB,0xB,0xB,0x9,0xE,0xD,0xC,0x6B,0x6A,0x4D,0x4E,0x4E,0x4C,0x4C,0x2F,0x75,0x2F,0x76,0x76,0x74,0x73,0x77,0x73,0x70,0x37,0x7C,0x7B,0x7E,0x7A,0x7A};
//in SC-8820 mode, CC00 127 = MT32 map.

int gfxGraph[16];
int lastMap;
double lastTime;
double gfxTime;
boolean lastButton;
char *messages[]={"MIDI VizWiz!","GM Mode","GS Mode","MT-32 Mode","SC-55","SC-88","SC-88Pro","SC-8820","CM-64 C"};
int msgCoords[]={0,4,44,83,2,29,58,97,97};
char sysMsg[20]="";
int mt32Mode=0; //0 is off, 1 is 8820 in CM64 mode, 2 is mapped to any map via mt32GM.
boolean logEn=false;
boolean gfxEn=true;

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
  Serial.begin(9600);
  u8g2.setBusClock(8000000);
  u8g2.begin();
  u8g2.clearBuffer();
  gfxInit();
  gfxMessage(messages[0]);
  gfxSetMode(0);
  gsReset();
  setD70Map(4);
  gfxSetMap(3);
}

void loop() {

  if (MIDI.read()){
    if (MIDI.getType()==midi::ProgramChange){
      pcBuffer[MIDI.getChannel()-1]=MIDI.getData1();    
      if(mt32Mode==2){                                                  //if mt32 any map mode
        MIDI.sendProgramChange(mt32GM[pcBuffer[MIDI.getChannel()-1]], MIDI.getChannel()); //send similar sounding chanel
      }
    }
    
    if (MIDI.getType()==midi::SystemExclusive && MIDI.getSysExArray()[5]==0x20 && MIDI.getSysExArray()[6]==0x00 && MIDI.getSysExArray()[7]==0x00){
      for(int i=8;i<27;i++){
        sysMsg[i-8]=MIDI.getSysExArray()[i];
      }
      gfxMessage(sysMsg); //sysex message
    }
    
    if (MIDI.getType()==midi::NoteOn){
        gfxGraph[MIDI.getChannel()-1]=MIDI.getData2()/8; //21 pixels high
        gfxSetChannel(MIDI.getChannel()-1,1);
        gfxSetGraph(MIDI.getChannel()-1,gfxGraph[MIDI.getChannel()-1]);
    }
    if (MIDI.getType()==midi::NoteOff){
        gfxSetChannel(MIDI.getChannel()-1,0);
    }
    
  }
  
  for(int i=41;i<48;i++){
    if(!digitalRead(i) && lastButton==false && false){
      lastButton=true;
      lastTime=millis();
      {
        switch(i){
          case bGM:
            gfxSetMode(0);
            exitMT32Mode();
            gmReset();
            break;
          case bGS:
            gfxSetMode(1);
            exitMT32Mode();
            gsReset();
            break;
          case bMT32:
            gsReset();
            gfxSetMode(2);
            gfxSetMap(4);
            mt32Mode=1;
            setD70Map(4);
            break;
          case bSC55: 
            if(!(mt32Mode==1)){
              gfxSetMap(0);
              setD70Map(1);
            }
            break;
          case bSC88:
            if(!(mt32Mode==1)){
              gfxSetMap(1);
              setD70Map(2);
            }
            break;
          case bSC88p:
            if(!(mt32Mode==1)){
              gfxSetMap(2);
              setD70Map(3);
            }
            break;
          case b8820:
            if(mt32Mode==0){
              gfxSetMap(3);
            }
            else if((mt32Mode==1 && lastMap==4) || (mt32Mode==2 && lastMap<4)){         //if in CM-64 mode already, change to 8820 mode
              gfxSetMap(3);             //set 8820 gfx map
              mt32Mode=2;
            }
            else if(mt32Mode==2){       //if in MT32->8820 mode already -> change to CM-64 mode
              gfxSetMap(4);
              mt32Mode=1;
            }      
            setD70Map(4); 
            break;
        }
      }
    }
  }

  if(millis()-lastTime>1000){  //allows new button presses after 1 sec
      lastButton=false;
      lastTime=millis();
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
}    

void gfxInit(){
  u8g2.clearBuffer();
  gfxMapInit();
  gfxModeInit();
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
    //u8g2.setDrawColor(0);
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

void setD70Map(int map){
    for(int j=1;j<17;j++){
      sysEx[5]=0x40+j;
      sysEx[6]=0x01;
      sysEx[7]=map;
      sysEx[8]=128-((sysEx[4]+0x40+j+map+1)%128);
      MIDI.sendSysEx(9,sysEx,false);
    }
    for(int k=1;k<17;k++){
      if(mt32Mode==1){
        MIDI.sendControlChange(0,127,k);
        MIDI.sendProgramChange(pcBuffer[k-1],k);
      }
      else if(mt32Mode==2){
        MIDI.sendControlChange(0,0,k);
        MIDI.sendProgramChange(mt32GM[pcBuffer[k-1]],k);        
      }
      else{
        MIDI.sendControlChange(0,0,k);
        MIDI.sendProgramChange(pcBuffer[k-1],k);
      }
    }
    lastMap=map;    
}

void exitMT32Mode(){
  mt32Mode=0;
  gfxSetMap(lastMap-1);
  for(int k=0;k<16;k++){
   MIDI.sendControlChange(0,0,k);
   MIDI.sendProgramChange(pcBuffer[k-1],k);
  }  
}

void gsReset() {
  MIDI.sendSysEx(9,gsResetSysEx,false);
  reSendPCs();
}

void gmReset(){
  MIDI.sendSysEx(4,gmResetSysEx,false);
  reSendPCs();
}

void reSendPCs(){
  for(int k=0;k<16;k++){
    MIDI.sendProgramChange(pcBuffer[k-1],k);
  } 
}
