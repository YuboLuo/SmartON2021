
#define LED 12
#define LED2 4
#define button 3
#define buzzer 5
#define MOSFET 2


int buttonState = 0;
int count = 0;
int ButtonPressed = 0;
int flag_1stPress = 0;
int MOSFET_cnt = 0;

int countdown;
int buzzerFreq = 2000;
unsigned long Time=0;

/* eventtype-0 */
//int event[120] = {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,    1,1,0,0,0,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,    1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,0,0,1,1,1,1,0,0,0,1,1,1,1,   0,0,0,1,1,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/* eventtype-1 */
//int event[120] = {1,0,0,1,1,0,1,1,0,0,0,1,1,1,0,1,1,0,1,0,1,0,1,0,0,0,1,1,1,0,0,1,1,1,0,1,1,1,1,1,0,0,0,1,1,1,0,0,0,1,0,0,1,0,1,1,1,1,0,0,0,0,1,1,1,0,0,0,1,0,1,1,1,0,1,0,0,0,1,0,1,1,1,1,0,1,1,1,1,0,0,1,1,0,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,1,0,1,1,0,0,1,1,1};

/* eventtype-2 */
//int event[120] = {0,0,0,0,1,0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,0,1,1,0,1,0,1,1,1,1,0,0,1,1,0,0,1,0,1,0,1,0,0,1,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0};

/* eventtype-3 */
int event[120] =   {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,0,0,1,1,0,1,0,0,0,0,0,0,1,1,0,0,1,1,0,0,0,1,0,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,0,0,0,0,0};

// for debug
//int event[120] =     {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0};
int n_event = 0;

void setup() {
  // put your setup code here, to run once
  // I have no idea, why if I do pinMode(buzzer,OUTPUT), the code does not work
//  pinMode(buzzer,OUTPUT); // it is so weird that the code works well only if you comment this line
  pinMode(LED,OUTPUT);
  pinMode(button,INPUT);
  attachInterrupt(digitalPinToInterrupt(button), pin_ISR, RISING );
  attachInterrupt(digitalPinToInterrupt(MOSFET), pin_ISR2, RISING );
  n_event = sizeof(event)/sizeof(event[0]);

  Serial.begin(9600);
  while (!Serial);

}

void loop() {
  // put your main code here, to run repeatedly:
  int i;



  while(ButtonPressed){

    count = 3; // blilnk LED very fast to indicate the event-play starts

    while(count--){
      digitalWrite(LED,HIGH);
      delay(30);
      digitalWrite(LED,LOW);
      delay(30);
    }

    flag_1stPress = 1;  // in case that the first press of button triggers the ISR multiple times and makes ButtonPressed = 2 
    MOSFET_cnt = 0; // rest MOSFET_cnt
    Serial.println("MOSFET_cnt is reset");

    
    /* play the events */
    for(i = 0; i < n_event; i++){

      Time = millis();
      if(event[i] == 1){
          countdown = 975;
          while(countdown--){
            digitalWrite(buzzer,HIGH);
            delayMicroseconds(500000/buzzerFreq);  // 0.25ms
            digitalWrite(buzzer,LOW);
            delayMicroseconds(500000/buzzerFreq);  // 0.25ms
            }
        }
      else{
          delay(500);      
      }
      if( ButtonPressed == 2 ){
        ButtonPressed = 1;
        
        break;
      }

      if(event[i] == 1){
        countdown = 975;
          while(countdown--){
            digitalWrite(buzzer,HIGH);
            delayMicroseconds(500000/buzzerFreq);  // 0.25ms
            digitalWrite(buzzer,LOW);
            delayMicroseconds(500000/buzzerFreq);  // 0.25ms
          }
      }
      else{
          delay(500);      
      }
      if( ButtonPressed == 2 ){
        ButtonPressed = 1;
        break;
      }
//      Serial.println(millis()-Time);

      if(MOSFET_cnt >= 5){
          Serial.println("MOSFET_cnt >= 5, break event-showing");
          delay(500);  // this delay is necessary for the ISR2 to count all MOSFET_cnt
            break;
        }
      
    } // end of for loop

    /* use different LEB blinking pattern to tell different situations */
    if( i == n_event ){ 
      Serial.println("I am in loop 1: event-playing finished");
        while(1){
          
            digitalWrite(LED,HIGH);
            delay(100);
            digitalWrite(LED,LOW);
            delay(100);
                      
            if( ButtonPressed == 2 ){
                ButtonPressed = 1;
                break;
            }    
        }
    }
//
//
//    if(MOSFET_cnt >= 16){  // model converged
//      MOSFET_cnt = 0;
//      Serial.println("I am in loop 2: model converged");
//          while(1){
//              countdown = 7;
//              while(countdown--){
//                digitalWrite(LED,HIGH);
//                delay(50);
//                digitalWrite(LED,LOW);
//                delay(50);
//              }   
//              delay(1300); 
//              if( ButtonPressed == 2 ){
//                  ButtonPressed = 1;
//                  break;
//              } 
//          }  
//      }

      else if(MOSFET_cnt >= 16){  // 10 iterations finsihed
        MOSFET_cnt = 0;
        Serial.println("I am in loop 3: 10 iterations finished");
          while(1){
              digitalWrite(LED,HIGH);
              delay(700);
              digitalWrite(LED,LOW);
              delay(700);
                        
              if( ButtonPressed == 2 ){
                  ButtonPressed = 1;
                  break;
              } 
          }  
      }


      else if(MOSFET_cnt < 16 && MOSFET_cnt > 6){  // one iteration finished
        MOSFET_cnt = 0;
        Serial.println("I am in loop 4: one iteration finished");
          while(1){
              digitalWrite(LED,HIGH);
              delay(100);
              digitalWrite(LED,LOW);
              delay(100);
                        
              if( ButtonPressed == 2 ){
                  ButtonPressed = 1;
                  break;
              }   
          }
      }      
      
    
    
  }// end of while loop
}


void pin_ISR(){



  
  if(ButtonPressed == 0)
    ButtonPressed = 1;
  if(ButtonPressed == 1 && flag_1stPress == 1)
    ButtonPressed = 2;
    
    Serial.println("Button is pressed, button state is ");
    Serial.println(ButtonPressed);
}

void pin_ISR2(){

  MOSFET_cnt ++;

  Serial.println("MOSFET_cnt");
  Serial.println(MOSFET_cnt);
  
}
