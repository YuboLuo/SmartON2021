import numpy as np


filename = "data/audio/test_result"
data_raw = np.fromfile(filename, dtype='int16')

data = np.asarray((data_raw),dtype='int')
data = data.reshape((len(data)//2,2))

table = {}
for elem in data:

    if len(table) > 1 and elem[0] == 0:
        break


    if elem[0] not in table:
        table[elem[0]] = [1,1,0] if elem[1] > 1 else [1,0,1]
    else:
        table[elem[0]][0] += 1
        table[elem[0]][1] += 1 if elem[1] > 1 else 0
        table[elem[0]][2] += 0 if elem[1] > 1 else 1

exclude = []  # exclude defected data

res_rate = []  # record the detection rate, positive detections / total detections
res_num = []   # record the number of positive detections
res_total = [] # record the total number of detections
for i in table:
    if i not in exclude:
        rate = table[i][1]/table[i][0]
        res_rate.append(rate)
        res_num.append(table[i][1])
        res_total.append(table[i][0])
        print(i,table[i], '{:.2f}'.format(rate))
    else:
        print(i, table[i], '{:.2f} deleted'.format(table[i][1]/table[i][0]))

print('\nAveraged results:'
      '\nPositive detections: {:.1f}'
      '\nTotal detections: {:.1f}'
      '\nPositive detection rate {:.3f}'
      .format(np.mean(res_num),np.mean(res_total),np.mean(res_rate)))