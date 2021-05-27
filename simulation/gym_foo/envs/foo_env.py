import gym
from gym import error, spaces, utils
from gym.utils import seeding

from gym.envs.toy_text import discrete
import numpy as np


import xlrd
import scipy.stats as st
import matplotlib.pyplot as plt

class FooEnv(discrete.DiscreteEnv):
    metadata = {'render.modes': ['human']}



    def __init__(self):


        # define useful variables
        self.StateDuration = 30    # how many seconds one state represents
        self.EventDuration = 1  # each event lasts for this amout of seconds
        self.reward_catch = 5   # if wake up and catch an event
        self.reward_nocatch = -1   # if wake up and does not catch an event
        self.reward_cap = 0     # if charged to EnergyBankMax

        self.actions = [np.inf,5,2,1]  # for RLEnergy
        # self.actions = [np.inf, 2]  # for RLEvent
        
        self.EnergyBankMax =  np.inf  # 12 * 3600  # capacity of energybank, once reached charging is wasted

        self.TotalTime = 1200  # the time slice is second, we take one day as one period = 24 hours
        self.ChargingRate = 1  # how many unit energy the system harvests in one second
        self.DischargingRate = 9  # how many unit energy the system consumes in one second when in running mode


        ### for coverage of 60 seconds
        # self.params = [[600, 13, 100],[570, 60, 100],[600-35,0.5,20,100],[0.4, 630-12, 5, 100]] # norm; loggamma; lognorm; uniform

        # ### for coverage of 120seconds
        self.datapoint = 100
        self.params = [[600, 19, self.datapoint],[540, 120, self.datapoint], [600 - 65, 0.5, 33, self.datapoint], [0.4, 660 - 12, 8, self.datapoint]]  # norm; uniform; lognorm ; loggamma

        self.eventtype = 0
        self.ELthreshold = []


        self.EnergyBank = 0    # usable energy
        self.OverCharge = 0
        self.TimetoDetect = 1  # how many time slices it requires to successfully detect an event
        # self.RunFlag = 0       # 0 = sleep, 1 = run
        # self.FigureFlag = 0    # if draw figure for diagnosis
        # self.animal = 2


        self.HarvEnPattern = np.ones((self.TotalTime,), dtype=int)  # sim1: harvestable energy is always available
        # self.set_HarvEnPattern(start=6,end=18) # windy, rainy, sunny, random

        self.EventPattern = np.zeros((self.TotalTime,), dtype=int)  # initialize anomaly pattern
        self.gene_Event()  # the parameter is event duration, must <= 3600

        # for on-line learning we need to verify if the system can learn on line
        # self.EventPattern_new = np.zeros((self.TotalTime,), dtype=int)  # for on-line learning
        # self.gene_New1(2)   # generate and set data for changed environment, the parameter is to how many hours you want to shift the distribution

        self.EventPattern_valid = np.zeros((self.TotalTime,), dtype=int)
        self.gene_Event_valid()


        self.SimuFIXED = np.zeros((self.TotalTime,), dtype=int)  # fixed
        self.SimuDynamic = np.zeros((self.TotalTime,), dtype=int)

        self.action_hist = []
        self.TotalHarvestedEnergy = 0;
        self.reward_window = [None] * 1000     # slide window for reward

        self.new_dist_flag = 0


        ### number of states, each state represents self.StateDuration seconds
        nS = self.TotalTime / self.StateDuration
        nA = self.actions.__len__()

        ### we do not use P, isd in our customized env
        P = {s: {a: [] for a in range(nA)} for s in range(nS)}
        isd = np.zeros(nS) + 0.0000000001
        isd /= isd.sum()


        discrete.DiscreteEnv.__init__(self, nS, nA, P, isd)


        output = ("StateDuration={}".format(self.StateDuration),
                  "EventDuration={}".format(self.EventDuration),
                  "Reward=[{},{},{}]".format(self.reward_catch, self.reward_nocatch, self.reward_cap),
                  "EnMax={}".format(self.EnergyBankMax),
                  "Actions={}".format(self.actions))
        print output


    def reset(self):
    # each episode starts from the state_0, the beginning of the Timeline
    # the default reset() in discrete.py randomly simples a number as the start state
    # as we need to harvests energy from the beginning, we need to always starts from the first state
        self.s = 0
        self.lastaction = None
        self.EnergyBank = 0
        self.TotalHarvestedEnergy = 0;
        self.SimuDynamic.fill(0)
        self.OverCharge = 0

        self.reward_window = [0]*1000
        return  self.s


    def step(self, action,online_learning = 0,training = 0):
        #### during the training of qlearning, we should not consider the energy left
        ### because during the training phase, the beginning steps may consume a lot of energy
        ### which causes the later steps when the event happens do not have enough energy to wake up as frequently as it should do

        reward = 0
        info = None
        reward_hist = []
        EnergyDepletedFlag = 0  ### if the energy has been exhausted
        # training = 1


        for i in range(self.StateDuration):

            index = self.s * self.StateDuration + i

            ### check if energy depletion happens
            if i % self.actions[action] == 0 and i != 0 and self.EnergyBank - self.DischargingRate < 0 and action!=0:
                EnergyDepletedFlag = 1


            if (i % self.actions[action] == 0 and i != 0 and (self.EnergyBank - self.DischargingRate >= 0 or training)) and action!=0:
                self.EnergyBank -= self.DischargingRate

                self.SimuDynamic[index] = 1    # log wakeup time

                # reward += self.get_reward_by_prob(index,online_learning)
                # reward += self.get_event_prob(float(self.TotalTime)/index) * 5
                reward += self.get_reward(index)
                reward_hist.append(reward)

            else:
                if(self.EnergyBank >= self.EnergyBankMax):
                    reward = self.reward_cap

                if self.HarvEnPattern[index] > 0:
                    if self.EnergyBank >= self.EnergyBankMax:
                        self.OverCharge += 1
                    else:
                        self.EnergyBank += self.ChargingRate
                        self.TotalHarvestedEnergy += self.ChargingRate   # log total harvested energy


        if self.s == self.observation_space.n - 1:
        # if this is the last state
            done = True
        else:
        # if this is not the last state
            done = False
            self.s += 1

        self.lastaction = action

        return self.s, reward,done,EnergyDepletedFlag


    def get_reward(self,index):

        # if flag == 0: # training
        if self.EventPattern[index] > 0:  # if catch an evnent, calculate rewards
            # reward = self.EventPattern[index] * self.reward_catch     # consider event strength
            reward = self.reward_catch                                # do not consider event strength
            return reward
        else:  # if no catch, give a punishment
            return self.reward_nocatch




    def actionlimit(self):
    # for current state, check what actions are executable based on the following parameters
    # EnergyBank, ChargingRate, DischargingRate
    # return the id_of_action which is the lowest impossible action

        for i in range(1,self.action_space.n):
            # for each action we test if they are executable

            EnergyBank = self.EnergyBank  # current usable energy
            for j in range(self.StateDuration):

                index = self.s * self.StateDuration + j  # get current time slice
                if j % self.actions[i] == 0 and j != 0:
                    EnergyBank -= self.DischargingRate
                else:
                    if (self.HarvEnPattern[index] > 0):
                        EnergyBank += self.ChargingRate

                if EnergyBank < 0:
                # energy deficit happens, this action is not executable
                # return the index of this action
                    return i
        # return the index of this action
        return i+1




    def get_event_prob(self,x,paramsID,flag=0):
        # # x is the time, return its probability
        # if flag == 0:
        #     arg = self.params[:-2]
        #     loc = self.params[-2]
        #     scale = self.params[-1]
        #     return self.distribution.pdf(x, loc=loc, scale=scale, *arg)
        # else:
        #     arg = self.params_new[:-2]
        #     loc = self.params_new[-2]
        #     scale = self.params_new[-1]
        #     return self.distribution_new.pdf(x, loc=loc, scale=scale, *arg)

        if flag == 0:
            loc = self.params[paramsID][0]
            scale = self.params[paramsID][1]
            return self.distribution.pdf(x,loc=loc,scale=scale)
        else:
            # to be continued
            print('error!!!')
            exit(1)


    # def set_HarvEnPattern(self, start=6, end=18):
    # # energy source is harvestable in [start,end)
    # # set [start,end) to 1, 1 means there is available energy to be harvested
    #     self.HarvEnPattern.fill(0)
    #     for idx in range(0, self.TotalTime):
    #         self.HarvEnPattern[idx] = 1



    def gene_Event(self):
    ### use self.params to generate event pattern
    ### each element in data means an event happens in this specific time unit


        ### refill with 0
        self.EventPattern.fill(0)

        ### select event type
        params = self.params[self.eventtype]


        ### use the proper distribution function to generate data points
        if self.eventtype == 0:
            ### norm
            # parameters: [loc, scale, size]
            samples = st.norm.rvs(loc=params[0], scale=params[1], size=params[2])

        elif self.eventtype == 1:
            ### uniform
            # parameters: [loc,scale,size]
            samples = st.uniform.rvs(loc=params[0], scale=params[1], size=params[2])


        elif self.eventtype == 2:
            ### lognorm, peak in the left side
            # parameters: [loc,s,scale,size]
            samples = st.lognorm.rvs(loc=params[0], s=params[1], scale=params[2],size=params[3])

        elif self.eventtype == 3:
            ### loggamma, peak in the right side
            # parameters: [c,local,scale,size]
            samples = st.loggamma.rvs(c=params[0], loc=params[1], scale=params[2], size=params[3])



        for elem in samples:
            temp = int(elem)
            for idx in range(temp, temp + self.EventDuration):

                if idx < self.TotalTime:
                    self.EventPattern[idx] += 1


    def gene_Event_valid(self):
    # for evaluation, generate new data instance everytime

        ### new version, generate events from the newly sampled data points
        self.EventPattern_valid.fill(0)

        ### select event type
        params = self.params[self.eventtype]

        ### use the proper distribution function to generate data points
        if self.eventtype == 0:
            ### norm
            # parameters: [loc, scale, size]
            samples = st.norm.rvs(loc=params[0], scale=params[1], size=params[2])

        elif self.eventtype == 1:
            ### uniform
            # parameters: [loc,scale,size]
            samples = st.uniform.rvs(loc=params[0], scale=params[1], size=params[2])


        elif self.eventtype == 2:
            ### lognorm
            # parameters: [loc,s,scale,size]
            samples = st.lognorm.rvs(loc=params[0], s=params[1], scale=params[2],size=params[3])

        elif self.eventtype == 3:
            ### loggamma
            # parameters: [c,local,scale,size]
            samples = st.loggamma.rvs(c=params[0], loc=params[1], scale=params[2], size=params[3])

        for elem in samples:
            temp = int(elem)
            for idx in range(temp, temp + self.EventDuration):

                if idx < self.TotalTime:
                    self.EventPattern_valid[idx] += 1



    def evaluation(self,V_high,save = 0,flag = 0,num = 10):
    ### this is the fixed charging-recharging scenario where capacitor powers up the syste once it is charged to V_high
    ### and drain all of its energy in one time, then recharging again
    ### we calculate the number of captured event objects, the bigger the better

        ##############################################
        #####         Fixed simulation          ######
        ##############################################
        ### calculate average value of multiple experiments for FIXED simulation
        positive_wakeupB_hist = []
        positive_wakeupM_hist = []
        wakeup_hist = []
        EnergyBank_hist = []
        TotalHarvestedEnergy_hist = []

        filename = 'results_[{},{}]_sigma={}_{}.txt'.format(self.StateDuration,
                   self.EventDuration, self.params[0][1], self.actions)


        ### control the fixed sensing frequency, for example, once the system starts sensing after the high_threshold is reached
        ### how frequently it senses, e.g. sense once in every 1s or 2s or 3s or 4s, etc..
        # V_high = 90
        sensingfreq = 1     # if this is n, it means sense once in every n seconds
        sensingcounter = 0  #

        #### get the waking up time idx
        RunFlag = False
        EnergyBank = 0
        TotalHarvestedEnergy = 0
        self.SimuFIXED.fill(0)
        for time in range(self.TotalTime):
            if RunFlag == True and EnergyBank - self.DischargingRate >= 0:
                # the system starts sensing

                if sensingcounter % sensingfreq == 0:  # sensing once in every [sensingfreq] seconds
                    EnergyBank -= self.DischargingRate
                    self.SimuFIXED[time] = 1  # log wake up time
                else:  # else we do not sense and continue charging
                    EnergyBank += self.ChargingRate
                    TotalHarvestedEnergy += self.ChargingRate  # log total harvested energy

                ### increase the counter
                sensingcounter += 1

            else:  # RunFlag == False or left energy is not enough
                # in sleep mode, harvest energy
                RunFlag = False
                if (self.HarvEnPattern[time] > 0):
                    EnergyBank += self.ChargingRate
                    TotalHarvestedEnergy += self.ChargingRate  # log total harvested energy
            if EnergyBank >= V_high:
                # once energy level reaches V_th, wake up the MCU
                RunFlag = True
                sensingcounter = 0
        np.set_printoptions(precision=1)
        idx_base = [i for i, v in enumerate(self.SimuFIXED > 0) if v]

        TotalHarvestedEnergy_hist.append(TotalHarvestedEnergy)


        ### we repeat the experiment num times
        ### actually, for Fixed system, the event detection rate is proptional to the ratio of charging to discharging.
        ### for example, if the ratio is 10, the Fixed system should only be able to detect 1/10 of the total events
        for i in range(20):

            rand = np.random.randint(V_high)  # generate a random number
            idx = [e+rand for e in idx_base if e+rand < self.TotalTime]

            # regenerate event every time
            self.gene_Event_valid()

            if flag == 0:  # training
                positive_wakeupB = [i for i in self.EventPattern_valid[idx] if i > 0].__len__()
                positive_wakeupM = sum([i for i in self.EventPattern_valid[idx] if i > 0])
            else:  # testing online learning
                positive_wakeupB = [i for i in self.EventPattern_new[idx] if i > 0].__len__()
                positive_wakeupM = sum([i for i in self.EventPattern_new[idx] if i > 0])


            positive_wakeupB_hist.append(positive_wakeupB)
            positive_wakeupM_hist.append(positive_wakeupM)
            wakeup_hist.append(self.SimuFIXED.sum())



        output = ("SimuFIXED  ", 
               "#CatchB={:>4}".format(sum(positive_wakeupB_hist)/len(positive_wakeupB_hist)),   ### CatchB: catches in Binary, only one type of event
               "#CatchM={:>4}".format(sum(positive_wakeupM_hist) / len(positive_wakeupM_hist)),
               "Wake={:>5}".format(sum(wakeup_hist)/len(wakeup_hist)),
               "Harvested={:>6}".format(sum(TotalHarvestedEnergy_hist)/len(TotalHarvestedEnergy_hist)))
        # print output

        ### output the number of catches of each run
        # output = ("positive_wakeupB_hist={}".format(positive_wakeupB_hist))
        # print output

        ### for return value
        returnValue = np.zeros(10) # 10 values in the result for each event type
        returnValue[0] = sum(positive_wakeupB_hist)/len(positive_wakeupB_hist)
        returnValue[1] = sum(positive_wakeupM_hist) / len(positive_wakeupM_hist)
        returnValue[2] = sum(wakeup_hist)/len(wakeup_hist)

        if save == 1:
                f = open(filename,'a+')  # append new results to the file
                f.write(str(output))
                f.write('\r')
                f.close()


        ##############################################
        #####        dynamic simulation         ######
        ##############################################

        # using the training data to evaluate dynamic method

        positive_wakeupB_hist_test = []  # for testing on the data used for training
        positive_wakeupM_hist_test = []
        idx = [i for i, v in enumerate(self.SimuDynamic > 0) if v]
        positive_wakeupB_test = [i for i in self.EventPattern[idx] if i].__len__()
        positive_wakeupM_test = sum([i for i in self.EventPattern[idx] if i])
        positive_wakeupB_hist_test.append(positive_wakeupB_test)
        positive_wakeupM_hist_test.append(positive_wakeupM_test)


        #### using the newly generated data to evaluate dynamic method
        positive_wakeupB_hist = []
        positive_wakeupM_hist = []

        ### for groundtruth
        positive_wakeupB_hist_groundtruth = []
        positive_wakeupM_hist_groundtruth = []
        for shift in range(10):

            # regenerate event every time
            self.gene_Event_valid()

            idx = [i for i, v in enumerate(self.SimuDynamic > 0) if v]

            if flag == 0:  # training
                positive_wakeupB = [i for i in self.EventPattern_valid[idx] if i].__len__()
                positive_wakeupM = sum([i for i in self.EventPattern_valid[idx] if i])
                ### for groundtruch
                positive_wakeupB_groundtruth = [i for i in self.EventPattern_valid if i].__len__()
                positive_wakeupM_groundtruth = sum([i for i in self.EventPattern_valid if i])
            else:   # testing online learning
                positive_wakeupB = [i for i in self.EventPattern_new[idx] if i > 0].__len__()
                positive_wakeupM = sum([i for i in self.EventPattern_new[idx] if i > 0])
                ### for groundtruch
                positive_wakeupB_groundtruth = [i for i in self.EventPattern_new if i > 0].__len__()
                positive_wakeupM_groundtruth = sum([i for i in self.EventPattern_new if i > 0])


            positive_wakeupB_hist.append(positive_wakeupB)
            positive_wakeupM_hist.append(positive_wakeupM)
            ### for groundtruth
            positive_wakeupB_hist_groundtruth.append(positive_wakeupB_groundtruth)
            positive_wakeupM_hist_groundtruth.append(positive_wakeupM_groundtruth)


        output = ("SimuDynamic",
                  "#CatchB={:>4}".format(sum(positive_wakeupB_hist)/len(positive_wakeupB_hist)),
                  "#CatchM={:>4}".format(sum(positive_wakeupM_hist) / len(positive_wakeupM_hist)),
                  "Wake={:>5}".format(self.SimuDynamic.sum()),
                  "Harvested={:>6}".format(self.TotalHarvestedEnergy),
                  "Left={}".format(self.EnergyBank),
                  "OverCharge={}".format(self.OverCharge),
                  "Train#pic={}".format(sum(positive_wakeupB_hist_test)/len(positive_wakeupB_hist_test)))
        # print output

        ### output the number of catches of each run
        # output = ("positive_wakeupB_hist={}".format(positive_wakeupB_hist))
        # print output

        returnValue[3] = sum(positive_wakeupB_hist)/len(positive_wakeupB_hist)
        returnValue[4] = sum(positive_wakeupM_hist) / len(positive_wakeupM_hist)
        returnValue[5] = self.SimuDynamic.sum()
        returnValue[6] = returnValue[3] / returnValue[5] * 100   # energy efficiency, postive waking up ratio


        if save == 1:
            f = open(filename, 'a+')
            f.write(str(output))
            f.write('\r')
            f.close()


        ##############################################
        #####           ground truth            ######
        ##############################################

        # positive_wakeupB = [i for i in self.EventPattern if i > 0].__len__()
        # positive_wakeupM = sum([i for i in self.EventPattern if i > 0])
        output = ("GroundTruth",
                  "#CatchB={:>4}".format(sum(positive_wakeupB_hist_groundtruth) / len(positive_wakeupB_hist_groundtruth)),
                  "#CatchM={:>4}".format(sum(positive_wakeupM_hist_groundtruth) / len(positive_wakeupM_hist_groundtruth)),
               "Wake={:>5}".format(self.TotalTime),
               "ConsumedEn={:>6}".format(self.TotalTime * self.DischargingRate),
               "Har-ableEn={}".format((self.HarvEnPattern * self.ChargingRate).sum()))
        # print output

        returnValue[7] = sum(positive_wakeupB_hist_groundtruth) / len(positive_wakeupB_hist_groundtruth)
        returnValue[8] = sum(positive_wakeupM_hist_groundtruth) / len(positive_wakeupM_hist_groundtruth)
        returnValue[9] = self.TotalTime


        if save == 1:
            f = open(filename, 'a+')
            f.write(str(output))
            f.write('\r')
            f.close()



        output = ("StateDuration={}".format(self.StateDuration),
                  "EventDuration={}".format(self.EventDuration),
                  "Reward=[{},{},{}]".format(self.reward_catch,self.reward_nocatch,self.reward_cap),
                  "Fixed={}".format(V_high),"EnBankCap={}".format(self.EnergyBankMax),
                  "Actions={}".format(self.actions),
                  "Params={}".format(self.params))
                  # "SlideWindow={}".format(self.reward_slide_window_flag))
        # print output

        if save == 1:
            f = open(filename, 'a+')
            f.write(str(output))
            f.write('\r*********************************************************************************************************************\r')
            f.close()

        return returnValue

    def draw_actions(self):
    # each element in action_hist_hist is [actions_in_one_episode, episode_id]
    # show actions in order to verify if the algorithm has converged or not
        action_hist_hist = self.action_hist


        plt.figure()
        # if action_hist_hist.__len__() > 6:
        #     ### if more than 6 episodes were logged
        #     bai = action_hist_hist.__len__()/3 + 1
        #     subplot_pos =  bai * 100 + 30 + 1
        # else :
        #     ### otherwise
        #     subplot_pos = 321

        if action_hist_hist.__len__() > 6:
            idx = [0,1,2,3,4,action_hist_hist.__len__()-1]
        else:
            idx = range(0,action_hist_hist.__len__())

        subplot_pos = 321

        for i in range(idx.__len__()):
            action_hist = (action_hist_hist[idx[i]])[0]
            episode = (action_hist_hist[idx[i]])[1]

            x = np.linspace(0, self.TotalTime, action_hist.__len__())

            ax = plt.subplot(subplot_pos+i)
            plt.title('Actions: episode-{},SR-{}'.format(episode,self.StateDuration))

            plt.scatter(x, action_hist)

            # ### draw the event distribution
            # params = self.params[self.eventtype]
            # # for j in range(len(self.params)):
            # x = np.linspace(0, self.TotalTime, 100)
            # pdf_fitted = self.distribution.pdf(x, loc=self.params[j][0], scale=self.params[j][1])
            # pdf_fitted *= (float)((self.action_space.n - 1) / np.max(pdf_fitted))
            # if j == 1:
            #     plt.plot(x, pdf_fitted, 'r',label=str(action_hist_hist[i][2]))
            # else:
            #     plt.plot(x, pdf_fitted, 'r')
            # legend = ax.legend(loc='upper right', shadow=True, fontsize='x-large')
        plt.subplots_adjust(top=0.925,
            bottom=0.065,
            left=0.06,
            right=0.95,
            hspace=0.435,
            wspace=0.2)
        plt.show()

        output = ("StateDuration={}".format(self.StateDuration),
                  "EventDuration={}".format(self.EventDuration),
                  "Reward=[{},{},{}]".format(self.reward_catch, self.reward_nocatch, self.reward_cap),
                  "EnMax={}".format(self.EnergyBankMax),
                  "Actions={}".format(self.actions))
        print output




    def render(self, mode='human'):

        # draw harvestable energy pattern
        x = np.linspace(0, self.TotalTime, self.TotalTime)
        plt.figure()
        plt.title("HarEnPattern")
        plt.xlabel('Time')
        plt.ylabel('Availability of harvestable energy')
        plt.plot(x, self.HarvEnPattern)
        plt.xlim(0, self.TotalTime)
        plt.show()

        # draw event pattern
        x = np.linspace(0, self.TotalTime, self.TotalTime)
        indice = np.nonzero(self.EventPattern > 0)
        indice_zero = np.nonzero(self.EventPattern == 0)
        plt.figure()
        plt.title("EventPattern")
        plt.xlabel('Time')
        plt.ylabel('Event occurrence')
        plt.scatter(x[indice], self.EventPattern[indice]>0 )
        plt.scatter(x[indice_zero], self.EventPattern[indice_zero],color='r')
        plt.xlim(0,self.TotalTime)
        plt.ylim(0,np.max(self.EventPattern))
        plt.show()


        ###  switch to NORM distribution
        for i in range(len(self.params)):
            x = np.linspace(0,self.TotalTime,100)
            pdf_fitted = self.distribution.pdf(x,loc=self.params[i][0],scale=self.params[i][1])
            pdf_fitted *= (float)( (np.max(self.EventPattern)-1) / np.max(pdf_fitted) )
            plt.plot(x,pdf_fitted,'r')
        for i in range(len(self.params)):
            plt.figure()
            for j in range(10):
                x = self.distribution.rvs(loc=self.params[i][0], scale=self.params[i][1], size=self.params[i][2])
                y = np.ones_like(x)
                y.fill(j)
                plt.scatter(x, y)
            plt.title("Params[{}]".format((self.params[i])))
            plt.xlabel('Time')
            plt.ylabel('Instance')
            plt.show()


        # if self.new_dist_flag != 0:
        #     # draw new event pattern
        #     x = np.linspace(0, self.TotalTime , self.TotalTime)
        #     indice = np.nonzero(self.EventPattern_new > 0)
        #     plt.figure()
        #     plt.title("EventPattern - New")
        #     plt.xlabel('Time')
        #     plt.ylabel('Animal occurrence')
        #     plt.scatter(x[indice], self.EventPattern_new[indice])
        #     plt.xlim(0,self.TotalTime)
        #     plt.ylim(0,np.max(self.EventPattern_new))
        #     plt.show()
        #
        #     # draw new event distribution plot
        #     arg = self.params_new[:-2]
        #     loc = self.params_new[-2]
        #     scale = self.params_new[-1]
        #     x = np.linspace(0, self.TotalTime, 100)
        #     pdf_fitted = self.distribution_new.pdf(x, loc=loc, scale=scale, *arg)
        #     pdf_fitted *= (float)(np.max(self.EventPattern_new)) / np.max(pdf_fitted)  # scale fitted to action_number
        #     # plt.figure()
        #     # plt.title(self.distribution_new.name + " - Distribution Fitting - New")
        #     # plt.xlabel('Time')
        #     # plt.ylabel('Probability')
        #     # plt.xlim(0, 24)
        #     plt.plot(x, pdf_fitted,'y')


    # def gene_New1(self, shift):
    #
    #     self.EventPattern_new.fill(0)
    #
    #     # load raw event data from excel file, prepare discrete dataset for training Q_learning
    #     self.distribution_new = st.alpha
    #     animal = self.animal  # which animal you want to study in the excel file
    #     # read animal watering data from excel
    #     wb = xlrd.open_workbook(
    #         "E:/Bitbucket/research_related_yubo/Yubo's_first_paper_to_be/wildlife_watering_pattern.xlsx")
    #     sheet = wb.sheet_by_index(0)
    #     freq = np.asarray((sheet._cell_values[animal])[1:-1], dtype=int)
    #     clock = np.arange(1, 25)
    #
    #     self.data_new = np.asarray([], dtype=int)
    #     for idx, elem in enumerate(clock):
    #         for repeat in range(freq[idx]):
    #             if elem - shift < 0:
    #                 self.data_new = np.append(self.data_new, elem)
    #             else:
    #                 self.data_new = np.append(self.data_new, elem-shift)
    #
    #     # DISTRIBUTION = [st.alpha]
    #
    #     # for distribution in DISTRIBUTION:
    #     self.params_new = self.distribution_new.fit(self.data_new)
    #
    #     for elem in self.data_new:
    #         temp = np.random.randint((elem - 1) * 3600, elem * 3600 - self.EventDuration)
    #         for idx in range(temp, temp + self.EventDuration):
    #             # self.EventPattern_new[idx] += 1
    #             self.EventPattern_new[idx] = 1





    def run(self,qtable,episode=999):
        # run one perion based on current qtable

        state = self.reset() 
        done = False
        action_hist = []
        for step in range(self.observation_space.n):

            limit = self.actionlimit()
            action = self.get_max(qtable[state, :]) # action = np.argmax(qtable[state,:limit])
            new_state,reward,done,info = self.step(action)
            action_hist.append(action)
            state = new_state
            if done == True:
                break

        idx = [i for i, v in enumerate(self.SimuDynamic > 0) if v]
        self.action_hist.append([action_hist, episode,
                        [j for j in self.EventPattern
                        [idx]
                         if j].__len__(),
                        0])


    def run_RLEnergy(self,qtableRLEvent,qtableRLEnergy,EnergyLevelNum,StateCoverageNum,episode=999,energyshift=0):
        # run one perion based on current qtable

        stateRLEvent = self.reset()
        done = False
        action_hist = []

        # shift = np.random.randint(low=-400, high=400)
        # shitf = -200
        self.EnergyBank += energyshift

        StepPos = -1  # reset StepPos every iteration
        flag = 0
        for step in range(self.observation_space.n):

            ### for RLEvent Model, it must be in working mode here, so we just pickup the highest probable action
            actionRLEvent = np.argmax(qtableRLEvent[stateRLEvent, :2])  # actionRLEvent list has only two actions

            if actionRLEvent > 0:
            ### if action1 is picked up in RLEvent, then we need to start RLEnergy model
                ### change action according to RLEnergy Model
                EnergyLevel = self.FindEnergyLevel(EnergyLevelNum)  # get the current energy level
                StepPos += 1  # move forward by one step
                stateRLEnergy = StepPos * EnergyLevelNum + EnergyLevel

                actionRLEnergy = np.argmax(qtableRLEnergy[stateRLEnergy, :self.actions.__len__()])
                action = actionRLEnergy

                ### for CTIDpro version of DCOSS, make action fixed
                action = 3  # if not for CTIDpro, comment this line

                ### print out the entry energy level
                if flag == 0:
                    # print(EnergyLevel, self.EnergyBank)
                    flag = 1
            else:
                action = actionRLEvent


            # limit = self.actionlimit()
            # action = self.get_max(qtable[state, :]) # action = np.argmax(qtable[state,:limit])


            new_stateRLEvent,reward,done,info = self.step(action)

            if actionRLEvent > 0:
            ### if action1 is picked up in RLEvent, then we need to start RLEnergy model
                ### find the next stateRLEnergy
                if StepPos < StateCoverageNum - 1:
                    EnergyLevel = self.FindEnergyLevel(EnergyLevelNum)  # get the current energy level
                    new_stateRLEnergy = (StepPos+1) * EnergyLevelNum + EnergyLevel
                else :
                    new_stateRLEnergy = new_stateRLEnergy


            action_hist.append(action)
            stateRLEvent = new_stateRLEvent
            if done == True:
                break


        idx = [i for i, v in enumerate(self.SimuDynamic > 0) if v]
        self.action_hist.append([action_hist, episode,
                        [j for j in self.EventPattern
                        [idx]
                         if j].__len__(),
                        0])

        return action_hist


    def reward_slide_window(self,reward,WinLength=10):

        for i in range(WinLength):
            self.reward_window[i] = self.reward_window[i+1]

        self.reward_window[i] = reward

        return sum(self.reward_window[:WinLength])/WinLength



    def test_dist(self,value):
    # Nirjon suggested to sample data from the distribution and then
    # use the sampled data to train the model
    # this function is to visualize the sampled data

    # "value" means the number of instances

        arg = self.params[:-2]
        loc = self.params[-2]
        scale = self.params[-1]

        plt.figure()
        for i in range(0,value,1):
            # size is the number of sampled points
            a = self.distribution.rvs(*arg, loc=loc, scale=scale, size=100)
            b = np.ones_like(a)
            b.fill(i)
            plt.scatter(a, b)

        plt.show()


    def get_max(self,actions):

        temp = actions[0]
        idx = 0
        for i in range(1,actions.__len__()):
            if actions[i] > temp:   ### >:conservative way; >=:bold way
                temp = actions[i]
                idx = i


        return idx

    ##### output qtalbe to text which we can directly copy to CCS/MSP430
    def OutputQtableintoText(self,qtable):

        f = open("qtable\qtable.txt", 'w')
        f.write('{')
        [m, n] = qtable.shape
        for i in range(m):
            if i != 0:
                f.write(' {')
            else:
                f.write('{')

            for j in range(n):
                f.write('{0:.2f}'.format(qtable[i][j]))
                if j != (n - 1):
                    f.write(', ')

            if i != (m - 1):
                f.write('},\n')

        f.write('}};')

        f.close()

    def IfConverged_RLEnvent(self, qtable_old, qtable,state_visited_positive):
    ### check if qtable has converged


        res = 0

        list = []  ## log the index of positive states
        for i in range(self.DischargingRate ,(qtable.shape)[0]):

            ### if there is no change of polarity, product should be positive
            # product = (qtable[i][0] - qtable[i][1]) * (qtable_old[i][0] - qtable_old[i][1])
            # if(product <= 0):  ### there is a change of polarity, or a1 has no yet been visited
            #     return -1

            if(qtable[i][1] == 0): # it is converged if every state has been visited; once visited, its value will be non-zero
                return -1

            if(qtable[i][1]>0): ### log the index of positive states
                list.append(i)

        # ### check if there is only one peak
        # if list[-1] - list[0] != list.__len__() - 1: # we assume there is one continous peak
        #     print('Multiple peaks exist in qtable')


        ### if it comes here, it reaches the first-step of return; all states have been visited
        res = 1

        ### check if all positive states have been visisted by multiple times
        positive_state_visited_requirement = 1  # define how many times we should visit each positive state
        positive = state_visited_positive[np.nonzero(state_visited_positive)]
        if np.sum(positive) / positive.__len__() >= positive_state_visited_requirement:
            res = 2


        return res


    def IfConverged_RLEnergy(self,qtableRLEnergy_old, qtableRLEnergy,MarkVisitedState,EnergyLevelNum, episode):
    ### check if RLEnergy model has converged by comparing the current and previous RLEnergy qtables
    ### NEW: we also need to check if all actions of one state are all explored

        ChangedNCount = 0  # count how many elements that have changed in the qtable
        ChangeVar = 0

        ### when the model is close to convergence, it may only have small changes, e.g. from 3 to 6 but by our definition of big change, (6-3)/3 =  1 is still big
        ### so we need to rule out this scenario, we only regard it as a big change when the value of old and new is bigger than a number, e.g. 10, 20
        IfValueBig = 0


        for i in range(qtableRLEnergy.shape[0]):
            for j in range(qtableRLEnergy.shape[1]):

                ### if this element has changed
                if qtableRLEnergy[i][j] != qtableRLEnergy_old[i][j]:
                    ChangedNCount += 1
                    ### use (old - new)/old to show how much it has changed, in case the denominator is zero
                    ChangeVar += abs(qtableRLEnergy[i][j] - qtableRLEnergy_old[i][j]) / abs(qtableRLEnergy_old[i][j]+0.00000001)
                    # print("{}-->{}".format(qtableRLEnergy_old[i][j],qtableRLEnergy[i][j]))

                    if abs(qtableRLEnergy[i][j]) > 20 or abs(qtableRLEnergy_old[i][j] > 20):
                        IfValueBig = 1

        if ChangedNCount != 0:
            AveragedChangeVar = ChangeVar / ChangedNCount
        else:
            AveragedChangeVar = 0


        ### in DCOSS2021, we do not check if all actions are all visited
        flag = 0  # show if there is an action has not been explored yet in the state
        for i in range(MarkVisitedState.__len__()):
            if MarkVisitedState[i] == 1 : # it means this state has been visited
                ### then we need to check if all actions of this state have been explored
                ### (1) for final step states
                if i >= qtableRLEnergy.shape[0] - EnergyLevelNum:
                    start = 1
                ### (2) for non-final step states
                else :
                    start = 0
                for j in range(start,qtableRLEnergy.shape[1]):
                    if qtableRLEnergy[i][j] == 0: # if found one action that has not been explored yet
                        flag = 1
                        break

            if flag  == 1:  ## there exists non-explored action
                break


        ### show convergence progress
        # print("episode = {}: Changed = {} / AveragedVar = {}, Flag={}".format(episode,ChangedNCount,AveragedChangeVar,flag))

        ### DCOSS2021, we do not check if all actions are all visited

        if AveragedChangeVar < 1 or IfValueBig == 0 :#and flag == 0:  # flag = 0 means all actions of visited states have been explored
            return 1
        else:
            return  0







    def FindEnergyLevel(self,EnergyLevelNum):
    ### RLEnergy Model needs to locate the state according to the current energy level
    ### energy level can be identified by self.EnergyBank
        # thresholds = [15,30,45,60,75,90,115,120]  # thresholds.len = EnergyLevelNum - 1

        if EnergyLevelNum == 5:
            self.ELthreshold = [28,  42,  60, 90]  # thresholds.len = EnergyLevelNum - 1
        elif EnergyLevelNum == 8:
            self.ELthreshold = [14, 28, 35, 49, 60, 80, 100]
        elif EnergyLevelNum == 10:
            self.ELthreshold = [14, 21, 28, 35, 42, 49, 60, 80, 100]
        elif EnergyLevelNum == 15:
            self.ELthreshold = [12,17,22,27,32,37,42,47,52,57,62,67,75,95]  




        flag = 0
        for i in range(len(self.ELthreshold)):
            if self.EnergyBank < self.ELthreshold[i]*self.DischargingRate:
                EnergyLevel = i
                flag = 1
                break

        if flag == 0:
            EnergyLevel = len(self.ELthreshold)

        return  EnergyLevel


    def getRealQtable(self):
    ### using the current self.EventPattern to calculate a qtable withtout considering energy limit
    ### this func is used in training, when we want to check if the learnt qtable learns the real environment well
    ### RealQtable = env.getRealQtable()
        RealQtable = np.zeros((self.observation_space.n,1))

        for i in range(self.observation_space.n):
            temp = self.EventPattern[i*self.StateDuration:(i+1)*self.StateDuration]
            positive = (temp[np.nonzero(temp)]).__len__()
            negative = self.StateDuration - positive
            reward = positive * self.reward_catch + negative * self.reward_nocatch
            RealQtable[i] = reward

        return RealQtable



    def rearrange_pval_table(self,pval_table,qtableRLEnergy,EnergyLevelNum):
    ### in order to accelerate the convergence speed, we want to increase the probability of picking up the unvisited actions
    ### so at the end of each iteration, we rearrange the pval_table according to the current qtableRLEnergy

        ### for non-final-step states
        for i in range(qtableRLEnergy.shape[0]-EnergyLevelNum):
            indexOfZero = np.nonzero(qtableRLEnergy[i]==0)[0]  # find out the index of zero value
            indexOfNonZero = np.nonzero(qtableRLEnergy[i])[0]  # find out the index of non zero value
            if indexOfZero.__len__() == qtableRLEnergy.shape[1]: # none of the actions has been explored
                continue
            elif indexOfZero.__len__() == 0 : # all actions have been explored
                pval_table[i] = [1/(float)(qtableRLEnergy.shape[1])]*qtableRLEnergy.shape[1]
            else: # rearragement is needed
                pval_table[i][indexOfZero] = [1/(float)(indexOfZero.__len__())]*indexOfZero.__len__() # re-distriute the probability over the un-explored actions,
                pval_table[i][indexOfNonZero] = 0 # explored action will have zero-prob

        ### for final-step states
        for i in range(qtableRLEnergy.shape[0] - EnergyLevelNum,qtableRLEnergy.shape[0]):
            indexOfZero = np.nonzero(qtableRLEnergy[i] == 0)[0]  # find out the index of zero value
            indexOfNonZero = np.nonzero(qtableRLEnergy[i])[0]  # find out the index of non zero value
            indexOfZero = indexOfZero[indexOfZero > 0]           # delete the index of a0
            indexOfNonZero = indexOfNonZero[indexOfNonZero > 0]  # delete the index of a0
            if indexOfZero.__len__() == qtableRLEnergy.shape[1] - 1 : # none of the actions has been explored
                continue
            elif indexOfZero.__len__() == 0:  # all actions have been explored
                pval_table[i][1:] = [1 / (float)(qtableRLEnergy.shape[1]-1)] * (qtableRLEnergy.shape[1]-1)
            else:  # rearragement is needed
                pval_table[i][indexOfZero] = [1 / (float)(indexOfZero.__len__())] * indexOfZero.__len__() # re-distriute the probability over the un-explored actions,
                pval_table[i][indexOfNonZero] = 0 # explored action will have zero-prob
