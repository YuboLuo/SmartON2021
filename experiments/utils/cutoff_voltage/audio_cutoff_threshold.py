import numpy as np

cutoff = np.fromfile('data/audio_cutoff_4.7mf12mf12mf', dtype='uint16')


a = cutoff.reshape((int(cutoff.shape[0]/2),2))

res = []
cnt = 0
flag = 0
for i in range(len(a)):

    if a[i,0] == 0:
        res.append(cnt)
        break

    if a[i,0] < 2000:
        continue

    flag = 0
    if i != 0 and a[i,0] > a[i-1,0] and a[i-1,0] < 2100 and a[i,0] > 2200:
        flag = 1


    if flag:
        res.append(cnt)
        cnt = 1
    else:
        cnt += 1



print(res)
print(np.mean(res))


