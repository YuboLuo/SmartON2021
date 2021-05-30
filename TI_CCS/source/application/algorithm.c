/*
 * qlearning.c
 *
 *  Created on: May 6, 2019
 *      Author: Yubo-ASUS
 */


#include "global.h"
#include <stdlib.h>

///////////////////////////////////////////////////////////////////

/* we save the following information for analyzing the learning process , and also for debugging the system */

uint8_t TESTING_FLAG = 0;  // to show if we are testing the model, it will be set to 1 in the beginning of testQLearning()

uint8_t IMAGE_BASED = 1;  // 1: image-based, 0: audio-based

#pragma LOCATION(QtableLog, 0x11000);  // log the qtable of each iteration,
#pragma PERSISTENT(QtableLog);
float QtableLog[10][StateNum][ActionNum] = {0}; // it supports to log 10 qtables at max


#pragma LOCATION(qtable, 0x11A00);  // the most updated qtable saved here, addr for StateNum = 16
#pragma PERSISTENT(qtable);
float qtable[StateNum][ActionNum] =
{0};


/* how many iterations it has done so far in total, change this at each re-erase
 * because our QtableLog can only save 10 qtables at max, we have to use this variable
 * to record how many iterations done in total
 *  */
float SavedQtableCnt_Offset = 0;


#pragma PERSISTENT(SumOfConvergedResult);  // record the convergence result at the end of each iteration
uint16_t SumOfConvergedResult =0;


//////////////////////////////////////////////////////
 /* for Phase-1 Event Profiling
  * comment this section if you are not doing Phase-1
  * */
//#pragma LOCATION(RewardLog, 0x11DC0);  //
//#pragma PERSISTENT(RewardLog);
//float RewardLog[QtableLogCapacity][StepNum] = {0};  // for Phase-1, StepNum = 40 = 1200/30
//
//#pragma LOCATION(ActionLog, 0x12400);  //
//#pragma PERSISTENT(ActionLog);
//float ActionLog[QtableLogCapacity][StepNum] = {0};
//
//#pragma LOCATION(StateLog, 0x12A40);  // *** use this to save if power outage happens ***
//#pragma PERSISTENT(StateLog);
//float StateLog[QtableLogCapacity][StepNum] = {0};
//
//#pragma LOCATION(VoltLog, 0x13080);  //
//#pragma PERSISTENT(VoltLog);
//float VoltLog[QtableLogCapacity][StepNum] = {0};
//
//#pragma LOCATION(SWModeLog, 0x136C0);  //
//#pragma PERSISTENT(SWModeLog);
//float SWModeLog[QtableLogCapacity][StepNum] = {0};
//
///* in order to make sure the highest-freq action be visited. This is to make sure the pre-state's actions get a proper future_reward */
//#pragma LOCATION(pval_table, 0x13D00);
//#pragma PERSISTENT(pval_table); // mark if this state has been visited or not
//float pval_table[StateNum] = {0,0,0,0,1,1,1,0,1,1,1,1,1,1,1,0,1,1,0,1,0,0,1,1,1,0,1,1,1,1,1,0,0,1,1,1,1,1,1,1};

//////////////////////////////////////////////////////




//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
/* for Phase-2/Phase-3
 * comment this section if you are doing Phase-1
 * */
#pragma LOCATION(RewardLog, 0x11B00);  // log the reward we get in each iteration
#pragma PERSISTENT(RewardLog);
float RewardLog[QtableLogCapacity][StepNum] = {0}; // for Phase-2 RL, StepNum = 4

#pragma LOCATION(ActionLog, 0x11BA0);  // log the actions we take in each iteration
#pragma PERSISTENT(ActionLog);
float ActionLog[QtableLogCapacity][StepNum] = {0};

#pragma LOCATION(StateLog, 0x11C40);  // log the state sequence the system goes through
#pragma PERSISTENT(StateLog);
float StateLog[QtableLogCapacity][StepNum] = {0};

#pragma LOCATION(VoltLog, 0x11CE0);  // log the voltage detected in getEnergyLevel()
#pragma PERSISTENT(VoltLog);
float VoltLog[QtableLogCapacity][StepNum] = {0};

#pragma LOCATION(SWModeLog, 0x11D80);  // log the SWMode detected in getEnergyLevel()
#pragma PERSISTENT(SWModeLog);
float SWModeLog[QtableLogCapacity][StepNum] = {0};

#pragma LOCATION(NumSenseLog, 0x11E20);  // log the number of waking ups in each state
#pragma PERSISTENT(NumSenseLog);
float NumSenseLog[QtableLogCapacity][StepNum] = {0};

/* in order to make sure the highest-freq action be visited. This is to make sure the pre-state's actions get a proper future_reward */
#pragma PERSISTENT(pval_table); // mark if this state has been visited or not
uint8_t pval_table[StateNum] = {0};
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////






#pragma PERSISTENT(SavedQtableCnt);  // to count the number of saved qtable after an erase
uint16_t SavedQtableCnt = 0; //




#pragma PERSISTENT(ConvergedFlag);  // set the flag to 1, if the system is converged
uint16_t ConvergedFlag = 0;





////////////////////////////////////////////////////////



/* Auxiliary functions for saving useful information above
 * Those variables are only for one iteration, at the end of each iteration
 * they will be copied to non-volatile memory and then be reset
 * */
#pragma PERSISTENT(Actions); // log actions taken in one iteration, we will copy it to our ActionLog at the end of each iteration
float Actions[StepNum] = {0};

