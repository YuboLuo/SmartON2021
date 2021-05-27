### in order to test if the discretization of energy level will affect the performance
### we need to (1) fully train the qtable; then (2) test the whole spectrum of energy level

import numpy as np
import gym
import gym_foo
import random
import matplotlib.pyplot as plt
import copy

import xlsxwriter


env = gym.make('foo-v0')
action_size = env.action_space.n
state_size = env.observation_space.n



total_test_episodes = 1000     # Total test episodes
learning_rate = 0.7          # Learning rate
gamma = 0.618                 # Discounting rate




epsilon_hist = []
ConvergeCondition = 5
repeat = 12
PerformanceHistory = np.zeros((4,repeat/4,100,10))  ### ( num of eventtype,
                                                       # num of re-run for each type,
                                                       # num of energy level in evaluation,
                                                       # num of parameters in the returned results, must be consistent with the returned "returnValue" of env.evaluation() )


for eventtype in range(1):

    env.eventtype = 0
    env.gene_Event()

    for k in range(repeat/4):

        ### parameters related to RLEnergy Model
        StateCoverageNum = 4   # the distribution covers how many states
        EnergyLevelNum = 10    # how to discretize the energy level, [30,60,90,120] times of wake-up
        actionsRLEnergy = [np.inf, 3,2,1]
        qtableRLEnergy = np.zeros((StateCoverageNum * EnergyLevelNum, actionsRLEnergy.__len__()))
        MarkVisitedState = np.zeros(StateCoverageNum * EnergyLevelNum)   # mark the visited state, useful in IfConverged_RLEnergy

        ### in order to accelerate the convergence speed, we want to increase the probability of picking up the unvisited actions
        pval_table = np.zeros((StateCoverageNum * EnergyLevelNum, actionsRLEnergy.__len__()))
        for i in range((StateCoverageNum-1)*EnergyLevelNum): # for non-final-step states
            pval_table[i] = [1/(float)(actionsRLEnergy.__len__())]*actionsRLEnergy.__len__()
        for i in range((StateCoverageNum-1)*EnergyLevelNum,StateCoverageNum*EnergyLevelNum): # for final-step states
            pval_table[i][0] = 0
            pval_table[i][1:] = [1/(float)(actionsRLEnergy.__len__()-1)]*(actionsRLEnergy.__len__()-1)

        ### load qtable from saved data and do on-line learning
        # qtableRLEvent = np.load("qtable_iter13_SD30_ED5_ReW[30,-1]_SC4.npy")
        qtableRLEvent = np.zeros((env.TotalTime/env.StateDuration,2))
        qtableRLEvent[18:22,1] = 100

        # EnergyShift = [-450, -400, -350, 300, -250, -200 ,-150, -100, -50, 0, 50, 100, 150, 300, 500]  ### energy shift for EL = 15

        # EnergyShift = [-410, -340, -280, -200, -150, -110, 0, 110, 300, 500]  ### energy shift for EL = 10 
        # EnergyShift = [-450, -365, -295, -225, -155, -85, 0, 160, 360, 500]  ### energy shift for EL = 10, after rearrange
        EnergyShift = [-450, -369, -315, -243, -189, -135, -36, 90, 270, 450] # DCOSS

        # EnergyShift = [-410, -300, -200, -130, 0, 110, 300, 500]  ### energy shift for EL = 8
        # EnergyShift = [-450, -330, -225, -120, 0, 160, 360, 500]  ### energy shift for EL = 8 , after rearrange
        
        # EnergyShift = [-340, -170, -50, 200, 450]  ### energy shift for EL = 5 #DCOSS
        
        
        order = np.asarray(range(EnergyLevelNum))
        np.random.shuffle(order)  # shuffle the order
        
        ### train the whole qtable completely
        for i in range(EnergyLevelNum):
            energyshift = EnergyShift[order[i]]

            if energyshift == 0:
                i = i

            SumOfConvergedResult = 0
            ConvergedFlag = 0
            
            
            for episode in range(total_test_episodes):
    
                if episode % 5 == 0 and episode > 20:
                    episode = episode
    
                ### init parameters for q_learning
                stateRLEvent = env.reset()
                step = 0
                done = False
    
                ### to make RLEnergy Model learn fast, shitf the energy bank
                # energyshift = np.random.randint(low=-400, high=400)
                env.EnergyBank += energyshift
    
    
                action_hist = []
                StepPos = -1 # reset StepPos every iteration
    
    
                qtableRLEnergy_old = copy.deepcopy(qtableRLEnergy)
                for step in range(env.observation_space.n):
    
                    ### First we randomize a number
                    exp_exp_tradeoff = random.uniform(0, 1)
    
    
    
                    ### for RLEvent Model, it must be in working mode here, so we just pickup the highest probable action
                    actionRLEvent = np.argmax(qtableRLEvent[stateRLEvent, :2])  # actionRLEvent list has only two actions
    
                    if actionRLEvent > 0:
                    ### if action1 is picked up in RLEvent, then we need to start RLEnergy model
                        ### change action according to RLEnergy Model
                        EnergyLevel = env.FindEnergyLevel(EnergyLevelNum)  # get the current energy level
                        StepPos += 1   # move forward by one step
                        stateRLEnergy = StepPos * EnergyLevelNum + EnergyLevel
    
                        ### we DO NOT use epsilon, we always choose an action randomly during the learning phase
                        actionRLEnergy = np.random.randint(actionsRLEnergy.__len__())
                        # actionRLEnergy = np.argmax(np.random.multinomial(1,pval_table[stateRLEnergy]))  # pick on action based on pval table probability
    
                        action = actionRLEnergy
                    else:
                    ### else, it should sleep in this step/slot
                        action = actionRLEvent
    
                    ### execute the chosen action
                    new_stateRLEvent, reward, done, EnergyDepletedFlag = env.step(action)
                    # print('step=', step, "limit=", limit, "action=", action, "new_state=", new_state)
    
    
                    ### update RLEnergy qtable
                    if actionRLEvent > 0:
                    ### if action1 is picked up in RLEvent, then we need to start RLEnergy model
                        if StepPos < StateCoverageNum - 1:
                            EnergyLevel = env.FindEnergyLevel(EnergyLevelNum)  # get the current energy level
                            new_stateRLEnergy = (StepPos+1) * EnergyLevelNum + EnergyLevel
                        else :
                            new_stateRLEnergy = new_stateRLEnergy
    
                        ### Update Q(s,a):= Q(s,a) + lr [R(s,a) + gamma * max Q(s',a') - Q(s,a)]
                        ### determine the future reward
                        if StepPos < StateCoverageNum - 1: # when the next state is not the ternimal state
                            future_reward = np.max(qtableRLEnergy[new_stateRLEnergy, :])
                        else:
                            # it comes to the terminal state
                            if EnergyDepletedFlag == 1: # if enery depletion happens
                                future_reward = 0
                            else:
                                future_reward = 0 # there is no future reward from the terminal state
    
                        ### update qtableRLEnergy
                        qtableRLEnergy[stateRLEnergy, actionRLEnergy] = \
                            qtableRLEnergy[stateRLEnergy, actionRLEnergy] + \
                            learning_rate * ( reward + gamma * future_reward -
                                              qtableRLEnergy[stateRLEnergy, actionRLEnergy])
    
                        MarkVisitedState[stateRLEnergy] = 1  # mark the visited state, useful in IfConverged_RLEnergy
    
    
    
                    ### update state
                    stateRLEvent = new_stateRLEvent
    
                    ### if reach the end of the period, exit
                    if done == True:
                        break

    
                ### check if RLEnergy model has converged
                res = env.IfConverged_RLEnergy(qtableRLEnergy_old,qtableRLEnergy,MarkVisitedState,EnergyLevelNum,episode)
                if res == 1:
                    SumOfConvergedResult += res
                else:
                    SumOfConvergedResult = 0
                if SumOfConvergedResult >= ConvergeCondition:
                    ### model has converged
                    ConvergedFlag = 1
                    break


        
        ### when the whole qtable has converged, we start testing the performance
        ### make the entry energy randomly spread among the whole specturm of that energy level
        for idx, energyshift in enumerate(range(10,1001,10)):             
        
            # env.run_RLEnergy(qtableRLEvent,qtableRLEnergy,EnergyLevelNum,StateCoverageNum,episode,energyshift-540)
            env.run_RLEnergy(qtableRLEvent,qtableRLEnergy,EnergyLevelNum,StateCoverageNum,episode,EnergyShift[idx/10])


            ### env.evaluation()'s return value = [Fixed B, M, Wakeups, Simu B, M, Wakeups, GT B, M, wakeups]
            PerformanceHistory[eventtype,k,idx] = env.evaluation(65)
            
        print('k = {}'.format(k))


    print('event type = {}'.format(eventtype))

