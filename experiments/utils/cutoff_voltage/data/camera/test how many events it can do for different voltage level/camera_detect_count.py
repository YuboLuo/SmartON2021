import numpy as np

cutoff = np.fromfile('data/camera_detect_log_2.3V-2.0V_12mf12mf', dtype='uint16')


a = cutoff.reshape((int(cutoff.shape[0]/2),2))

b = [ e for e in a  if e[0]>2000 and e[0] != 4095]
b.sort(key=lambda x:x[0])
b = np.asarray((b))



