
#define LED 2
#define button 3

int buttonState = 0;
int count = 0;
int ButtonPressed = 0;
int flag_1stPress = 0;

/* eventtype-0 */
//int event[120] = {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,1,1,0,0,0,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,0,0,1,1,1,1,0,0,0,1,1,1,1,0,0,0,1,1,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/* eventtype-1 */
//int event[120] = {1,0,0,1,1,0,1,1,0,0,0,1,1,1,0,1,1,0,1,0,1,0,1,0,0,0,1,1,1,0,0,1,1,1,0,1,1,1,1,1,0,0,0,1,1,1,0,0,0,1,0,0,1,0,1,1,1,1,0,0,0,0,1,1,1,0,0,0,1,0,1,1,1,0,1,0,0,0,1,0,1,1,1,1,0,1,1,1,1,0,0,1,1,0,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,1,0,1,1,0,0,1,1,1};

/* eventtype-2 */
//int event[120] = {0,0,0,0,1,0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,0,1,1,0,1,0,1,1,1,1,0,0,1,1,0,0,1,0,1,0,1,0,0,1,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0};

/* eventtype-3 */
int event[120] =   {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,0,0,1,1,0,1,0,0,0,0,0,0,1,1,0,0,1,1,0,0,0,1,0,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,0,0,0,0,0};

//int event[120] =     {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0};
int n_event = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED,OUTPUT);
  pinMode(button,INPUT);
  attachInterrupt(digitalPinToInterrupt(button), pin_ISR, RISING );
  n_event = sizeof(event)/sizeof(event[0]);
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

    /* play the events */
    for(i = 0; i < n_event; i++){
      
      if(event[i] == 1){
          digitalWrite(LED, HIGH);   
          delay(500); 
        }
      else{
        digitalWrite(LED, LOW);    
        delay(500);         
      }
      if( ButtonPressed == 2 ){
        ButtonPressed = 1;
        break;
      }

      delay(500);
      if( ButtonPressed == 2 ){
        ButtonPressed = 1;
        break;
      }

      
    } // end of for loop

    /* after one iteration is finished without break, we stop in this loop for a while */
    if( i == n_event ){ 
      
      for(i = 0; i < 10000; i++){
        
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
  
}