#pragma PERSISTENT(Rewards); // log rewards in one iteration, we will copy it to our RewardLog at the end of each iteration
float Rewards[StepNum] = {0};

#pragma PERSISTENT(States); // log the state, will be copied to StateLog at the end of each iteration
float States[StepNum] = {0};

#pragma PERSISTENT(Volts); // log the voltage, will be copied to VoltLog at the end of each iteration
float Volts[StepNum] = {0};

#pragma PERSISTENT(SWModes); // log the switch modes, will be copied to SWModeLog at the end of each iteration
float SWModes[StepNum] = {0};

#pragma PERSISTENT(NumSense); // log the number of wake-ups, will be copied to NumSenseLog at the end of each iteration
float NumSense[StepNum] = {0};

//////////////////////////////////////////////////////////
/* compare qtable and Preqtable for checking convergence
 * our convergence condition is there is only small changes in a number [n] of consecutive iterations
 * */
#pragma PERSISTENT(Preqtable); //
float Preqtable[StateNum][ActionNum] = {0};

#pragma PERSISTENT(Iteration);
uint8_t Iteration = 0;


#pragma PERSISTENT(reward);
float reward = 0;

#pragma PERSISTENT(step);
uint16_t step = 0;

#pragma PERSISTENT(state);
uint16_t state = 0;

#pragma PERSISTENT(action);
uint8_t action = 0;

#pragma PERSISTENT(PowerOutageFlag);  // if the systems dies in run_step()
uint8_t PowerOutageFlag = 0;


#pragma PERSISTENT(NumOfSense);
uint16_t NumOfSense = 0;


#pragma PERSISTENT(NumOfRestart);
uint16_t NumOfRestart = 0;



/* action: means how many seconds one event detection happens once
 * e.g. 5 means event detection happens one every 5 seconds
 * e.g. 100 means not waking up at all since our state duration is only 30-second long
 */
uint16_t Action_List[ActionNum] = {100,5,2,1};  // for Phase-2/Phase-3 RL
//uint16_t Action_List[ActionNum] = {100,2}; // {100,1}     // for Phase-1 Event Profiling




/* for debug, saving useful information  */
#pragma LOCATION(TestResults, 0x14000);
#pragma PERSISTENT(TestResults);
uint16_t TestResults[1200][2] = {0};






/*
 * every reboot, the TimerA_StateSwitch should only be initialized once
 * after reboot, for the 1st entry into step(), we need to set the TimerA_StateSwitch according to the time left in current state
 * after reboot, for the 2nd/later entry into step(), we set TimerA_StateSwitch to StateDuration
 */
uint8_t TimerA_StateSwitch_InitFlag = 0;  // to show if it is the first entry into step()


/*
 * Race Condition: if TimerA_StateSwitch ISR happens when the TIMER_BASE_PeriodicWakingup ISR is happening
 * the LPM_wakeup command in TimerA_StateSwitch will lost its functionality
 * we need a flag to notify TIMER_BASE_PeriodicWakingup ISR, if TimerA_StateSwitch ISR happens within it,
 * TIMER_BASE_PeriodicWakingup ISR should make the program exit LPM
 */
uint8_t StateSwitchFlag = 0;   // 1 means state switch is happening
uint8_t PeriodicWakingupFlag = 0; // 1 means periodic waking up is happening



uint16_t new_state;

int EL; //PreEL,PrePreEL,PrePrePreEL;
//int Preaction = -1,PrePreaction = -1;
int termination_flag = 0;  // if volt < 2000 (2.0V), we set the termination_flag to 1 to terminate this iteration








////////////////////////////////////////////////////////////////////////////////////
/* Phase-1, how many iterations Phase-1 needs to converge */
////////////////////////////////////////////////////////////////////////////////////
void Phase1ConvergeRate(void){


    uint16_t LeftTimeofCurState,initial_step;   // left time of current state
    int i,j,EnergyFlag;



    /* if the system has converged, we blink the LED slowly */
    if(ConvergedFlag == 1){
        while(1){
            P1OUT |= BIT0;
            __delay_cycles(4000000);
            P1OUT &= ~BIT0;
            __delay_cycles(8000000);
        }
    }


    /* reset Actions and Rewards for the first iteration after a new erase*/
    if(NumOfRestart == 1){ // for log
        memset(Actions,'1',4*StepNum);
        memset(Rewards,'1',4*StepNum);
        memset(States,'1',4*StepNum);
        memset(Volts,'1',4*StepNum);
        memset(SWModes,'1',4*StepNum);
        memset(NumSense,'1',4*StepNum);
    }


//    GetTime();
//    initial_step = Time / StateDuration;  // Call GetTime(), decide which state it is now
//    LeftTimeofCurState = StateDuration - (Time % StateDuration);  // how many seconds left in this current state
    initial_step = 0;
    LeftTimeofCurState = StateDuration;  // in DCOSS2021, we eliminated DS3231 RTC. We consider a power-off will automatically end this iteration


    if(PowerOutageFlag == 1){  // if power outage happens
        States[step] = Time;  // log
        reward = 0;  // clear reward
    }


    /* copy current qtable to Preqtable which will be used in CheckIfConverged */
    for(i = 0; i < StateNum; i++){
        for(j = 0; j < ActionNum; j++){
            Preqtable[i][j] = qtable[i][j];
        }
    }

    for(step = initial_step; step < StepNum; step++){

        if( pval_table[step] > 0 ){  // if this state has been visited by action=1
            action = 0;   // each state only be profiled once
        }
        else{ // otherwise, we randomly pick up an action
//            action = Get_RandomAction(ActionNum);
            action = 1;
        }

        /* check if this action=1 is executable based on the current energy level */
        EnergyFlag = IfFreq2Executable(step);  // passing the parameter 'step' to the function is for logging volt in Volts
        if(!EnergyFlag){  // if energyflag == 0, it means action1 is not supported; otherwise, we do not change the action
            action = 0;
        }

        PowerOutageFlag = 1;  // set flag
        run_step(LeftTimeofCurState,action); // run, power outage may happen during this process
        PowerOutageFlag = 0;  // if the system does not die, it will reset the flag

        /* update the qtable */
        // first we save the information we need for further off-line analysis
        Rewards[step] = reward; // log
        Actions[step] = action; // log

        qtable[step][action] = qtable[step][action] + LEARNING_RATE * (reward - qtable[step][action]);  // no future reward here
        reward = 0;
        if(action == 1) pval_table[step] = 1;  // mark the pval_table, this state has been visited by action=1

    }

    /* operations for end of one iteration */
    EndofIteration(2);


}





