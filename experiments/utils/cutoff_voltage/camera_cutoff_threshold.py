import numpy as np

cutoff = np.fromfile('data/cutoff', dtype='uint16')


a = cutoff.reshape((int(cutoff.shape[0]/2),2))

# b = [ e for e in a  if e[0]>2000 and e[0] != 4095]
# b.sort(key=lambda x:x[0])
# b = np.asarray((b))



