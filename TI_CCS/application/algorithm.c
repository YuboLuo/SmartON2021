/*
 * qlearning.c
 *
 *  Created on: May 6, 2019
 *      Author: Yubo-ASUS
 */


#include "global.h"

///////////////////////////////////////////////////////////////////

/* we save the following information for analyze the learning process , and also for debugging the system */


#pragma LOCATION(QtableLog, 0x11000);  // log the qtable of each iteration
#pragma PERSISTENT(QtableLog);
float QtableLog[10][StateNum][ActionNum] = {0};


#pragma LOCATION(qtable, 0x11C80);  // the most updated qtable saved here
#pragma PERSISTENT(qtable);
float qtable[StateNum][ActionNum] =
{0};


float SavedQtableCnt_Offset = 0;        // offset, change this every time

#pragma PERSISTENT(SumOfConvergedResult);  // record the convergence result of each iteration
uint16_t SumOfConvergedResult = 0;


//////////////////////////////////////////////////////
 /* for RLEvent model */
//#pragma LOCATION(RewardLog, 0x11DC0);  //
//#pragma PERSISTENT(RewardLog);
//float RewardLog[QtableLogCapacity][StepNum] = {0};  // for RLEvent model, StepNum = 40 = 1200/30
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
// /* for RLEnergy model */

#pragma LOCATION(RewardLog, 0x11DC0);  // log the reward we get in each iteration
#pragma PERSISTENT(RewardLog);
float RewardLog[QtableLogCapacity][StepNum] = {0}; // for RLEnergy model, StepNum = 4

#pragma LOCATION(ActionLog, 0x11E60);  // log the actions we take in each iteration
#pragma PERSISTENT(ActionLog);
float ActionLog[QtableLogCapacity][StepNum] = {0};

#pragma LOCATION(StateLog, 0x11F00);  // log the state sequence the system goes through
#pragma PERSISTENT(StateLog);
float StateLog[QtableLogCapacity][StepNum] = {0};

#pragma LOCATION(VoltLog, 0x11FA0);  // log the voltage detected in getEnergyLevel()
#pragma PERSISTENT(VoltLog);
float VoltLog[QtableLogCapacity][StepNum] = {0};

#pragma LOCATION(SWModeLog, 0x12040);  // log the SWMode detected in getEnergyLevel()
#pragma PERSISTENT(SWModeLog);
float SWModeLog[QtableLogCapacity][StepNum] = {0};

#pragma LOCATION(NumSenseLog, 0x120E0);  // log the number of waking ups in each state
#pragma PERSISTENT(NumSenseLog);
float NumSenseLog[QtableLogCapacity][StepNum] = {0};

/* in order to make sure the highest-freq action be visited. This is to make sure the pre-state's actions get a proper future_reward */
#pragma PERSISTENT(pval_table); // mark if this state has been visited or not
uint8_t pval_table[StateNum] = {0};
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////





#if TRAINING
    #pragma PERSISTENT(SavedQtableCnt);  // counter for saving qtable
    uint16_t SavedQtableCnt = 0; // for training
#else
    #pragma PERSISTENT(SavedQtableCnt);  // counter for saving qtable
    uint16_t SavedQtableCnt = 1; // for testing
#endif


#pragma PERSISTENT(ConvergedFlag);  // set the flag to 1, if the system is converged
uint16_t ConvergedFlag = 0;





////////////////////////////////////////////////////////



/* Auxiliary functions for saving useful information above */
#pragma PERSISTENT(Actions); // log actions taken in one iteration, we will copy it to our ActionLog at the end of each iteration
float Actions[StepNum] = {0};

#pragma PERSISTENT(Rewards); //
float Rewards[StepNum] = {0};

#pragma PERSISTENT(States); //
float States[StepNum] = {0};

#pragma PERSISTENT(Volts); //
float Volts[StepNum] = {0};

