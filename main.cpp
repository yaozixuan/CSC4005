#include "mpi.h"
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <time.h>

using namespace std;
 
int swap(int *num, int i, int j);

int main(int argc, char * argv[]) {
	string argvi(argv[1]);
	int size = atoi(argv[1]);
	int *array_in = (int *)malloc(sizeof(int)*size); // array in
	int *array_out = (int *)malloc(sizeof(int)*size); // array out
	int local_size, local_size_max, rem;
	int recv_right, send_left, send_right, recv_left;
	int numtasks, rank, len;
	double start, finish;
	double totaltime;
	char hostname[MPI_MAX_PROCESSOR_NAME];
	
	MPI_Init(&argc, &argv); // initialize MPI
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks); // get number of tasks
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); // get my rank
	MPI_Get_processor_name(hostname, &len); // this one is obvious

	//Generate random array
	for (int i = 0; i < size; i++) {
		array_in[i] = ((int)rand()) % 100;
		//printf("%d ", array_in[i]);
	}

	start = MPI_Wtime();

	rem = size % numtasks;
	local_size = size / numtasks;
	local_size_max = local_size;
	if (local_size % 2 == 1) { // odd numbers in each processor
		local_size += 1;
		local_size_max = local_size;
	}
	else { // even numbers in each processor
		if (rank == numtasks - 1) {
			local_size_max = local_size + rem; 
		}
	}

	printf("\nnumber of tasks= %d my rank= %d local_size= %d running on %s \nLocal array: ", numtasks, rank, local_size_max, hostname);
	
	int *local_array = (int *)malloc(sizeof(int)*local_size);  // local array
	for (int i = 0; i < local_size_max; i++) { //MPI_Scatter(&array_in, local_size_max, MPI_INT, local_array, local_size_max, MPI_INT, 0, MPI_COMM_WORLD);
		if (local_size*rank + i < size) {
			local_array[i] = array_in[local_size*rank + i];
		}
		else{ // padding
			local_array[i] = 100; //1000 actually should be the max of array
		}
		printf("%d ", local_array[i]);
	}

	if (rank == 0) {
		printf("\n\nName : Yao Zixuan\n");
		printf("ID : 115010267\n");
		printf("Sorting %d random numbers using %d processors\n", size, numtasks);

		srand((int)time(NULL)); // random seed
		printf("Random Numbers Generating... \n\nOrignal array: ");
		for (int i = 0; i < size; i++) {
			printf("%d ", array_in[i]);
		}
		printf("\n\nSorting...\n");
	}
	
	for (int i = 1; i < size+1; i++) {
		//printf("\n Iteration%d  ", i);
		if (i % 2 == 1) { // odd iteration
			for (int k = 0; k < local_size_max / 2; k++) { // local_size_max odd, but no action
				if (local_array[2 * k] > local_array[2 * k + 1]){
					swap(local_array, 2 * k, 2 * k + 1);
				}
			}	
		}
		else { // even iteration
			if (rank == numtasks-1) {
				send_left = local_array[0];
				MPI_Send(&send_left, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD);
				//printf("send_left: %d  ", send_left);
			}
			else if (rank != 0){
				MPI_Recv(&recv_right, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				send_left = local_array[0];
				MPI_Send(&send_left, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD);
				//printf("recv_right: %d, send_left: %d  ", recv_right, send_left);
			}
			else if (rank == 0){
				if (numtasks != 1) {
					MPI_Recv(&recv_right, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					//printf("recv_right: %d  ", recv_right);
				}
			}

			for (int k = 0; k < local_size_max / 2; k++) { // local_size_max odd
				if (local_size_max % 2 != 0) { // rank == numtasks - 1
					if (local_array[2 * k + 1] > local_array[2 * k + 2]) {
						swap(local_array, 2 * k + 1, 2 * k + 2);
					}
				}
				else {
					if (k < (local_size_max / 2) - 1) {
						if (local_array[2 * k + 1] > local_array[2 * k + 2]) {
							swap(local_array, 2 * k + 1, 2 * k + 2);
						}
					}
					else {
						if (rank != numtasks - 1) {
							if (local_array[2 * k + 1] > recv_right) {
								send_right = local_array[2 * k + 1];
								local_array[2 * k + 1] = recv_right;
							}
							else {
								send_right = recv_right;
							}
						}
					}
				}	
			}

			if (rank == 0) {
				if (numtasks != 1) { // only one processor
					MPI_Send(&send_right, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
					//printf("send_right: %d  ", send_right);
				}
			}
			else if (rank != numtasks - 1) {	
				MPI_Recv(&recv_left, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				local_array[0] = recv_left;
				MPI_Send(&send_right, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
				//printf("recv_left: %d, send_right: %d  ", recv_left, send_right);
			}
			else if (rank == numtasks - 1) {
				MPI_Recv(&recv_left, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				local_array[0] = recv_left;
				//printf("recv_left: %d  ", recv_left);
			}
		}
		
		if (i == size - 1) {
			printf("\nOutput array: ");
			for (int i = 0; i < local_size_max; i++) {
				printf("%d ", local_array[i]);
			}
		}
	}

	int *send_rem = (int *)malloc(sizeof(int)*rem);
	int *recv_rem = (int *)malloc(sizeof(int)*rem);
	MPI_Gather(local_array, local_size, MPI_INT, array_out, local_size, MPI_INT, 0, MPI_COMM_WORLD);
	if (((size / numtasks) % 2 == 0) && (rem > 0)) {  //rem still need to send
		if (rank == numtasks - 1) {
			for (int i = 0; i < rem; i++) {
				send_rem[i] = local_array[local_size + i];
			}
			MPI_Send(send_rem, rem, MPI_INT, 0, 0, MPI_COMM_WORLD);
		}
		else if (rank == 0) {
			MPI_Recv(recv_rem, rem, MPI_INT, numtasks - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			for (int i = 0; i < rem; i++) {
				array_out[size - rem + i] = recv_rem[i];
			}
		}
	}

	finish = MPI_Wtime();

	MPI_Finalize();// done with MPI
	if (rank == 0) {
		printf("\nSorted array: ");
		for (int j = 0; j < size; j++) {
			printf("%d ", array_out[j]);
		}
		totaltime = (double)(finish - start);
		printf("\ntotal time : %f\n", totaltime);
	}
	return 0;
}

int swap(int *num, int i, int j) {
	int tmp = num[i];
	num[i] = num[j];
	num[j] = tmp;

	return 0;
}
