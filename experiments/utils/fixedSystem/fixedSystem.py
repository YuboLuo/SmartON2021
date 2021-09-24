import numpy as np

filename = 'audio\detectionfreq=1_ET0_97'
# filename = 'camera\detectionfreq=1_ET3'


cutoff = np.fromfile(filename, dtype='uint16')
print('filename = {}'.format(filename))

a = cutoff.reshape((int(cutoff.shape[0]/2),2))

len(np.where(a[:,1]==9)[0])

print('Total positive detection (10 rounds): {}'.format(len(np.where(a[:,1]==9)[0])))
print('Total detections in 20 minutes: {}'.format(len(np.where(a[:,1]!=0)[0])))
print('Positive rate: {}'.format( len(np.where(a[:,1]==9)[0])/10/len(np.where(a[:,1]!=0)[0]) ))





