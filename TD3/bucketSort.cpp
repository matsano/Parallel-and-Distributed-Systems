#include <iostream>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <mpi.h>

#define MIN 1
#define MAX 10

std::ostream& operator << (std::ostream& aStream, const std::vector<int>& numbers)
{
    for (auto it: numbers)
        aStream << it << " / ";
    return aStream;
}

std::vector<int> getMedians(std::vector<int> list, int nb){
    std::vector<int> medians;
    int numDiv = 0;
    for (int i = int(list.size()/nb); i < list.size() && numDiv < nb-1; i += int(list.size()/nb)){
        medians.push_back(list[i]);
        numDiv ++;
    }
    return medians;
}

int getBucket(int element, std::vector<int> med){
    for (int i = 0; i < med.size(); i++){
        if (element < med[i]){
            return i;
        }
    }
    return med.size();
}

void getSizeBuckets(std::vector<std::vector<int>> buckets, std::vector<int> &sizeBuckets){
    for (int i = 0; i < buckets.size(); i++){
        sizeBuckets[i] = buckets[i].size();
    }
}

int getSum(int* vector, int limit){
    int sum = 0;
    for (int i = 0; i < limit; i++){
        sum += vector[i];
    }
    return sum;
}


int main(int argc, char *argv[]){

    MPI_Init(&argc, &argv);
    int nbTasks;
    int myRank;
    MPI_Comm_size(MPI_COMM_WORLD, &nbTasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Barrier(MPI_COMM_WORLD);


    //Liste de nombres aleatoires
    srand(time(NULL));
    int lenght = 420;
    int lenght_local = lenght/nbTasks;

    std::vector<int> numbers;
    if (myRank == 0){
        for (int i = 0; i < lenght; i++){
            numbers.push_back(rand()%100);
        }
        std::cout << "not ordered = " << numbers << std::endl;
        std::cout << " " << std::endl;
    }
    

    //Creation de listes locales
    std::vector<int> localNumbers (lenght_local);
    
    MPI_Scatter(numbers.data(), lenght_local, MPI_INT, localNumbers.data(), lenght_local, MPI_INT, 0, MPI_COMM_WORLD);
    std::sort(localNumbers.begin(), localNumbers.end());

    //Obtenir les medianes
    std::vector<int> mediansLocal = getMedians(localNumbers, nbTasks);

    //Envoyer au processus 0 et trier les medianes
    std::vector<int> medians(mediansLocal.size()*nbTasks);
    MPI_Gather(mediansLocal.data(), mediansLocal.size(), MPI_INT, medians.data(), mediansLocal.size(), MPI_INT, 0, MPI_COMM_WORLD);
    if (myRank == 0){
        std::sort(medians.begin(), medians.end());
        mediansLocal = getMedians(medians, nbTasks);
    }

    //Envoie les medianes choisies a d'autres processus
    MPI_Bcast(mediansLocal.data(), mediansLocal.size(), MPI_INT, 0, MPI_COMM_WORLD);

    //Separer les nombres en buckets
    std::vector<std::vector<int>> buckets(nbTasks);
    for (int i = 0; i < localNumbers.size(); i++){
        buckets[getBucket(localNumbers[i], mediansLocal)].push_back(localNumbers[i]);
    }

    //Fusionner les buckets de chaque processus dans leurs processus respectifs
    std::vector<std::vector<int>> sizeBuckets(nbTasks);
    for (int i = 0; i < nbTasks; i++){
        sizeBuckets[i].resize(nbTasks);
    }
    getSizeBuckets(buckets, sizeBuckets[myRank]);
    
    //Partage la taille des buckets de chaque processus avec les autres processus
    for (int i = 0; i < nbTasks; i++){
        MPI_Bcast(sizeBuckets[i].data(), sizeBuckets[i].size(), MPI_INT, i, MPI_COMM_WORLD);
    }

    //Détermine la somme des tailles de bucket
    int sizeToSend[nbTasks][nbTasks];
    for (int i = 0; i < nbTasks; i++){
        for (int j = 0; j < nbTasks; j++){
            sizeToSend[j][i] = sizeBuckets[i][j];
        }
    }

    int displs[nbTasks][nbTasks];
    for (int i = 0; i < nbTasks; i++){
        for (int j = 0; j < nbTasks; j++){
            if (j == 0){
                displs[i][j] = 0;
            } else{
                displs[i][j] = getSum(sizeToSend[i], j);
            }
        }
    }

    //Determine le nouvel emplacement de chaque processus
    std::vector<int> newLocal(getSum(sizeToSend[myRank], nbTasks));
    for (int i = 0; i < nbTasks; i++){
        MPI_Gatherv(buckets[i].data(), buckets[i].size(), MPI_INT, newLocal.data(), sizeToSend[i], displs[i], MPI_INT, i, MPI_COMM_WORLD);
    }

    //Ordonner ce nouvel emplacement
    std::sort(newLocal.begin(), newLocal.end());

    //Retour au processus 0
    int sizeAllBuckets[nbTasks];
    for (int i = 0; i < nbTasks; i++){
        sizeAllBuckets[i] = getSum(sizeToSend[i], nbTasks);
    }
    int sizeAllDispls[nbTasks];
    for (int i = 0; i < nbTasks; i++){
        if (i == 0){
            sizeAllDispls[i] = 0;
        } else{
            sizeAllDispls[i] = getSum(sizeAllBuckets, i);
        }
    }
    MPI_Gatherv(newLocal.data(), newLocal.size(), MPI_INT, numbers.data(), sizeAllBuckets, sizeAllDispls, MPI_INT, 0, MPI_COMM_WORLD);

    if (myRank == 0){
         std::cout << "ordered = " << numbers << std::endl;
    }

    MPI_Finalize();

    return 0;


}
