from mpi4py import MPI

commWorld = MPI.COMM_WORLD.Dup()
nbp = commWorld.size
rank = commWorld.rank

filename = f"Output{rank:03d}.txt"
out      = open(filename, mode='w')

if (rank == 0):
    jeton = 1
    commWorld.send(jeton, dest=1)
    jeton = commWorld.recv(source=nbp-1)
else:
    jeton = commWorld.recv(source=rank-1)
    jeton += 1
    commWorld.send(jeton, dest=(rank+1)%nbp)

out.write(f"Valeur du jeton : {jeton}")

out.close()

# mpirun -n 2 python3 circulation_jeton.py

'''
globCom = MPI.COMM_WORLD.Dup()
nbp     = globCom.size
rank    = globCom.rank
filename = f"Output{rank:03d}.txt"
out      = open(filename, mode='w')

if rank==0:
    jeton = 1
    globCom.send(jeton, dest=1)
    jeton = globCom.recv(source=nbp-1)
    jeton += 1
else:
    jeton = globCom.recv(source=rank-1)
    jeton += 1
    globCom.send(jeton,dest=(rank+1)%nbp)

out.write(f"Valeur du jeton : {jeton}")

out.close()

'''