////////////////////////////////////////////////////////////////////////////////////
/* Phase-2, RL learning phase*/
////////////////////////////////////////////////////////////////////////////////////
void runQLearning(void){

    int i,j;

    uint16_t LeftTimeofCurState,initial_step;   // left time of current state
    float future_reward;


//    GetTime();
//    initial_step = Time / StateDuration;  // Call GetTime(), decide which state it is now
//    LeftTimeofCurState = StateDuration - (Time % StateDuration);  // how many seconds left in this current state
    initial_step = 0;
    LeftTimeofCurState = StateDuration;  // in DCOSS2021, we eliminated DS3231 RTC. We consider a power-off will automatically end this iteration

    /* reset Actions and Rewards for the first iteration after a new erase*/
    if(NumOfRestart == 1){
        memset(Actions,'1',4*StepNum); // each float has 4-byte
        memset(Rewards,'1',4*StepNum);
        memset(States,'1',4*StepNum);
        memset(Volts,'1',4*StepNum);
        memset(SWModes,'1',4*StepNum);
        memset(NumSense,'1',4*StepNum);
    }


    /* copy current qtable to Preqtable */
    for(i = 0; i < StateNum; i++){
        for(j = 0; j < ActionNum; j++){
            Preqtable[i][j] = qtable[i][j];
        }
    }

    /* initialize pval_table after a new erase*/
    /* pval_table is saved in FRAM, we only need to initialized once every re-erase */
    if( SavedQtableCnt == 0 ){
        for(i = 0; i < StateNum; i++){
            for(j = 0; j < ActionNum; j++){
                if(qtable[i][j] != 0){ // it means this state has at lease one action that has been visited
                    pval_table[i] = 1;
                    break; // jump out, go for the next state
                }
            }
        }
    }


    /* start a new iteration */
    for(step = initial_step; step < StepNum; step++){

        /* if volt < 2000 (2.0V), we terminate this iteration */
        if (termination_flag == 1){
            break;
        }


        /* get energy level and state*/
        if (step == 0) {
            SWModes[0] = get_SW_Mode();  // convert SW_Mode into integer
            Volts[0] = getEnergyLevel();
            state = step*TotalEnergyLevels + EL;
        }

        /* select an action randomly */
        action = Get_RandomAction(ActionNum);


        // for debug, manually control actions
//        if(SavedQtableCnt == 0){
//
//            if(step == 0) action = 3;  // for debug
//            else if(step == 1) action = 3;
//            else if(step == 2) action = 3;
//            else if(step == 3) action = 3;
//        }
//        else if(SavedQtableCnt == 1){
//
//            if(step == 0) action = 2;  // for debug
//            else if(step == 1) action = 2;
//            else if(step == 2) action = 2;
//            else if(step == 3) action = 2;
//        }
//        else if(SavedQtableCnt == 2){
//
//            if(step == 0) action = 3;  // for debug
//            else if(step == 1) action = 2;
//            else if(step == 2) action =0;
//            else if(step == 3) action = 3;
//        }
//        else{
//            action = 3;
//        }
//        action = 3;


//        if(step == 0) action = 0;  // for debug
//        else if(step == 1) action = 0;
//        else if(step == 2) action = 0;
//        else action = 1;




        PowerOutageFlag = 1;  // set flag to 1, if power-off happens during run_step()
                              // PowerOutageFlag will remain on 1 after a restart
                              // so we will be able to know a power-off happened


        run_step(LeftTimeofCurState,action);  // execute one action, which takes LeftTimeofCurState seconds to finish

        PowerOutageFlag = 0;  // if the system does not power-off during run_step(), make PowerOutageFlag to 0
                              // which means a power-off did not happen


        if (step == StepNum - 1) // if it comes to the last step
            future_reward = 0;
        else{
            SWModes[step+1] = get_SW_Mode();   // for log
            Volts[step+1] = getEnergyLevel(); // get energy level, for log

            new_state = (step + 1) * TotalEnergyLevels + EL;
            future_reward = qtable[new_state][max_q_act(qtable[new_state])];
        }


        /* update the qtable */
        // first we save the information we need for further off-line analysis
        Rewards[step] = reward;  // for log
        Actions[step] = action;  // for log
        States[step] = state;    // for log

        qtable[state][action] = qtable[state][action] + LEARNING_RATE * (reward + GAMMA * future_reward - qtable[state][action]); // update qtable
        reward = 0;  // reset reward
        state = new_state;  // go to next state
        pval_table[state] = 1;  // mark this state as it has been visited now

        __no_operation();



    }

    /* operations for end of one iteration */
    EndofIteration(0);

}