#pragma PERSISTENT(SWModes); //
float SWModes[StepNum] = {0};

#pragma PERSISTENT(NumSense); //
float NumSense[StepNum] = {0};

//////////////////////////////////////////////////////////

#pragma PERSISTENT(Preqtable); // compare qtable and Preqtable for checking convergence
float Preqtable[StateNum][ActionNum] = {0};

#pragma PERSISTENT(Iteration);
uint8_t Iteration = 0;

#pragma PERSISTENT(episode);
uint8_t episode = 0;

#pragma PERSISTENT(epsilon);
float epsilon = 1;



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




uint16_t Action_List[ActionNum] = {100,5,2,1};  // for RLEnergy model
//uint16_t Action_List[ActionNum] = {100,2};      // for RLEvent model





#pragma LOCATION(TestResults, 0x14000);
#pragma PERSISTENT(TestResults);
uint16_t TestResults[1200][2] = {0};






/*
 * every reboot, the TimerA_StateSwitch should only be initialized once
 * after reboot, for the 1st entry into step(), we need to set the TimerA_StateSwitch according to the time left in current state
 * after reboot, for the 2nd/later entry into step(), we set TimerA_StateSwitch to StateDuration
 */
uint8_t TimerA_StateSwitch_InitFlag = 0;  //

/*
 * Race Condition: if TimerA_StateSwitch ISR happens when the TIMER_BASE_PeriodicWakingup ISR is happening
 * the LPM_wakeup command in TimerA_StateSwitch will lost its functionality
 * we need a flag to notify TIMER_BASE_PeriodicWakingup ISR, if TimerA_StateSwitch ISR happens within it,
 * TIMER_BASE_PeriodicWakingup ISR should make the program exit LPM
 */


uint8_t StateSwitchFlag = 0;
uint8_t PeriodicWakingupFlag = 0;



uint16_t new_state;
uint16_t TotalEnergyLevels = 5;

int EL,PreEL,PrePreEL,PrePrePreEL;
int Preaction = -1,PrePreaction = -1;



