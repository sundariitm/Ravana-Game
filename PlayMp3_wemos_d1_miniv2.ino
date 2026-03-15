// this example will play a track and then 
// every five seconds play another track
//
// it expects the sd card to contain these three mp3 files
// but doesn't care whats in them
//
// sd:/mp3/0001.mp3
// sd:/mp3/0002.mp3
// sd:/mp3/0003.mp3
// This is a Ravana game where player is supporting Lord Rama by trying to kill Ravana.
// There is also a digital input connected with a push button. This push button is triggered when player hits the heart of Ravana 
//            with arrow/bow.  Once player hits 3  times he wins.
//  There are 3 MP3 tracks in a SD card connected with DFMiniMP3 player module.
//        track 1 is the Demonic laugh that keeps repeating
//        track 2 is a Cry of ravana once hit by a bow
//        track 3 is victory song played after hit the 3rd time.
//  To reset the game same input switch is hit once by hand and the Demoic laugh will begin for the next game
//  Board : Lolin Wemons D1 R2 & Mini  

#include <DFMiniMp3.h>
#include <DfMp3Types.h>
#include <SoftwareSerial.h>

#define MAX_WIN_CNT       2
#define MAX_VOLUME_SET    30

// Software serial UART pins
const int SS_UART_RX_PIN  = 4;    // D2 pin Wroom2 
const int SS_UART_TX_PIN  = 5;    // D1 pin Wroom2 
const int  arrowHitpin    = 0;    // D3 pin in Wroom2 Pulled high in Flash button
//const int  builtinledPin  = 16; 
const int  restartpin     = 13;    // D7 pin in D1 mini v2

int   targethit = 0;
int   trackToplay = 2; // 3- JaiSriRam  2 - laugh,  1 - cry
int   winCnt = 0;
int   gameOver = 0;
int   ledState = 0;


// forward declare the notify class, just the name
class Mp3Notify; 

// define a handy type using serial and our notify class
//
//typedef DFMiniMp3<HardwareSerial, Mp3Notify> DfMp3; 

// instance a DfMp3 object, 
//
//DfMp3 dfmp3(Serial1);

// Some arduino boards only have one hardware serial port, so a software serial port is needed instead.
// comment out the above definitions and use these
SoftwareSerial secondarySerial(SS_UART_RX_PIN , SS_UART_TX_PIN ); // RX  (D2 pin), TX (D1 pin) 
typedef DFMiniMp3<SoftwareSerial, Mp3Notify> DfMp3;
DfMp3 dfmp3(secondarySerial);

// implement a notification class,
// its member methods will get called 
//
class Mp3Notify
{
public:
  static void PrintlnSourceAction(DfMp3_PlaySources source, const char* action)
  {
    if (source & DfMp3_PlaySources_Sd) 
    {
        Serial.print("SD Card, ");
    }
    if (source & DfMp3_PlaySources_Usb) 
    {
        Serial.print("USB Disk, ");
    }
    if (source & DfMp3_PlaySources_Flash) 
    {
        Serial.print("Flash, ");
    }
    Serial.println(action);
  }
  static void OnError([[maybe_unused]] DfMp3& mp3, uint16_t errorCode)
  {
    // see DfMp3_Error for code meaning
    Serial.println();
    Serial.print("Com Error ");
    Serial.println(errorCode);
  }
  static void OnPlayFinished([[maybe_unused]] DfMp3& mp3, [[maybe_unused]] DfMp3_PlaySources source, uint16_t track)
  {
    Serial.print("Play finished for #");
    Serial.println(trackToplay);  
    
    dfmp3.playMp3FolderTrack(trackToplay);  // Laugh
    //dfmp3.playMp3FolderTrack(3);  // Cry
  
  }
  static void OnPlaySourceOnline([[maybe_unused]] DfMp3& mp3, DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "online");
  }
  static void OnPlaySourceInserted([[maybe_unused]] DfMp3& mp3, DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "inserted");
  }
  static void OnPlaySourceRemoved([[maybe_unused]] DfMp3& mp3, DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "removed");
  }
};


