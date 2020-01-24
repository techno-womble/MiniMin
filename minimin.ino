 
/****************************************************************************     
 minimin - a minimal Theremin style instrument
  
 Created 15  Dec 2019
 Modified 23 Jan 2020
 V1.2
 By John Potter (techno-womble)
*********************************************************************************/

//   download Mozzi library from https://sensorium.github.io/Mozzi/ 

#include <MozziGuts.h>
#include <Oscil.h> 
#include <RollingAverage.h>
#include <tables/triangle_valve_2048_int8.h>              // oscillator wavetable data

Oscil <TRIANGLE_VALVE_2048_NUM_CELLS, AUDIO_RATE> osc(TRIANGLE_VALVE_2048_DATA);

#define CONTROL_RATE 128         

const boolean stepMode = false;

/*int notes[] = {131,139,147,156,165,175,185,196,208,220,233.246,       // chromatic scale
               262,277,294,311,330,349,370,392,415,440,466,494,
               523,554,587,622,659,698,739,783,830,880,932,988,
               1046}; */

/*int notes[] = {131,131,147,147,165,175,175,196,196,220,220.246,         // major scale
               262,262,294,294,330,349,349,392,392,440,440,494,
               523,523,587,587,659,698,698,783,783,880,880,988,
               1046} */
               
/*int notes[] = {131,131,147,147,156,175,175,196,196,220,220.246,         // minor scale
               262,262,294,294,311,349,349,392,392,440,440,494,
               523,523,587,587,622,698,698,783,783,880,880,988,
               1046}; */

int notes[] = {131,131,131,156,156,175,175,196,196,196,233.233,       // cminor pentatonic
               262,262,262,311,311,349,349,392,392,392,466,466,
               523,523,523,622,622,698,698,783,783,783,932,932,
               1046}; 

RollingAverage <int, 4> pAverage;             // how_many_to_average has to be power of 2
RollingAverage <int, 8> vAverage;            
int averaged;

const int volOut = 10;                        //pins 10 & 11 connected to volume distance sensor
const int volIn =11;
const int pitchOut=12;                        //pins 12 & 13 connected to pitch distance sensor
const int pitchIn=13;
const int pitchLowThreshold = 450;           // define the useable range of the pitch sensor
const int pitchHighThreshold = 50;    
const int volLowThreshold = 200;             // define the useable range of the volume sensor
const int volHighThreshold = 50; 
const int lowestFreq = 131;                  // C3 in Hz
const int highestFreq = 1046 ;               // C6 in hz

int pitchTimeOut = pitchLowThreshold * 8;
int volTimeOut = volLowThreshold * 8;
int smoothVol;
int vol = 0;

void setup(){
  pinMode(pitchOut,OUTPUT);                   // initialise pitch sensor pins
  pinMode(pitchIn,INPUT);
  digitalWrite(pitchOut,LOW);

  pinMode(volOut,OUTPUT);                   // initialise volume sensor pins
  pinMode(volIn,INPUT);
  digitalWrite(volOut,LOW);
  
  Serial.begin(9600);
  startMozzi(CONTROL_RATE);                   // set the control rate
  osc.setFreq(110); // set the frequency
}


int nearest(int x)
{
  int idx = 0; // by default near first element
  int distance = abs(notes[idx] - x);
  for (int i=1; i<37; i++)
  {
    int d = abs(notes[i] - x);
    if (d < distance)
    {
      idx = i;
      distance = d;
    }
  }
  return idx;
}
  
void updateControl(){
  int dur;
  int vDur;
  int freq;
  int targetFreq;
  int vol;
  long distance;
  int jitter;

  digitalWrite(pitchOut,HIGH);
  delayMicroseconds(10);
  digitalWrite(pitchOut,LOW);

  dur=pulseIn(pitchIn,HIGH,pitchTimeOut);
  if ( dur < 5 ){
    freq = lowestFreq;
    vol = vol - 4;                             // fade audio out if no echo detected 
    if (vol < 0) vol = 0;
  } else
  {
     vol = vol + 4;                            // fade in if echo setected
     if (vol > 255) vol = 255;
     distance = dur / 6;
     if ( distance >= pitchLowThreshold) distance = pitchLowThreshold;
     if ( distance < pitchHighThreshold) distance = pitchHighThreshold;
     freq = map(distance,pitchHighThreshold,pitchLowThreshold,highestFreq,lowestFreq);
  }   
  jitter = (random(-5,5)); 
  if (stepMode == true) {

     targetFreq = pAverage.next(notes[nearest(freq)]);
     osc.setFreq(targetFreq + jitter); 
  } else                                           

  {
     averaged = pAverage.next(freq); 
     osc.setFreq(averaged + jitter); 
  }

  digitalWrite(volOut,HIGH);
  delayMicroseconds(10);
  digitalWrite(volOut,LOW);

  vDur=pulseIn(volIn,HIGH,volTimeOut);
  if ( vDur < 5 ){
    vol = vol - 4;                             // fade audio out if no echo detected 
    if (vol < 0) vol = 0;
     distance = 200;
  } else
  {
      distance = vDur / 6;
      if ( distance > volLowThreshold) distance = volLowThreshold;
      if ( distance < volHighThreshold) distance = volHighThreshold;
   
  }
 vol = map(distance,volHighThreshold,volLowThreshold,256,0);
  smoothVol = vAverage.next(vol);  

}

int updateAudio(){
  return (osc.next() * smoothVol) >> 8;         // return an int signal centred around 0
}

void loop(){
  audioHook();                 // required here
}



