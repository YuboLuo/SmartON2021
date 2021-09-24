import numpy as np


filename = "data/audio/eventtype2/qtables_RLEnergy_EL12_Iter20-23"
data_raw = np.fromfile(filename, dtype='float32')

data = np.asarray((data_raw),dtype='int')
data = data.reshape((len(data)//4,4))


n_qtable = 10
n_state = 16
n_action = 4

### extract data
qtables = data[:160,:]
qtable = data[160:176,:]
rewards = data[176:186,:]
actions = data[186:196,:]
states = data[196:206,:]
volts = data[206:216,:]
SWs = data[216:226,:]
Wakingups = data[226:236,:]
ELs = np.asarray(states%4) # will be deducted from states



for i in reversed(range(10)):
    if qtables[(i+1)*n_state - 1][0] != 0:
        print('Iter = {}\nConvergedCnt = {}'.format(qtables[(i+1)*n_state - 1][0],qtables[(i+1)*n_state - 2][0]))
        break

### check which elem has changed in the qtable
qtables = np.hstack((qtables,np.zeros_like(qtables)))
qtables[n_state - 1, 0] *= 1000
for i in range(1,n_qtable):  # we save 10 qtables in a row
    # skip the first qtable

    for j in range(n_state):  # we have 16 states in each qtable

        if j == 15:
            qtables[i * n_state + j, 0] *= 1000


        for k in range(n_action):  # we have 4 actions

            ### avoid the slots where we saved important information
            if (j == 15 or j == 14) and k == 0:
                # j == 15 and k == 0, it has the SavedQtableCnt info
                # SavedQtableCnt means how many iterations have been done so far
                # j == 14 and k == 0, it has the SumOfConvergedResult info
                # SumOfConvergedResult means how many times the converge condition is met consecutively
                continue



            if qtables[i * n_state + j, k] != qtables[( i - 1 ) * n_state + j, k]:
                qtables[i * n_state + j, n_action + 0] = 9999  # mark there is a change
                qtables[i * n_state + j, n_action + 1] = qtables[( i - 1 ) * n_state + j, k] # old value
                qtables[i * n_state + j, n_action + 2] = qtables[i * n_state + j, k] # new value
                break  # there is at maximum one change for each row




IfGenFlag = 1
qtable_save = np.copy(qtable)
if IfGenFlag:
    filename = 'qtable.txt'
    m,n = qtable_save.shape

    # clear the old file
    f = open(filename,'w')
    f.write('{')
    for i in range(m):
        for j in range(n):

            if j == 0:
                if i == 0:
                    f.write('{')
                else:
                    f.write(' {')





            if j == n - 1:
                f.write('{0: >5}'.format(qtable_save[i][j]))
                if i != m - 1:
                    f.write('},\n')
                else:
                    f.write('}')

            else:

                f.write('{0: >5},'.format(qtable_save[i][j]))

    f.write('};')
    f.close()