void ICACHE_RAM_ATTR ISR() {
    targethit += 1;
}


void setup() 
{
  DfMp3_PlaybackMode playmode;

  playmode = DfMp3_PlaybackMode_SingleRepeat;
  Serial.begin(115200);
  //pinMode (builtinledPin ,OUTPUT);
  pinMode(arrowHitpin, INPUT_PULLUP );
  pinMode(restartpin, INPUT_PULLUP );
  
  attachInterrupt(digitalPinToInterrupt(arrowHitpin), ISR, FALLING);
  Serial.println("initializing...");
  
  dfmp3.begin();
  // for boards that support hardware arbitrary pins
  // dfmp3.begin(10, 11); // RX, TX

  // during development, it's a good practice to put the module
  // into a known state by calling reset().  
  // You may hear popping when starting and you can remove this 
  // call to reset() once your project is finalized
  dfmp3.reset();

  uint16_t version = dfmp3.getSoftwareVersion();
  Serial.print("version ");
  Serial.println(version);

  uint16_t volume = dfmp3.getVolume();
  Serial.print("volume ");
  Serial.println(volume);
  dfmp3.setVolume(MAX_VOLUME_SET);
  
  uint16_t count = dfmp3.getTotalTrackCount(DfMp3_PlaySource_Sd);
  Serial.print("files ");
  Serial.println(count);
  
  Serial.println("starting...");

  dfmp3.setPlaybackMode(playmode);  // Repeat Single Track
  // start the first track playing
  // dfmp3.playMp3FolderTrack(1);  // sd:/mp3/0001.mp3
  dfmp3.playMp3FolderTrack(trackToplay); 

  
}

void waitMilliseconds(uint16_t msWait)
{
  uint32_t start = millis();
  
  while ((millis() - start) < msWait)
  {
    // if you have loops with delays, its important to 
    // call dfmp3.loop() periodically so it allows for notifications 
    // to be handled without interrupts
    dfmp3.loop(); 
    delay(1);
  }
}

void restartGame () {
   // Reset all variables and start over game
    targethit = 0;
    gameOver = 0;
    winCnt = 0;
    trackToplay = 2; // Demon laugh
    ledState = false;
    //digitalWrite ( builtinledPin, ledState );
    dfmp3.playMp3FolderTrack(trackToplay); 
}


void loop() 
{
  waitMilliseconds(100);
   // start the first track playing
  //dfmp3.playMp3FolderTrack(1);  // sd:/mp3/0001.mp3
  //dtmp3.setPlaybackMode (2);  // Repeat Single Track
  // dfmp3.playMp3FolderTrack(3);  // Cry
  // dfmp3.playMp3FolderTrack(1);  // Laugh


  if (targethit) {
    ledState = ( ledState == 1) ? 0 : 1; 
    //digitalWrite ( builtinledPin, ledState );
    //interrupts();
   }
   
  // targethit_n = digitalRead (arrowHitpin);
  // When targethit 3 times  - Jai SriRam to be played
  // When targethit <3 times - Demon cry played once then Demon laugh
  if (targethit && !gameOver) {
    targethit = 0; 
    if (winCnt < MAX_WIN_CNT) {
      winCnt += 1;
      trackToplay = 1;
      dfmp3.playMp3FolderTrack(trackToplay);  // Demon cry
      waitMilliseconds(6100);  // When demon cry is happening just freeze
      trackToplay = 2;
      dfmp3.playMp3FolderTrack(trackToplay);  // Demon laugh
    } else if (winCnt >= MAX_WIN_CNT)  {
        gameOver = 1;
        trackToplay = 3;
        dfmp3.playMp3FolderTrack(trackToplay);  // Jai Sri Ram track
        waitMilliseconds(4000);  // After Game over play Jai Sri Ram track and just freeze for few seconds
    }
    targethit = 0;
  } else if (gameOver && targethit) {
      restartGame();
    }

  //if (!digitalRead(restartpin)) {
  //    restartGame();
  //}
  
}
