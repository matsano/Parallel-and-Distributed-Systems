# Produit matrice-vecteur v = A.u
import numpy as np
from mpi4py import MPI

globalComm = MPI.COMM_WORLD.Dup()
nbp = globalComm.size
rank = globalComm.rank

# Dimension du problème (peut-être changé)
dim = 120

nloc = dim//nbp
firstLin = rank*nloc

# Initialisation de la matrice
A_local = np.array([ [(i+j+firstLin)%dim+1. for j in range(dim)] for i in range(nloc) ])
print(f"A_local = {A_local}")

# Initialisation du vecteur u
u_local = np.array([i+1. for i in range(dim)])
print(f"u_local = {u_local}")

# Produit matrice-vecteur
v_local = A_local.dot(u_local)
print(f"v_local = {v_local}")

v = np.empty(dim, dtype=v_local.dtype)
globalComm.Allgather(v_local, v)
print(f"v = {v}")