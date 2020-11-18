#include "models.h"
#include "display.h"
#include <pthread.h>

int iteration, x, num_thread;
TemperatureField *field;
TemperatureField *tempField, *swapField;

int dx[4] = {0, -1, 0, 1};
int dy[4] = {1, 0, -1, 0};

int size;
pthread_t *threads;

void *temperature_iterate(void *t)
{
	int i, j, d;
	int start = (int)t;
	for (i = start; i < start + size; i++) {
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
	//tempField->t[0][0] = 100.0f;
	// for(i = field->x * 0.3; i < field->x * 0.7; i++)
	// 	tempField->t[0][i] = 100.0f;
}

int main(int argc, char **argv)
{
    XInitThreads(); /* multiple threads draw */
    FILE *file;
    int i;

    if (argc<4)
	    printf("Usage: %s x y iteration\n", argv[0]);
    sscanf(argv[1], "%d", &x);
    sscanf(argv[2], "%d", &iteration);
	sscanf(argv[3], "%d", &num_thread);

    field = malloc(sizeof(TemperatureField));
    tempField = malloc(sizeof(TemperatureField));
    newField(field, x, x, 0, 0);
    newField(tempField, x, x, 0, 0);
    initField(field);
    XWindow_Init(field);

	struct timespec start_time, end_time;
	double totaltime;
	clock_gettime(CLOCK_MONOTONIC, &start_time);

	//threads = (pthread_t*)malloc(num_thread * sizeof(pthread_t));
	pthread_t threads[num_thread];

	size = (field->X_range % num_thread == 0) ? field->X_range / num_thread : field->X_range / num_thread + 1;

    int iter;
	for (iter = 0; iter < iteration; iter++) {

		for (i = 0; i < num_thread; i++) {
			int start = i * size;
			pthread_create(&threads[i], NULL, temperature_iterate, (void *)start);
		}

		for (i = 0; i < num_thread; i++)
			pthread_join(threads[i], NULL);

        for(i = field->X_range * 0.3; i < field->X_range * 0.7; i++)
			tempField->t[0][i] = 100;
		field = tempField;
		XRedraw(field);
	}

	clock_gettime(CLOCK_MONOTONIC, &end_time);
	totaltime = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;
	char str[100];
	sprintf(str, "HD_Pthread_#Thread:%d_#x:%d_#iter:%d.txt", num_thread, x, iter);
	printf("The total time for calculation is %f s.\n", totaltime);
	file = fopen(str, "a");
	fprintf(file, "%f\n", totaltime);
	fclose(file);

	//sleep(20);
	pthread_exit(NULL);
    return 0;
}
