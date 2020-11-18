//#include "const.h"
#include "models.h"
#include "display.h"

#define legal(x, n) ( (x)>=0 && (x)<(n) )

int iteration,X,Y;
TemperatureField *field;
TemperatureField *tempField, *swapField;

int dx[4] = {0, -1, 0, 1};
int dy[4] = {1, 0, -1, 0};

TemperatureField * temperature_iterate(TemperatureField *field)
{
	int i, j, d;
	for (i=0; i<field->X_range; ++i){
		for (j=0; j<field->Y_range / 2 + 1; ++j){
			int cnt = 0;
			tempField->t[i][j] = 0;
			for (d=0; d<4; ++d){
				if ( legal(i+dx[d], field->X_range) && legal(j+dy[d], field->Y_range) ) {
					tempField->t[i][j] += field->t[i+dx[d]][j+dy[d]];
					++cnt;
				}
			}
			tempField->t[i][j] /= cnt;
			tempField->t[i][field->Y_range - j] = tempField->t[i][j];
		}	
	}
	for (j = (int)(0.3*field->Y_range); j < (int)(0.7*field->Y_range); j++){
		tempField->t[0][j] = 100.0f;
	}
	return tempField;
}

int main(int argc, char **argv)
{
    FILE *file;
	
	if (argc<4)
    {
	    printf("Usage: %s X Y iteration\n", argv[0]);
    }
    sscanf(argv[1], "%d", &X);
    sscanf(argv[2], "%d", &Y);
    sscanf(argv[3], "%d", &iteration);

    field = malloc(sizeof(TemperatureField));
    tempField = malloc(sizeof(TemperatureField));
    newField(field, X, Y,0,0);
    newField(tempField, X, Y,0,0);
    initField(field);
    XWindow_Init(field);

    struct timespec start_time, end_time;
    double totaltime;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    int iter;
    for (iter=0; iter<iteration; iter++){
		tempField = temperature_iterate(field);
		//swapField = field;
		field = tempField;
		//tempField = swapField;
		//XRedraw(field);
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    totaltime = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;
	char str[100];
	sprintf(str, "HD_Sequential_#x:%d_#iter:%d.txt", X, iter);
	printf("The total time for calculation is %f s.\n", totaltime);
	file = fopen(str, "a");
	fprintf(file, "%f\n", totaltime);
	fclose(file);
    return 0;
}