////////////////////////////////////////////////////////////////////////////////////
/* Phase-3, RL exploitation phase */
////////////////////////////////////////////////////////////////////////////////////
void testQLearning(void){

    int i,j;

    TESTING_FLAG = 1;


    /* if already 10 runs, we blink the LED */
    if(SavedQtableCnt >= 10){
        while(1){
            for(i=0; i<3; i++){  // blink
                GPIO_setOutputHighOnPin(REDLED);
                __delay_cycles(200000);
                GPIO_setOutputLowOnPin(REDLED);
                __delay_cycles(200000);
            }
        }
    }



    uint16_t LeftTimeofCurState,initial_step;   // left time of current state
    float future_reward;


//    GetTime();
//    initial_step = Time / StateDuration;  // Call GetTime(), decide which state it is now
//    LeftTimeofCurState = StateDuration - (Time % StateDuration);  // how many seconds left in this current state
    initial_step = 0;
    LeftTimeofCurState = StateDuration;  // in DCOSS2021, we eliminated DS3231 RTC. We consider a power-off will automatically end this iteration

    /* reset Actions and Rewards for the first iteration after a new erase*/
    if(NumOfRestart == 1){
        memset(Actions,'1',4*StepNum); // each float has 4-byte
        memset(Rewards,'1',4*StepNum);
        memset(States,'1',4*StepNum);
        memset(Volts,'1',4*StepNum);
        memset(SWModes,'1',4*StepNum);
        memset(NumSense,'1',4*StepNum);
    }


    /* copy current qtable to Preqtable */
    for(i = 0; i < StateNum; i++){
        for(j = 0; j < ActionNum; j++){
            Preqtable[i][j] = qtable[i][j];
        }
    }


    /* start a new iteration */
    for(step = initial_step; step < StepNum; step++){

        /* if volt < 2000 (2.0V), we terminate this iteration */
        if (termination_flag == 1){
            break;
        }


        /* get energy level and state*/
        if (step == 0) {
            SWModes[0] = get_SW_Mode();  // convert SW_Mode into integer
            Volts[0] = getEnergyLevel();
            state = step*TotalEnergyLevels + EL;
        }

        // pick up an action with max value according to the qtable
        action = max_q_act(qtable[state]);
        action = 3;

        run_step(LeftTimeofCurState,action);


        if (step == StepNum - 1) // if it comes to the last step
            future_reward = 0;
        else{
            SWModes[step+1] = get_SW_Mode();
            Volts[step+1] = getEnergyLevel(); // get energy level
            new_state = (step + 1) * TotalEnergyLevels + EL;
            future_reward = qtable[new_state][max_q_act(qtable[new_state])];
        }


        /* update the qtable */
        // first we save the information we need for further off-line analysis
        Rewards[step] = reward;
        Actions[step] = action;
        States[step] = state;

        reward = 0;
        state = new_state;

        __no_operation();



    }

    /* operations for end of one iteration */
    EndofIteration(0);


}







void run_step(uint16_t LeftTimeofCurState, uint8_t action){


    /* PeriodicWakingup */
    /* according to the taken action, re-initialize the timer initTimer_PeriodicWakingup */
    if(TimerA_StateSwitch_InitFlag == 0){ // we use the same flag with StateSwitch here
        /* this if condition will be only executed once at the beginning after a reboot */
        initTimer_PeriodicWakingup(action);
        Timer_A_startCounter(TIMER_BASE_PeriodicWakingup,TIMER_A_UP_MODE);   // set MC = 01, Up mode: Timer start counting up to TAxCCR0
    }
    else{
        /* for all later states, we only change the TAxCCR0 value, and we do not need to re-initialize the whole timer again */
        HWREG16(TIMER_BASE_PeriodicWakingup + OFS_TAxCCR0) = Action_List[action] * NumOfOneSecond;   // directly change TAxCCR0
        Timer_A_startCounter(TIMER_BASE_PeriodicWakingup,TIMER_A_UP_MODE);  // set MC = 01, Up mode: Timer start counting up to TAxCCR0
    }


    /* StateSwitch */
    /* according to how much time left for this state, re-initialize the timer initTimerA_StateSwitch */
    if(TimerA_StateSwitch_InitFlag == 0){ // for the 1st time of entering run_step()
        // if this is the first time entering step() after a reboot
        // only initialize once
        initTimerA_StateSwitch(LeftTimeofCurState);
        Timer_A_startCounter(TIMER_BASE_StateSwitch,TIMER_A_UP_MODE);   // set MC to MC_01, Up mode: Timer counts up to TAxCCR0
        TimerA_StateSwitch_InitFlag = 1;  // only change the TIMER_BASE_StateSwitch in the first two times of entering step()
    }
    else if (TimerA_StateSwitch_InitFlag == 1){
        // if this is the second time entering step() after a reboot, change the TAxCCR0 to StateDuration

        /* Based on User's Guide MSP430FR59xx Family, Page 643, Section 25.2.3.1.1
         * When changing TAxCCR0 while the timer is running,
         * if the new period is greater than or equal to the old period or greater than the current count value,
         * the timer counts up to the new period. If the new period is less than the current count value,
         * the timer rolls to zero. However, one additional count may occur before the counter rolls to zero.
         */
        /* in our case, the new value will only be greater than or equal to the old value */
        HWREG16(TIMER_BASE_StateSwitch + OFS_TAxCCR0) = StateDuration * NumOfOneSecond;  // directly change TAxCCR0
        TimerA_StateSwitch_InitFlag = 2;  // only change the TIMER_BASE_StateSwitch in the first two times of entering step()
    }
    // only change the TIMER_BASE_StateSwitch in the first two times of entering step()






    /*
     * the system enters LPM and should wake up from TIMERA_ISR_StateSwitch
     * but if race condition happens, it wakes up from TIMER_BASE_PeriodicWakingup
     */
    __bis_SR_register(LPM3_bits + GIE);  // enter LPM3

    /*
     * when it comes to the end of each state
     * a timer will exit LPM3, and the program comes to here
     */

    /* clear flag */
    StateSwitchFlag = 0;


#if DEBUG
    /* turn on the green LED for 0.2s to indicate state-switch*/
    P1OUT &= ~BIT0;  // turn off RED LED
    P1OUT |= BIT1;   // turn on GREEN LED
    __delay_cycles(1600000);  // light up the green LED for 0.2s
    P1OUT &= ~BIT1;  // turn off GREEN LED
#endif


}


