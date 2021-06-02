from scipy.spatial.distance import pdist, squareform
from sklearn.metrics.pairwise import pairwise_distances
import numpy as np

s = 35

N = s*s*s
coord = np.zeros((N,3))
charge = np.zeros(N)

n = 0
for i in range(s):
    for j in range(s):
        for k in range(s):
            coord[n,0] = i
            coord[n,1] = j
            coord[n,2] = k
            charge[n] = 0.33
            n += 1

# Matrix routines are memory prohibitive !
dist=pdist(coord)
# Fast, but not multithreaded !
dist = pairwise_distances(X = coord, metric = 'euclidean', n_jobs = 4)
# Multithreaded but memory prohibitive !
