#include "models.h"
#include "display.h"
#include <mpi.h>

int iteration, x;
TemperatureField *field;
TemperatureField *tempField, *swapField;

int dx[4] = { 0, -1, 0, 1 };
int dy[4] = { 1, 0, -1, 0 };

int size;

void temperature_iterate(int start, int size) {
	int i, j, d;
	for (i = start; i < start + size; i++) {
		if (legal(i, field->X_range)) {
			if (legal(i, field->X_range)) /* check for i over X_range */
			for (j = 0; j < field->Y_range / 2 + 1; j++) {
				int cnt = 0;
				tempField->t[i][j] = 0;
				for (d = 0; d < 4; ++d) {
					if (legal(i + dx[d], field->X_range) && legal(j + dy[d], field->Y_range)) {
						tempField->t[i][j] += field->t[i + dx[d]][j + dy[d]];
						++cnt;
					}
				}
				tempField->t[i][j] /= cnt;
				tempField->t[i][field->Y_range - j] = tempField->t[i][j];
			}
		}
	}
}

int main(int argc, char **argv){

    //XInitThreads();
	FILE * file;
	int i;

	int num_processor, rank;
	double start_time, end_time;
	start_time = MPI_Wtime();
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &num_processor);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (argc<3) {
	    printf("Usage: %s x iteration\n", argv[0]);
    }
    sscanf(argv[1], "%d", &x);
    sscanf(argv[2], "%d", &iteration);

    field = malloc(sizeof(TemperatureField));
    tempField = malloc(sizeof(TemperatureField));
    newField(field, x, x, 0, 0);
    newField(tempField, x, x, 0, 0);
    initField(field);

	if (rank == 0) {
        XWindow_Init(field);
	}

	size = (field->X_range % num_processor == 0) ? field->X_range / num_processor : field->X_range / num_processor + 1;
    int start = rank * size;
    int iter;
	for (iter = 0; iter < iteration; iter++) {
		temperature_iterate(start, size);
        MPI_Allgather(&(tempField->t[start][0]), size*field->Y_range*2, MPI_FLOAT, &(field->t[0][0]), size*field->Y_range*2, MPI_FLOAT, MPI_COMM_WORLD);

		if (rank == 0) {
            for(i = x * 0.3; i < x * 0.7; i++)
			    field->t[0][i] = FIRE_TEMP;
            XRedraw(field);
		}
	} 

    if (rank == 0) {
        end_time = MPI_Wtime();
		double totaltime = end_time - start_time;
		char str[100];
		sprintf(str, "HD_MPI_#Processor:%d_#x:%d_#iter:%d.txt", num_processor, x, iter);
		printf("The total time for calculation is %f s.\n", totaltime);
		file = fopen(str, "a");
		fprintf(file, "%f\n", totaltime);
		fclose(file);
	}
    
    MPI_Finalize();
    return 0;
}
