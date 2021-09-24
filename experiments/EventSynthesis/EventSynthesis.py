'''
we use this file to generate events according to the a specific event distribution
we have 4 distributions as shwon in the paper
the generated events are saved in a textfile containing an array of 0,1 , 1 = event-ON, 0 = event-OFF
each binary number represents one time unit which is one second in our case

We use an Arduino to read the textfile and then play the events by an LED(image-based) or a Buzzer(audio-based)
in the image-based experiments, 1 = power up the LED; 0 = power down the LED. Out system uses a camera to catch the LED blinking events
in the sound-based experiments, 1 = play a sound with a specific frequency; 0 = no sound. Out system uses an audio sensor to catch the sound events
'''


import scipy.io.wavfile
import numpy as np
import sounddevice
import time

import scipy.stats as st
import matplotlib.pyplot as plt
import sys

# draw scatter plot to show the event distribution of params
class EventVisualization():

    def __init__(self,timespace=1200,ED=1,eventtype=0):  # ED=event duration
        self.TotalTime = timespace # how many second
        self.EventDuration = ED
        self.EventPattern = np.zeros((self.TotalTime,),dtype=int)  # cumulated event strength for each time unit
        self.EventPattern_boolen = np.zeros((self.TotalTime,), dtype=int) # for binary mode
        self.signal_Final = []
        self.plt = []
        self.UnitEventSignal = []

        ### in the baseline, there are how many time units where an event is happening
        self.TotalCatchesBinary = 0      # binary mode, eventON or eventOFF
        self.TotalCatchesMultiple = 0    # multiple mode, differnt events have different scores

        self.datapoint = 100
        self.params = [[600, 19, self.datapoint], [540, 120, self.datapoint], [600 - 65, 0.5, 33, self.datapoint],
                       [0.4, 660 - 12, 8, self.datapoint]]  # norm; uniform; lognorm ; loggamma


        self.eventtype = eventtype

    def gene_Event(self,showplot=0):

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

        ### use fill function to clear data
        self.EventPattern.fill(0)         # for mulitple mode
        self.EventPattern_boolen.fill(0)  # for binary mode
        for elem in samples:
            temp = (int)(elem)
            if temp < 0 or temp >= self.TotalTime: # filter out outliers
                continue

            for idx in range(temp,temp+self.EventDuration):
                self.EventPattern[idx] += 1          # for mulitple mode
                self.EventPattern_boolen[idx] = 1    # for binary mode

        ### plot the distribution
        x = np.linspace(0,self.TotalTime,100)
        if self.eventtype == 0:
            pdf_fitted = st.norm.pdf(x, loc=params[0], scale=params[1])
        elif self.eventtype == 1:
            pdf_fitted = st.uniform.pdf(x, loc=params[0], scale=params[1])
        elif self.eventtype == 2:
            pdf_fitted = st.lognorm.pdf(x, loc=params[0], s=params[1], scale=params[2])
        elif self.eventtype == 3:
            pdf_fitted = st.loggamma.pdf(x, c=params[0], loc=params[1], scale=params[2])


        if showplot > 0:

            plt.figure()
            plt.plot(x, pdf_fitted, 'r')


            plt.figure()
            plt.hist(samples)


            plt.figure(dpi=600)
            plt.scatter(range(len(self.EventPattern)),self.EventPattern)


            plt.figure(dpi=600)
            plt.scatter(range(len(self.EventPattern_boolen)),self.EventPattern_boolen)

            plt.show()


        ### in the baseline, there are how many time units where an event is happening
        self.TotalCatchesBinary = np.count_nonzero(self.EventPattern)      # binary mode, eventON or eventOFF
        self.TotalCatchesMultiple = np.sum(self.EventPattern)      # differnt event has different scores

        '''
        you can return either EventPattern or EventPattern_boolen
        EventPattern: the system can distinguish different event strengthes (event1, event2, event3,etc.)
        EventPattern_boolen: the system can only tell eventON or eventOFF
        '''
        return self.EventPattern

    def generate_test_sound_single_freq(self,freq,period=500):
        '''
        generate a test audio sound,
        input: freq of the sound that you want to generate;
        return: the generated audio sound
        '''


        # period = 750; # length of the audio file(in second)
        duration = 1;
        fs = 8000;
        values = np.linspace(1 / fs, duration, fs * duration, endpoint=True)
        audio_frame = np.sin(2 * np.pi * freq * values)

        audio = np.zeros_like(audio_frame)
        for i in range(period):
            audio = np.concatenate((audio, audio_frame))

        return audio,fs

    def generate_test_sound_all_freq(self):
        '''
        generate an audio which contains all supported frequencies
        '''

        ### first, we generate unit-time audio signal for different events
        N = 5; # up to N different events
        fs = 8000;  # sampling frequency
        duration = 1;  # how many unit time, 1 = 1second
        UnitEventSignal = np.zeros((N+1, fs)) # we store the generated unit-time Event signal here, including the non_event signal
        for i in range(1, N+1):
            freq = 900 + (i-1) * 100;  # 500Hz, 800Hz, ...
            values = np.linspace(1 / fs, duration, fs * duration, endpoint=True)
            UnitEventSignal[i] = np.sin(2 * np.pi * freq * values)


        audio = np.zeros_like(UnitEventSignal[0])
        ### concatenate
        for i in range(100):
            if i <20:
                audio = np.concatenate((audio, UnitEventSignal[1]))
            elif i < 40:
                audio = np.concatenate((audio, UnitEventSignal[2]))
            elif i < 60:
                audio = np.concatenate((audio, UnitEventSignal[3]))
            elif i< 80:
                audio = np.concatenate((audio, UnitEventSignal[4]))
            else:
                audio = np.concatenate((audio, UnitEventSignal[5]))
        return audio, fs


    def audio_synthesis(self,Events):
        '''
        this funciton synthesizes the audio which is for experiments based on the input Events parameter
        :param Events: contains 0/1 information, each number represents one second, 0: eventOFF, 1:eventON
        :return: the synthesized audio and fs
        '''

        ### first, we generate unit-time audio signal for different events
        N = 5; # up to N different events
        fs = 8000;  # sampling frequency
        duration = 1;  # how many unit time, 1 = 1second
        self.UnitEventSignal = np.zeros((N+1, fs)) # we store the generated unit-time Event signal here, including the non_event signal
        freqList = [1900,1900,1900,1900,1900];
        for i in range(0, N):
            freq = freqList[i]  #
            values = np.linspace(1 / fs, duration, fs * duration, endpoint=True)
            self.UnitEventSignal[i+1] = np.sin(2 * np.pi * freq * values)         #

        ### play the sound, check if the sould is correct
        # sounddevice.play(UnitEventSignal.flatten(), fs)


        # self.signal_Final = np.zeros_like(self.UnitEventSignal[0]) ### first initialize one time-unit


        ### concatenate
        for idx,elem in enumerate(Events):

            ### for the first elem, we can not use concatenate,
            if idx == 0:
                if elem > N:
                    self.signal_Final = self.UnitEventSignal[N]
                else:
                    self.signal_Final = self.UnitEventSignal[elem]
                continue

            ### generate different sounds for different types of events
            if elem > N:
                ### if elem is beyong our max_number of event types
                self.signal_Final = np.concatenate((self.signal_Final, self.UnitEventSignal[N]))
            else:
                self.signal_Final = np.concatenate((self.signal_Final, self.UnitEventSignal[elem]))


        return self.signal_Final,fs



    def textfile_synthesis(self,Events):
        '''
        for our image based experiments, we need to use Arduino to light up a LED as the event
        we use this function to generate a textfile from which the Arduino can read the event pattern
        :param Events: the event-happening information for the entire experiment period, e.g. 20 minutes
        :return: the output textfile will only contain the information of the peak of the event-happening which is short, e.g. 2 minutes
        '''

        filename = 'textfile/1.txt'
        with open(filename,'w') as file:
            file.write(Events[0])