/*
 * read time from the RTC(DS3231), and calculate in which state it should be now
 */
//uint16_t GetStep(void){
//
//
//    GetTime();  // get time from DS3231, save it in a global variable "uint16_t Time"
////    printf("Time: %10u",Time);
//    return (Time / StateDuration);
//
//}



/*
 * generate a random integer number between [0,num)
 */
uint8_t Get_RandomAction(int num){

    // this is not a very random function, especially when num is large
    // but our action list is small, roughly around 5-6, so it is acceptable

    uint8_t res;

    // Configure MPU with INFO memory as RW
    MPUCTL0 = MPUPW;
    MPUSAM |= MPUSEGIRE | MPUSEGIWE;
    MPUCTL0 = MPUPW | MPUENA;

    // Storage for generated random 8-bit data
    uint8_t rnd[16];  // rnd array size must be multiple of 16
    rng_generateBytes(rnd, 16);  // 2nd parameter must be multiple of 16

    res = rnd[0]%num;
    return res;
}



/*
 * generate a random float number between [0,1]
 */
float Get_RandomFloat(void){

    float res = 0;
    // Configure MPU with INFO memory as RW
    MPUCTL0 = MPUPW;
    MPUSAM |= MPUSEGIRE | MPUSEGIWE;
    MPUCTL0 = MPUPW | MPUENA;

    // Storage for generated random 8-bit data
    uint8_t rnd[16];  // rnd array size must be multiple of 16
    rng_generateBytes(rnd, 16);  // 2nd parameter must be multiple of 16

    res = (float)rnd[0]/255;
    return res;

}



/*
 * TimerA1 initialization
 * Control periodic waking-ups
 */
void initTimer_PeriodicWakingup(uint8_t action){

    // first stop and clear timers
    Timer_A_stop(TIMER_BASE_PeriodicWakingup);    // set MC = 00
    Timer_A_clear(TIMER_BASE_PeriodicWakingup);   // clear TAR

    // re-initialize timer
    Timer_A_clearTimerInterrupt(TIMER_BASE_PeriodicWakingup);
    Timer_A_initUpModeParam param = {0};
        param.clockSource = TIMER_A_CLOCKSOURCE_ACLK;  // ACLK = VLOCLK = 9.4KHz, in LPM3 VLO_typ=8000
        param.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_40;  // Divided by 40
        param.timerPeriod = Action_List[action] * NumOfOneSecond;  // count up to this number (period), request time every second
        param.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
        param.captureCompareInterruptEnable_CCR0_CCIE =
                TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
        param.timerClear = TIMER_A_DO_CLEAR;
        param.startTimer = false;
    Timer_A_initUpMode(TIMER_BASE_PeriodicWakingup, &param);

}