////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
/* The implementation of RLEvent model, how many iterations it needs to converge */
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
void RLEventConvergeRate(void){


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
    if(NumOfRestart == 1){
        memset(Actions,'1',4*StepNum);
        memset(Rewards,'1',4*StepNum);
        memset(States,'1',4*StepNum);
        memset(Volts,'1',4*StepNum);
        memset(SWModes,'1',4*StepNum);
        memset(NumSense,'1',4*StepNum);
    }


    GetTime();
    initial_step = Time / StateDuration;  // Call GetTime(), decide which state it is now
    LeftTimeofCurState = StateDuration - (Time % StateDuration);  // how many seconds left in this current state


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
            action = 0;
        }
        else{ // otherwise, we randomly pick up an action
            action = Get_RandomAction(ActionNum);
        }

        /* check if this action=1 is executable based on the current energy level */
        EnergyFlag = IfFreq2Executable(step);  // passing the parameter 'step' to the function is for logging volt in Volts
        if(!EnergyFlag){  // if energyflag == 0, it means action1 is not supported; otherwise, we do not change the action
            action = 0;
        }

        PowerOutageFlag = 1;  // set flag
        run_step(LeftTimeofCurState,action); // run
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
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
/*RLEnergy Model*/
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
void runQLearning(void){


    /* if the system has converged, we blink the LED slowly */
    if(ConvergedFlag == 1){
        while(1){
            P1OUT |= BIT0;
            __delay_cycles(4000000);
            P1OUT &= ~BIT0;
            __delay_cycles(8000000);
        }
    }



    uint16_t LeftTimeofCurState,initial_step;   // left time of current state
    float future_reward;
    int i,j;


    initial_step = 0;
    LeftTimeofCurState = StateDuration;

    /* reset Actions and Rewards for the first iteration after a new erase*/
    if(NumOfRestart == 1){
        memset(Actions,'1',4*StepNum);
        memset(Rewards,'1',4*StepNum);
        memset(States,'1',4*StepNum);
        memset(Volts,'1',4*StepNum);
        memset(SWModes,'1',4*StepNum);
        memset(NumSense,'1',4*StepNum);
    }


    /* if the system wakes up from a power outage */
    if( PowerOutageFlag == 1 ){ // PowerOutageFlag == 1 means the system just waked up from a power outage


        // if it restarts we update the qtable and then stop learning of this iteration
        if (step == StepNum - 1) // if it comes to the last step
            future_reward = 0;
        else{
            new_state = (step + 1) * TotalEnergyLevels + 0;  // EL = 0
            future_reward = qtable[new_state][max_q_act(qtable[new_state])];
        }


        /* after a power outage, we update the qtable here */
        // first we save the information we need for further off-line analysis
        Rewards[step] = reward;
        Actions[step] = action;
        States[step] = state;

        qtable[state][action] = qtable[state][action] + LEARNING_RATE * (reward + GAMMA * future_reward - qtable[state][action]);
        reward = 0;
        pval_table[state] = 1;

        PowerOutageFlag = 0; // reset the flag

        /* operations for end of one iteration */
        EndofIteration(0);

    }




    /* initialize variables */

    /* those Pre variables are used in getEnergyLevel(); the hardware of system can not distinguish between EL=0 and EL=1 when SW_Mode = 4 */
    /* so we need to use previous EL and Action information to induce which energy state the system is now */
    PrePreEL = -1;
    PreEL = -1;
    PrePrePreEL = -1;

    Preaction = -1;
    PrePreaction = -1;

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


        /* get energy level and state*/
        if (step == 0) {
            SWModes[0] = get_SW_Mode();  // convert SW_Mode into integer
            Volts[0] = getEnergyLevel();
            state = step*TotalEnergyLevels + EL;
        }

        /* pick an action randomly */
        action = Get_RandomAction(ActionNum);



//        /* prioritize action=3 if this state has been visited while its action=3 has not */
//        if( pval_table[state] == 1){
//            if(qtable[state][3] == 0){
//                action = 3;
//            }
//        }


//        if(SavedQtableCnt == 0){
//
//            if(step == 0) action = 0;  // for debug
//            else if(step == 1) action = 1;
//            else if(step == 2) action = 2;
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
//            else if(step == 1) action = 0;
//            else if(step == 2) action = 0;
//            else if(step == 3) action = 3;
//        }
//        else{
//            action = 3;
//        }


//
//        if(step == 0) action = 2;  // for debug
//        else if(step == 1) action = 3;
//        else if(step == 2) action = 3;




        PowerOutageFlag = 1;  // set flag
        run_step(LeftTimeofCurState,action);
        PowerOutageFlag = 0;  // if the system does not die, it will reset the flag

        /* save Pre energy level */
        PrePrePreEL = PrePreEL;
        PrePreEL = PreEL;
        PreEL = EL;


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

        qtable[state][action] = qtable[state][action] + LEARNING_RATE * (reward + GAMMA * future_reward - qtable[state][action]);
        reward = 0;
        state = new_state;
        pval_table[state] = 1;

        __no_operation();

        /* save Pre actions */
        PrePreaction = Preaction;
        Preaction = action;

    }

    /* operations for end of one iteration */
    EndofIteration(0);

}



