# Produit matrice-vecteur v = A.u
import numpy as np
from mpi4py import MPI

globalComm = MPI.COMM_WORLD.Dup()
nbp = globalComm.size
rank = globalComm.rank

# Dimension du problème (peut-être changé)
dim = 120

#Nombre de colonnes local
nloc = dim//nbp
#Primeira coluna de cada bloco
firstLoc = nloc*rank

# Initialisation de la matrice
A_local = np.array([ [(i+j+firstLoc)%dim+1. for j in range(nloc)] for i in range(dim) ])
print(f"A_local = {A_local}")

# Initialisation du vecteur u
u_local = np.array([i+firstLoc+1. for i in range(nloc)])
print(f"u_local = {u_local}")

# Produit matrice-vecteur
v_local = A_local.dot(u_local)
print(f"v_local = {v_local}")

v = np.empty(dim, dtype=v_local.dtype)

#Funcao SUM eh a funcao par default do allreduce
globalComm.Allreduce(v_local, v)
print(f"v = {v}")