/*
 * TimerA1 interrupt service
 * Control periodic waking-ups
 */
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER_ISR_PeriodicWakingup(void){

    /* check if race condition happens w.r.t TIMERB_ISR_SupplyVoltage,
     * directly exit this current ISR function if it happens
     * IMPORTANT: we also need to check if race condition happens w.r.t itself
     */
    if(SupplyVoltageFlag || PeriodicWakingupFlag){
        return;
    }


    /* to avoid race condition with StateSwitch, see the explanation above when defining StateSwitchFlag
     * we need to immediately clear the StateSwitchFlag
     */
    StateSwitchFlag = 0;

    /* to avoid race condition with VCC_monitoring, set the PeriodicWakingupFlag */
    /* hold the lock */
    PeriodicWakingupFlag = 1;



    /* perform an action */
    int i;
    uint16_t event_detectionRes,volt;

    // sample and save the voltage, just for debug and analysis, not necessary for real deployment
    runSampling_VCC();
    volt = get_voltage();
    results_VCC[results_VCC_num*2] = volt;  // log voltage


    if(IMAGE_BASED){
        // for image_based experiments
        event_detectionRes = capture_image();
    }
    else{
        // for audio_based experiments
        runSampling_Audio();
        event_detectionRes = EventDetect(extractFreq());
    }

    // log detection results
    results_VCC[results_VCC_num*2+1] = event_detectionRes;
    results_VCC_num++;




    /* if volt < 2000, we terminate the this iteration, we assume 2000 is our turn-off voltage threshold  */
    if (volt < 2000){
        Timer_A_stop(TIMER_BASE_PeriodicWakingup);   // set MC = 00
        Timer_A_clear(TIMER_BASE_PeriodicWakingup);       // clear TAR
        Timer_A_stop(TIMER_BASE_StateSwitch);   // set MC = 00
        Timer_A_clear(TIMER_BASE_StateSwitch);       // clear TAR
        StateSwitchFlag = 11;
        termination_flag = 1;

        __bic_SR_register_on_exit(LPM3_bits);

        return;
    }




    /* event_detectionRes will be 0 if StateSwitch happens during sampling process */
    /* we do not process this sampling if event_detectionRes = 0 */
    if(event_detectionRes){

        /* get the freq from FFT */
//        freq = extractFreq();
//        event = EventDetect(freq);

        /* for testing, Phase-3 */
        TestResults[NumOfSense][0] = SavedQtableCnt;
        TestResults[NumOfSense][1] = event_detectionRes;
        NumOfSense++;

        /* log the number of waking ups in each state */
        NumSense[step] += 1;



        if(event_detectionRes > 1){  // if result > 1 , it means an event detected
            reward += RewardWithCatch;
        }
        else{  // no event detected
            reward += PenaltyWithoutCatch;  // no events happened
        }


    }



    /* race condition w.r.t StateSwitch happens
     * we need to do what is supposed to be done originally by TIMERA_ISR_StateSwitch
     * exit the LPM entered in void step()
     */
    if (StateSwitchFlag == 11){
        __bic_SR_register_on_exit(LPM3_bits);
    }

    /* release the lock */
    PeriodicWakingupFlag = 0;

}



/*
 * TimerA4 initialization
 * change to next state
 */
void initTimerA_StateSwitch(uint16_t counter){

    // first stop and clear timer
    Timer_A_stop(TIMER_BASE_StateSwitch);   // set MC = 00
    Timer_A_clear(TIMER_BASE_StateSwitch);  // clear TAR

    // re-initialize timer
    Timer_A_clearTimerInterrupt(TIMER_BASE_StateSwitch);
    Timer_A_initUpModeParam param = {0};
        param.clockSource = TIMER_A_CLOCKSOURCE_ACLK;  // ACLK = VLOCLK = 9.4KHz, in LPM3 VLO_typ=8000
        param.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_40;  // Divided by 40, 8000/40 = 200
        // f=200Hz, it supports around 300s at max
        param.timerPeriod = counter * NumOfOneSecond;  // interrupt at the end of each state
        param.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
        param.captureCompareInterruptEnable_CCR0_CCIE =
                TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
        param.timerClear = TIMER_A_DO_CLEAR;
        param.startTimer = false;
    Timer_A_initUpMode(TIMER_BASE_StateSwitch, &param);

}



/*
 * TimerA4 interrupt service
 * change to next state
 */
#pragma vector=TIMER4_A0_VECTOR
__interrupt void TIMERA_ISR_StateSwitch(void){

//    int i;
    /*
     * when this interrupt happens, it means the program comes to the end of the current state
     * we need to wake up from LPM, the one in step()
     */


    // we first stop PeriodicWakingup timer and clear its TAR
    Timer_A_stop(TIMER_BASE_PeriodicWakingup);   // set MC = 00
    Timer_A_clear(TIMER_BASE_PeriodicWakingup);       // clear TAR

    /* why make StateSwitchFlag be 11 */
    // because, while making it 1, every time when the function uint16_t extractFreq(void) is called
    // StateSwitchFlag becomes 1 (it should not), and I have no idea why this happens. so I just make it 11 and then
    // the calling of extractFreq(void) does not affect StateSwitchFlag any more. it is so weird
    StateSwitchFlag = 11;   // for solving the race condition with TIMER_ISR_PeriodicWakingup

//    /* blink LED fast */
//    for(i=0; i<3; i++){
//        P1OUT |= BIT0;
//        __delay_cycles(200000);
//        P1OUT &= ~BIT0;
//        __delay_cycles(200000);
//    }

    // exit the LPM3
    __bic_SR_register_on_exit(LPM3_bits);


}




uint8_t max_q_act(float row[ActionNum]){
// return the action that has the max q-value

    float max_q;
    uint8_t i,index;

    max_q = row[0];
    index = 0;
    for( i = 1; i < ActionNum; i++){
        if( row[i] > max_q){
            max_q = row[i];
            index = i;
        }
    }

    return index;

}


float get_max(float a, float b){

    if (a > b) return a;
    else return b;

}

float get_abs(float a){

    if(a < 0) return -a;
    else return a;

}

//******************************************************************************
// Functions for FFT_based event detection
//******************************************************************************



uint16_t extractFreq(void){

    uint16_t freqList[5] = {0};
    int i,counter,sum;

//    // do 5 times of FFT
//    for(i = 0; i < 5; i++){
//        freqList[i] = runFftWithLea(&dataRecorded[i*256]);
//    }
//
//    // check if targeted frequency number is more than 3
//    counter = 0;
//    sum = 0;
//    for(i = 0; i < 5; i++){
//        if( freqList[i] > 1450 && freqList[i] < 2450){
//            counter++;
//            sum += freqList[i];
//        }
//    }
//
//
//    if( counter <= 3)  // if counter less than the threshold, it means there is no-event
//        return 0;
//    else    // else, we return the averaged frequency value
//        return sum/counter;

    return runFftWithLea(&dataRecorded[0]);

}