print('Finished {} repetitions per event type, [SC,EL] = [{},{}], Ratio = {}, CovCond = {}'.format(repeat/4,StateCoverageNum,EnergyLevelNum,env.DischargingRate,ConvergeCondition))
final = np.average(PerformanceHistory,axis=1)
Catch_B = final[0,:,3]

### for CTIDpro
CTIDpro_Catch_B = np.mean(final[0,:,3].reshape(10,10),axis = 1)
CTIDpro_Catch_B_efficiency = np.mean(final[0,:,6].reshape(10,10),axis = 1)
CTIDpro_Catch_B_std = np.std(final[0,:,3].reshape(10,10),axis=1)
CTIDpro_Catch_B_efficiency_std = np.std(final[0,:,6].reshape(10,10),axis = 1)

workbook = xlsxwriter.Workbook('Example{}.xlsx'.format(env.eventtype))
worksheet = workbook.add_worksheet("My sheet")
for i in range(len(CTIDpro_Catch_B)):
    # worksheet.write(0,i,Catch_B[i])
    worksheet.write(0, i, CTIDpro_Catch_B[i])
    worksheet.write(1, i, CTIDpro_Catch_B_efficiency[i])
    worksheet.write(2, i, CTIDpro_Catch_B_std[i])
    worksheet.write(3, i, CTIDpro_Catch_B_efficiency_std[i])
workbook.close()




