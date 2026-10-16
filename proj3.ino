/*
CHANGE LOG:
  - Created the round-robin loop
  - Incorporated 2 tasks [task_blink, task_serial]
  - used #define for LED pinouts and messaging
  - will now include myMillis.c
*/
#include "myMillis.h"
#define LED1 2 // Pin for first LED
#define LED2 3 // Pin for second LED
#define CHOOSE_STATEMENT "Choose an LED [1 or 2]: \n"
#define BUF_SIZE 16

// rates for the LEDs respectively
unsigned int rate1 = 0;
unsigned int rate2 = 0;

// define target LED
int targetLED;

// defining input modifier
char inputBuffer[BUF_SIZE];
uint8_t bufInd = 0;
// save previous blink times; default is 0
unsigned long previousMillis1 = 0;
unsigned long previousMillis2 = 0;

// save previous state; default is low
int ledState1 = LOW;
int ledState2 = LOW;

bool awaitingInterval = false;

/*** START TASK MANAGEMENT ***/
void task_blink();
void task_serial();

typedef struct{
  unsigned long Interval;
  unsigned long LastTick;
  void (*Func)(void); // point this when a task finishes
}TaskType;

// create our task block
static TaskType Tasks[]{
  {1, 0, task_blink},
  {5, 0, task_serial}, // constant task
  // NOTE: RATES CAN BE CHANGED!
};

uint8_t Tsk_GetNumTasks(void) {return sizeof(Tasks) / sizeof(*Tasks);};

/*** END TASK MANAGEMENT***/

void setup() {
  myInit();
  // set pinModes to output
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED1, ledState1);
  digitalWrite(LED2, ledState2);
  Serial.begin(9600);

  Serial.println("\nSystem Ready");
  Serial.println(CHOOSE_STATEMENT);

}

void loop() {
  
  // we can track current millis for movement
  const uint8_t NumTasks = Tsk_GetNumTasks();
  unsigned long currentMillis = myMillis();

  for(uint8_t i = 0; i < NumTasks; i++){
    if(Tasks[i].Interval == 0){
      Tasks[i].Func();
    }else if(currentMillis - Tasks[i].LastTick >= Tasks[i].Interval){
      Tasks[i].LastTick = currentMillis; // update the last tick
      Tasks[i].Func();
    } // if & if-else block
  } // end for(int i = 0; ... i++)
}

void task_blink(){
  unsigned long currentMillis = myMillis();

  // Check for the first LED
  if(rate1 > 0){
    unsigned long interval1 = rate1 / 2;
    if (currentMillis - previousMillis1 >= interval1) {
      previousMillis1 = currentMillis; // Save the last blink time
      ledState1 = !ledState1;
      digitalWrite(LED1, ledState1);
    }
  }else{
    digitalWrite(LED1, LOW); // off if rate is 0
  }

  // Check for the second LED
  if(rate2 > 0){
    unsigned long interval2 = rate2 / 2;
    if (currentMillis - previousMillis2 >= interval2) {
      previousMillis2 = currentMillis; // Save the last blink time
      ledState2 = !ledState2;
      digitalWrite(LED2, ledState2);
    }
  }else{
    digitalWrite(LED2, LOW);
  }
} // end task_blink

void task_serial(){
  while(Serial.available() > 0){
    char ch = Serial.read();

    if(ch == '\n' || ch == '\r'){
      if(bufInd > 0){
        inputBuffer[bufInd] = '\0'; // clear C string for usage
        
        if(!awaitingInterval){
          targetLED = atoi(inputBuffer);
          if(targetLED == 1 || targetLED == 2){
            Serial.println(targetLED);
            awaitingInterval = true;
            Serial.println("Choose an interval in ms:\n");
          }else{
            Serial.println("LED Invalid, try again\n");
            Serial.println(CHOOSE_STATEMENT);
          }
        }else{
          // choosing an interval
          int interval = atoi(inputBuffer);
          Serial.println(interval);
          if(targetLED == 1) rate1 = interval;
          if(targetLED == 2) rate2 = interval;

          // start the reset
          awaitingInterval = false;
          Serial.println(CHOOSE_STATEMENT);
        }
        bufInd = 0; // reset the value for the buffer index
      }
    }else{
      // method to prevent buffer overflow
      if(bufInd < BUF_SIZE - 1){
        inputBuffer[bufInd++] = ch;
      }
    }
  }
} // end task_serial
