/*
 * qlearning.h
 *
 *  Created on: May 6, 2019
 *      Author: Yubo-ASUS
 */

#ifndef ALGORITHM_H_
#define ALGORITHM_H_


#include <stdint.h>
#include "rng.h"
#include <string.h>

/* TIMER used for different purposes */
#define TIMER_BASE_PeriodicWakingup   TIMER_A1_BASE
#define TIMER_BASE_StateSwitch        TIMER_A4_BASE

/*
 * qlearning parameters
 */
#define EPSILON 1.0
#define MAX_EPSILON 1.0
#define MIN_EPSILON 0.01

#define GAMMA 0.618        // Discounting rate
#define LEARNING_RATE 0.7  // Learning rate
#define DECAY_RATE 0.03    // Exponential decay rate for exploration prob
#define EPISODES 50        // Total episodes for training



// for the env

//#define EventDuration 100  // event duration
#define ActionNum 4// 2  // how many actions in total in the action list
#define Period 1200  // time space
#define StateDuration 30  // state duration
#define StateNum 20 //40 // how many states in total
#define StepNum 4 // 40 // how many steps in one iteration

#define QtableLogCapacity 10 // how many iterations we can save in the log

#define RewardWithCatch 10 // 30
#define PenaltyWithoutCatch -1
#define ConvergeCondition 5    // if the qtable does not have big change in ConvergeCondition consecutive iterations, we think it is converged

/* For timer counter, we use VLO for ACLK, in LPM F_typ=8KHz, in active mode, F_typ=9.4KHz */
#define NumOfOneSecond 230


void initTimer_PeriodicWakingup(uint8_t action);
void initTimerA_StateSwitch(uint16_t counter);

uint8_t max_q_act(float row[ActionNum]);


uint8_t Get_RandomAction(int num); // get a random action from the [0,num]
float Get_RandomFloat(void);

/*
 * it returns the current state number according to current time
 */
uint16_t GetStep(void);

void run_step(uint16_t LeftTimeofCurState, uint8_t action);


void runQLearning(void);
void testQLearning(void);
void RLEventConvergeRate(void);

extern uint16_t getEnergyLevel(void); // get energy level

extern void SaveQtabletoLog(void); // save qtable to log

extern void CheckIfConverged_RLEnergy(void); // compare the current qtable and the previous one
extern void CheckIfConverged_RLEvent(void); // RLEvent model

/* Functions for FFT_based event detection */
extern uint16_t EventDetect(uint16_t freq);
extern uint16_t extractFreq(void);

extern uint16_t get_SW_Mode(void);
extern int IfFreq1Executable(uint16_t);
extern int IfFreq2Executable(uint16_t);

void EndofIteration(int type);

#endif /* ALGORITHM_H_ */