uint16_t EventDetect(uint16_t freq){



    uint16_t event;

    if (freq < 2500 && freq > 1700)
        event = 9;  // detected an event
    else
        event = 1;  // no event detected


    return event;
}

//******************************************************************************

void CheckIfConverged_Phase1(void){

    int i;
    int threshod = 5; // for detect every 2 seconds, threshod = 5 is ok

    for(i = threshod; i < StateNum; i++){ // check every element of pval_table, skip the first few elements
        if(pval_table[i] == 0)  // if there exists non-visited state, return
            return;
    }

    // if all states have been visited, set the flag
    ConvergedFlag = 1;
}


/* RLEnergy model */
void CheckIfConverged_Phase2(void){
    /* we compare the current qtable and the previous one to see if there is only a small change */

    int i,j,counter;
    float old,new,variation,average;

    int flag = 0;  // when the model is close to convergence, it may only have small changes, e.g. from 3 to 6 but by our definition of big change, (6-3)/3 =  1 is still big
                   // so we need to rule out this scenario, we only regard it as a big change when the value of old/new is bigger than a number, e.g. 10

    variation = 0;
    counter = 0;
    for(i = 0; i < StateNum; i++){
        for(j = 0; j < ActionNum; j++){

            old = Preqtable[i][j];
            new = qtable[i][j]; // current value

            if(new != old){

                // if the two are different
                counter ++;
                variation += get_abs(new - old) / (get_abs(old) + 0.01);

                if(get_abs(old) > 20 || get_abs(new) > 20)  flag = 1;

            }

        }
    }

    if(counter != 0)  average = variation / counter;
    else  average = 0;


    if (average < 1 || flag == 0){
        SumOfConvergedResult ++;  // increase the result by one, only small changes happened or no change at all
    }
    else{
        SumOfConvergedResult = 0; // if not, we reset it to zero, because we require *consecutive* small changes
    }

    if(SumOfConvergedResult >= ConvergeCondition){
        // if it reaches the converged condition, we set the ConvergedFlag to 1
        ConvergedFlag = 1;
    }


}





void SaveQtabletoLog(void){
    /* I want to run multiple iterations after each erase, so that I do not need to save the qtable to my laptop after each iteration */
    /* so I save the qtable of each iteration to a persistent array and then I can read them all in one time */


    int i,j;
    float temp;

    /* save the qtable of this iteration into QtableLog */
    for(i = 0; i < StateNum; i++){
        for(j = 0; j < ActionNum; j++){
            QtableLog[SavedQtableCnt][i][j] = qtable[i][j];
        }
    }

    /* save the actions and rewards of this iteration */
    for(i = 0; i < StepNum; i ++){
        ActionLog[SavedQtableCnt][i] = Actions[i];
        RewardLog[SavedQtableCnt][i] = Rewards[i];
        StateLog[SavedQtableCnt][i] = States[i];
        VoltLog[SavedQtableCnt][i] = Volts[i];
        SWModeLog[SavedQtableCnt][i] = SWModes[i];
        NumSenseLog[SavedQtableCnt][i] = NumSense[i];
    }

    /* save some maker information of the qtable */
    temp = SavedQtableCnt;
    QtableLog[SavedQtableCnt][StateNum-1][0] = temp + SavedQtableCnt_Offset; // the number of iterations that we have done to obtain this this qtable
    QtableLog[SavedQtableCnt][StateNum-2][0] = SumOfConvergedResult; // the convergence condition

    SavedQtableCnt++;

}


/* according to the voltage level, return the current energy level
 * image-based experiments and audio-based experiments use different capacitor combinations
 * you have to do some experiments to decide how to discretize the voltage levels based on your specific combination of capacitors
 * Ideally, energy level should correspond to the action list. Different actions should cause the system to either go up or go down in energy levels
 * if one energy level is so wide that even doing different actions will stay in the same energy level, then there is not difference between taking different actions
 * which is not we want. we want the system to behave differently when you take different actions and that makes why our waking-up decision-making matters
 * for better investigate the effect of energy level, it is advised to do it in the simulation where you can change your discretizing strategy easily
 * */
uint16_t getEnergyLevel(void){



    uint16_t volt;
    runSampling_VCC();
    volt = get_voltage();  // after we use 1.2V ref voltage, it is quite stable, we do not need the stable version anymore


    if(IMAGE_BASED){

        switch(SW_Mode){

        case SW4:
            /* SW4 = we have 5 capacitors connected */
            if      (volt < 2200) EL = 0;  // those threshold numbers are obtained by experiments where you log how many times of event detection each energy level supports
            else if (volt < 2500) EL = 1;
            else if (volt < 2800) EL = 2;
            else                  EL = 3;
            break;

        case SW3:
            /* SW3 = we have 4 capacitors connected */
            if      (volt < 2300) EL = 0;
            else if (volt < 2850) EL = 1;
            else                  EL = 2;
            break;

        case SW2:
            /* SW2 = we have 3 capacitors connected */
            if (volt < 2700) EL = 0;
            else             EL = 1;
            break;

        default:
            /* otherwise */
            EL = 0;
            break;

        }

    }

    else{

        switch(SW_Mode){

        // for audio-based, we only use SW=2,3
        case SW3:
            /* SW3 = we have 4 capacitors connected */
            if      (volt < 2140) EL = 0;
            else if (volt < 2280) EL = 1;
            else if (volt < 2600) EL = 2;
            else                  EL = 3;
            break;

        case SW2:
            /* SW2 = we have 3 capacitors connected */
            if      (volt < 2400) EL = 0;
            else if (volt < 3050) EL = 1;
            else                  EL = 2;
            break;

        default:
            /* otherwise */
            EL = 0;
            break;

        }

    }











    return volt;
}