count = 0

buffer = []
for i in range(100):

    env = EventVisualization(timespace=1200,ED=1,eventtype=3)
    Events = env.gene_Event(showplot=0)

    EventNumB = np.count_nonzero(Events)
    EventNumM = np.sum(Events)


    ### you can specify the number of eventB you want
    ### eventtype = 0/1/2/3; EventNumB = 54/68/50/52; for datapoint = 100
    OptimalNum = [54,68,50,52]


    # print('count={}; EventNumB={}'.format(count, EventNumB))
    if EventNumB == OptimalNum[env.eventtype]:
        Event_peak = Events[540:660]
        buffer.append(np.count_nonzero(Event_peak[:105])) # image [22,48,75,107];                 audio [18,52,77,105]
        print('count={}; EventNumB={}'.format(count, EventNumB))
        continue

    count += 1

print(np.mean(buffer[:5]))
print(buffer)
sys.exit()

Event_peak = Events[540:660]  # only output the peak of event-happening

dist = "[{},{},{},{}]".format(np.count_nonzero(Event_peak[:30]),np.count_nonzero(Event_peak[30:60]),np.count_nonzero(Event_peak[60:90]),np.count_nonzero(Event_peak[90:120]))
print(dist)


outfilename = "events\Testing\\type{}\#{}_ET={}_#DP={}_#EventB{}_{}.txt".format(env.eventtype,0,env.eventtype,env.datapoint,EventNumB,dist)

with open(outfilename, 'w') as file:
    output = Event_peak
    output[np.where(output!=0)[0]] = 1
    outstr = ','.join(str(e) for e in output)

    file.write('{'+outstr+'};')

for i in range(4):
    print(outstr[i*60:(i+1)*60])
print('saved in {}'.format(outfilename))




# env.textfile_synthesis(Events)

# # signal_Final,fs = env.audio_synthesis(Events)
#
# # Events = Events[540:660]
# # dist = "[{},{},{},{}]".format(np.count_nonzero(Events[:30]),np.count_nonzero(Events[30:60]),np.count_nonzero(Events[60:90]),np.count_nonzero(Events[90:120]))
# # print(dist)
# # signal_Final = signal_Final[540*fs:660*fs] # only save the 2-minute audio clip
# # ## sounddevice.play(signal_Final,fs)
# #
# # filename = "testing_wav\#{}_ET={}_#DP={}_#EventB{}_{}.wav".format(0,env.eventtype,env.datapoint,EventNumB,dist)
# # # filename = "testing_wav\TestAlwaysON.wav"
# #
# # scipy.io.wavfile.write(filename,fs,signal_Final)
# # print(filename)
# #
# # # ### generate testing audio
# # # env = EventVisualization()
# # # audio,fs = env.generate_test_sound_single_freq(1900)
# # # scipy.io.wavfile.write("FFT_testSingleFreq_1900Hz.wav",fs,audio)
# #
# #
# # # for i in [15,17,19,21,23]:
# # #     period = 30
# # #     audio,fs = env.generate_test_sound_single_freq(i*100,period=period)
# # #     sounddevice.play(audio,fs)
# # #     time.sleep(period)