////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
/* test how the system works using the converged qtable */
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
void testQLearning(void){


    uint16_t LeftTimeofCurState,initial_step;   // left time of current state



    initial_step = 0;
    LeftTimeofCurState = StateDuration;


    if( PowerOutageFlag == 1 ){ // PowerOutageFlag == 1 means the system just waked up from a power outage


        PowerOutageFlag = 0; // reset the flag
        SavedQtableCnt++;

        /* turn on Red LED, this iteration ends here*/
        P1OUT |= BIT0;

        /* stop the program */
        __bis_SR_register(LPM3_bits + GIE);


    }



    /* initialize the Pre variables */
    // those Pre variables are used in getEnergyLevel(); the hardware of system can not distinguish between EL=0 and EL=1 when SW_Mode = 4
    // so we need to use previous EL and Action information to induce which energy state the system is now
    PrePreEL = -1;
    PreEL = -1;
    PrePrePreEL = -1;

    Preaction = -1;
    PrePreaction = -1;

    for(step = initial_step; step < StepNum; step++){


        // get energy level
        if (step == 0) {
            getEnergyLevel();
            state = step*TotalEnergyLevels + EL;
        }

        // pick up an action with max value according to the qtable
        action = max_q_act(qtable[state]); // Get_RandomAction(ActionNum);



        PowerOutageFlag = 1;  // set flag

        run_step(LeftTimeofCurState,action);

        PowerOutageFlag = 0;  // if the system does not die, it will reset the flag

        /* save Pre energy level */
        PrePrePreEL = PrePreEL;
        PrePreEL = PreEL;
        PreEL = EL;

        /* get new energy level at the end of each step*/
        getEnergyLevel();

        if(step != StepNum - 1)
            new_state = (step + 1) * TotalEnergyLevels + EL;


        state = new_state;

        __no_operation();

        /* save Pre actions */
        PrePreaction = Preaction;
        Preaction = action;

    }


    SavedQtableCnt++;

    /* after one round of run, stop all timers */
    Timer_A_stop(TIMER_BASE_PeriodicWakingup);       // set MC = 00
    Timer_A_stop(TIMER_BASE_StateSwitch);            // set MC = 00
    Timer_B_stop(TIMER_B0_BASE);                     // set MC = 00

    /* turn on Red LED */
    P1OUT |= BIT0;


    //// during initial test, we only run one round and then stop
    __bis_SR_register(LPM3_bits + GIE);

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
    if(TimerA_StateSwitch_InitFlag == 0){
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
uint16_t GetStep(void){


    GetTime();  // get time from DS3231, save it in a global variable "uint16_t Time"
//    printf("Time: %10u",Time);
    return (Time / StateDuration);

}



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
    uint16_t freq,event,result;
    result = runSampling_Audio();  //sample one audio signal, it takes around 0.3s
    /* result will be 0 if StateSwitch happens during sampling process */
    /* we do not process this sampling if result = 0 */
    if(result){

        /* get the freq from FFT */
        freq = extractFreq();
        event = EventDetect(freq);

        /* for testing */
        TestResults[NumOfSense][0] = SavedQtableCnt;
        TestResults[NumOfSense][1] = freq;
        NumOfSense++;

        /* log the number of waking ups in each state */
        NumSense[step] += 1;

        if(event > 0){  // if >0 , it means an event happened
            reward += RewardWithCatch;

#if DEBUG
    P1OUT |= BIT1;  // turn on Green LED
    __delay_cycles(1600000); // 0.2s
    P1OUT &= ~BIT1; // turn off Green LED
#endif

        }
        else{  // if < 0 , it means no event happened
            reward += PenaltyWithoutCatch;  // no events happened
        }
    }



    /* race condition w.r.t StateSwitch happens
     * we need to do what is supposed to be done originally by TIMERA_ISR_StateSwitch
     * exit the LPM entered in void step()
     */
    if (StateSwitchFlag == 11){
        GPIO_setOutputLowOnPin(LEDLoad);  // turn off the extra load LED
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

    int i;
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




//******************************************************************************
// Functions for FFT_based event detection
//******************************************************************************



uint16_t extractFreq(void){

    uint16_t freqList[5] = {0};
    int i,counter,sum;

    // do 5 times of FFT
    for(i = 0; i < 5; i++){
        freqList[i] = runFftWithLea(&dataRecorded[i*256]);
    }

    // check if targeted frequency number is more than 3
    counter = 0;
    sum = 0;
    for(i = 0; i < 5; i++){
        if( freqList[i] > 1450 && freqList[i] < 2450){
            counter++;
            sum += freqList[i];
        }
    }


    if( counter <= 3)  // if counter less than the threshold, it means there is no-event
        return 0;
    else    // else, we return the averaged frequency value
        return sum/counter;


}



uint16_t EventDetect(uint16_t freq){



    uint16_t event;


//    if(freq < 1450) event = 0;
//    else if( freq > 1450 && freq < 1650) event = 1;  // event1: 1500
//    else if( freq > 1650 && freq < 1850) event = 2;  // event2: 1700
//    else if( freq > 1850 && freq < 2050) event = 3;  // event3: 1900
//    else if( freq > 2050 && freq < 2250) event = 4;  // event4: 2100
//    else if( freq > 2250 && freq < 2450) event = 5;  // event5: 2300
//    else event = 0;


    /* only support binay event */
    // I have no idea why the five types do not work again. I tested them before and they did work well
    // so now, since we do not distinguish different types of events, we only need binary classification
    if (freq < 2500 && freq > 1700)
        event = 1;
    else
        event = 0;


    return event;
}



void CheckIfConverged_RLEvent(void){

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
void CheckIfConverged_RLEnergy(void){
    /* we compare the current qtable and the previous one to see if there is only a small change */

    int i,j,counter;
    float old,new,variation,average;

    variation = 0;
    for(i = 0; i < StateNum; i++){
        for(j = 0; j < ActionNum; j++){

            old = Preqtable[i][j];
            new = qtable[i][j]; // current value

            if(new != old){
                // if the two are different
                counter ++;
                if(new > old){
                    variation += (new - old) / (old + 0.01);
                }
                else{
                    variation += (old - new) / (old + 0.01);
                }
            }

        }
    }

    if(counter != 0)
        average = variation / counter;
    else
        average = 0;

    if (average < 1){
        SumOfConvergedResult ++;  // increase the result by one
    }
    else{
        SumOfConvergedResult = 0; // if not, we reset it to zero, because we require *consecutive* small changes
    }

    if(SumOfConvergedResult >= ConvergeCondition){
        // if it reaches the converged condition, we set the ConvergedFlag
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



uint16_t getEnergyLevel(void){
/* according to the voltage level, return the current energy level  */



    uint16_t volt;
    volt = get_voltage_StableVersion();


    switch(SW_Mode){

    case SW4:
        /* the board can not distinguish between EL 0 and 1 in SW_Mode 4 */
        /* here, it means we have 5 capacitors connected */
        if(volt < 717){


            if(PreEL == 0){
                EL = 0; break;
            }



            else if(PreEL == 1){

                if(PrePreEL == 1){
                    // because it is charging in the pre state, we lower down the threshold by one
                    if(action >= 2){ // action = 2,3
                        EL = 0; break;
                    }
                    else{  // action = 0,1
                        EL = 1; break;
                    }
                }

                /* if it is the first time EL = 1 */
                else{
                    if(action == 0){
                        EL = 1; break;
                    }
                    else{
                        EL = 0; break;
                    }
                }
            }



            else if(PreEL == 2){

                /* not the first time */
                if(PrePreEL == 2){

                    /* if it is the third time */
                    if(PrePrePreEL == 2){

                        // because in the past three states, it is also charging. we lower down the threshold by one
                        if( PrePreaction + Preaction + action <= 2){ // pre three actions = (0,0,0),(0,1,0),(0,2,0),(0,1,1), no order in bracket
                            EL = 2; break;
                        }
                        else if(action + Preaction == 3){ // pre three actions = (0,1,2),(0,0,3), no order in bracket
                            EL = 1; break;
                        }
                        else{  // otherwise
                            EL = 0; break;
                        }


                    }
                    else{
                        if(action + Preaction <= 1){  // pre two actions = (0,0),(0,1), no order in bracket
                            EL = 2; break;
                        }
                        else if(action + Preaction == 2){  // pre two actions = (1,1),(0,2), no order in bracket
                            EL = 1; break;
                        }
                        else{  // otherwise
                            EL = 0; break;
                        }
                    }
                }

                /* else, it comes to EL=2 for the first time */
                else{
                    if( action <= 1 ){   // action = 0,1
                        EL = 2; break;
                    }
                    else if(action == 2){  // action = 2
                        EL = 1; break;
                    }
                    else{    // action = 3
                        EL = 0; break;
                    }
                }

            }

            else{ // PreEL = -1,3,4
                EL = 2; break;
            }

        }

        else if (volt < 751)
            EL = 3;
        else
            EL = 4;
        break;


    case SW3:
        /* here, it means we have 4 capacitors connected */
        if (volt < 717){
            __no_operation();
            EL = 0;
        }
        else if (volt < 732){
            __no_operation();
            EL = 1;
        }
        else{
            EL = 2;
        }

        break;


    default:
        /* otherwise */
        EL = 0;
        break;

    }

    return volt;
}


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


int IfFreq1Executable(uint16_t step){
    /* check if waking up every 1 seconds is executable, based on the current energy residue */

    uint16_t volt;
    volt = get_voltage_StableVersion();


    Volts[step] = volt;  // log
    SWModes[step] = get_SW_Mode();  // log


    switch(SW_Mode){

    case DEFAULT:
        return 0;

    case SW1:
        return 0;

    case SW2:
        return 0;

    case SW3:
        if( volt > 717 ) return 1;  // it should support 30+ times of wake up
        else return 0;

    case SW4:
        return 1;

    default:
        return 0;
    }

}

int IfFreq2Executable(uint16_t step){
    /* check if waking up every 2 seconds is executable, based on the current energy residue */

    uint16_t volt;
    volt = get_voltage_StableVersion();


    Volts[step] = volt;  // log
    SWModes[step] = get_SW_Mode();  // log



    switch(SW_Mode){

    case DEFAULT:
        return 0;

    case SW1:
        return 0;

    case SW2:
        if( volt > 717 ) return 1;
        else return 0;

    case SW3:
        return 1;


    case SW4:
        return 1;

    default:
        return 0;
    }

}



void EndofIteration(int type){

    int i;

    /* EndofIteration is shared by runQLearning(); testQLearning(); RLEventConvergeRate() */
    /* we use type to identify the caller function */
    // runQLearning(); type = 0;
    // testQLearning(); type = 1;
    // RLEventConvergeRate(); type = 2;

    if(type == 2){ // RLEventConvergeRate()
        /* check if the RLEnvent model has converged */
        CheckIfConverged_RLEvent();
    }
    else{ // runQLearning,testQLearning
        /* check if the RLEnergy model has converged */
        CheckIfConverged_RLEnergy();
    }

    /* after each iteration, we save the current qtable to QtableLog */
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
        while(1){
            P1OUT |= BIT0;
            __delay_cycles(4000000);
            P1OUT &= ~BIT0;
            __delay_cycles(8000000);
        }
    }

    /* if already 10 runs, we blink the LED */
    if(SavedQtableCnt >= 10){
        while(1){
            for(i=0; i<3; i++){  // blink
                P1OUT |= BIT0;
                __delay_cycles(500000);
                P1OUT &= ~BIT0;
                __delay_cycles(500000);
            }
            __delay_cycles(1500000);
            __delay_cycles(1500000);
        }
    }

    /* turn on Red LED, this iteration ends here*/
    P1OUT |= BIT0;

    /* reset parameters */
    reward = 0;
    step = 0;
    state = 0;
    action = 0;
    PowerOutageFlag = 0;

    /* stop the program */
    __bis_SR_register(LPM3_bits + GIE);

}