/* get the integer of SW mode */
uint16_t get_SW_Mode(void){
    // because our SW_Mode is typedef enum, not integer
    // we need this function to convert it into number so that we can save easily

    switch(SW_Mode){

        case DEFAULT:
            return 0;

        case SW1:
            return 1;

        case SW2:
            return 2;

        case SW3:
            return 3;

        case SW4:
            return 4;

        default:
            return 4;
    }
}

/* check if waking up every 1 seconds is executable, based on the current energy residue */
/* it should support 30+ times of wake up
 * this function is used for Phase1ConvergeRate() where we only use high walking up frequency to profile the event pattern
 * */
int IfFreq1Executable(uint16_t step){

    uint16_t volt;
//    volt = get_voltage_StableVersion();
    runSampling_VCC();
    volt = get_voltage();

    Volts[step] = volt;  // log
    SWModes[step] = get_SW_Mode();  // log


    switch(SW_Mode){

    case DEFAULT:
        return 0;

    case SW1:
        return 0;

    case SW2:
        if (volt > 2700) return 1;   // those threshold numbers are obtained by experiments
        else return 0;

    case SW3:
        if(volt > 2850) return 1;
        else return 0;

    case SW4:
        if(volt > 2200) return 1;
        else return 0;

    default:
        return 0;
    }

}

int IfFreq2Executable(uint16_t step){
    /* check if waking up every 2 seconds is executable, based on the current energy residue */
    /* it should support at least 15times of event detection */

    uint16_t volt;
//    volt = get_voltage_StableVersion();
    runSampling_VCC();
    volt = get_voltage();

    Volts[step] = volt;  // log
    SWModes[step] = get_SW_Mode();  // log



    switch(SW_Mode){

    case DEFAULT:
        return 0;

    case SW1:
        if (volt > 3400) return 1;   // those threshold numbers are obtained by experiments
        else return 0;

    case SW2:
        if( volt > 2400 ) return 1;
        else return 0;

    case SW3:
        if (volt > 2200) return 1;
        else return 0;


    case SW4:
        if (volt > 2130) return 1;
        else return 0;

    default:
        return 0;
    }

}



void EndofIteration(int type){

    int i;

    /* EndofIteration is shared by runQLearning(); testQLearning(); Phase1ConvergeRate() */
    /* we use type to identify the caller function */
    // runQLearning(); type = 0;
    // testQLearning(); type = 1;
    // Phase1ConvergeRate(); type = 2;

//    if(!TESTING_FLAG){  // we only check convergence in training phase
//        if(type == 2){ // Phase1ConvergeRate()
//            /* check if the RLEnvent model has converged */
//            CheckIfConverged_Phase1();
//        }
//        else{ // runQLearning,testQLearning
//            /* check if the RLEnergy model has converged */
//            CheckIfConverged_Phase2();
//        }
//    }



    /* check if converged and save the current qtable to QtableLog */

//    CheckIfConverged_Phase2();
    SaveQtabletoLog();


    /* reset Actions and Rewards after each iteration*/
    memset(Actions,'1',4*StepNum);
    memset(Rewards,'1',4*StepNum);
    memset(States,'1',4*StepNum);
    memset(Volts,'1',4*StepNum);
    memset(SWModes,'1',4*StepNum);
    memset(NumSense,'1',4*StepNum);

    /* after one round of run, stop all timers */
    Timer_A_stop(TIMER_BASE_PeriodicWakingup);       // set MC = 00
    Timer_A_stop(TIMER_BASE_StateSwitch);            // set MC = 00
    Timer_B_stop(TIMER_B0_BASE);                     // set MC = 00


    /* if the system has converged, we blink the LED slowly */
    if(ConvergedFlag == 1){
        i = 3;
        while(1){
            GPIO_setOutputHighOnPin(REDLED);
            __delay_cycles(1000000);
            GPIO_setOutputLowOnPin(REDLED);
            __delay_cycles(1000000);
        }
        __bis_SR_register(LPM3_bits + GIE);
    }

    /* if already 10 runs, we blink the LED */
    if(SavedQtableCnt >= 10){
        i = 3;
        while(1){
            GPIO_setOutputHighOnPin(REDLED);
            __delay_cycles(1000000);
            GPIO_setOutputLowOnPin(REDLED);
            __delay_cycles(1000000);
        }
        __bis_SR_register(LPM3_bits + GIE);
    }

    /* turn on Red LED, this iteration ends here*/
//    P1OUT |= BIT0;

    /* reset parameters */
    reward = 0;
    step = 0;
    state = 0;
    action = 0;
    PowerOutageFlag = 0;

    i = 1;
    while(1){
        GPIO_setOutputHighOnPin(REDLED);
        __delay_cycles(1000000);
        GPIO_setOutputLowOnPin(REDLED);
        __delay_cycles(1000000);
    }

    /* stop the program */
    __bis_SR_register(LPM3_bits + GIE);

}

