/* Parallel n-Body simulation using Pthread with quadrant_tree */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include "quadrant_tree.h"

//#define Num_Pthreads 4         /* number of threads*/
#define N    1000              /* number of particles */
#define G    6                /* gravity constant 6.673e-11 */
#define timeslot 0.002        /* time slot of one movement */
#define totalstep 2000        /* total simulation step */
#define X_RESN 800            /* X resolution */
#define Y_RESN 800            /* Y resolution */

int X_L = 3*X_RESN / 8.0;     /* X lower bound for particle initial local */
int X_U = 5*X_RESN / 8.0;     /* X upper bound for particle initial local */
int Y_L = 3*Y_RESN / 8.0;     /* Y lower bound for particle initial local */
int Y_U = 5*Y_RESN / 8.0;     /* Y upper bound for particle initial local */

int X_L_M = 2*X_RESN / 8.0;   /* X lower bound for particle movement */
int X_U_M = 6*X_RESN / 8.0;   /* X upper bound for particle movement */
int Y_L_M = 2*Y_RESN / 8.0;   /* Y lower bound for particle movement */
int Y_U_M = 6*Y_RESN / 8.0;   /* Y upper bound for particle movement */

struct body * bodies;
int result[totalstep][N][3] = {0};
int Num_Pthreads;

struct thread_data
{
    int id;
    struct tree * rootnode;
};

static void *update_force(void *thread) {

    struct thread_data *my_data;
    my_data = (struct thread_data *) thread;
    int i;

    for (i = 0; i < N; i++) {
        if (bodies[i].active == 1 && my_data->id == i % Num_Pthreads) {
            Calculate_force(my_data->rootnode, bodies+i, G, 0.5);
        }
    }

}

int main(int argc, char const *argv[])
{
    sscanf(argv[1], "%d", &Num_Pthreads);
    int i, j;
    struct timespec start_time, end_time;

    /* Initialize bodies in all processors to store N body */
    bodies = malloc(N*sizeof(struct body));
    for (i = 0; i < N; i++) {

        srand(time(0) + rand());
        bodies[i].m = rand() % (1000 - 900) + 900;

        srand(time(0) + rand());
        bodies[i].vx = 0;

        srand(time(0) + rand());
        bodies[i].vy = 0;

        srand(time(0) + rand());
        bodies[i].x = rand() % (X_U - X_L) + X_L;

        srand(time(0) + rand());
        bodies[i].y = rand() % (Y_U - Y_L) + Y_L;

        bodies[i].fx = 0;
        bodies[i].fy = 0;
        bodies[i].active = 1;

    }

    clock_gettime(CLOCK_MONOTONIC, &start_time);

    for (int current_step = 0; current_step < totalstep; current_step++) {
        pthread_t threads[Num_Pthreads];
        struct thread_data td[Num_Pthreads];
        int rc;

        struct tree * rootnode;
        double xmin = X_L_M, xmax = X_U_M, ymin = Y_L_M, ymax = Y_U_M;

        /* reset force to be 0 at every iteration */
        for (i = 0; i < N; i++) {
            if (bodies[i].active == 1) {
                bodies[i].fx = 0;
                bodies[i].fy = 0;
            }
        }

        rootnode = Create_TreeNode(bodies, xmin, xmax, ymin, ymax);

        for (i = 1; i < N; i++) {
            if (bodies[i].active == 1) {
                Insert_Body(bodies+i, rootnode);
            }
        }

        for (i = 0; i < Num_Pthreads; i++) {
            td[i].id = i;
            td[i].rootnode = rootnode;
            rc = pthread_create(&threads[i], NULL, update_force, (void *)&td[i]);
            if (rc) {
                printf("Error: unable to create thread, %d\n", rc);
                exit(-1);
            }
        }

        for (i = 0; i < Num_Pthreads; i++) {
            rc = pthread_join(threads[i], NULL);
            if (rc) {
                printf("Error: unable to join thread, %d\n", rc);
                exit(-1);
            }
        }

        for (i = 0; i < N; i++) {
            if (bodies[i].active == 1) {
                bodies[i].vx += timeslot*bodies[i].fx / bodies[i].m;
                bodies[i].vy += timeslot*bodies[i].fy / bodies[i].m;

                bodies[i].x += timeslot*bodies[i].vx;
                bodies[i].y += timeslot*bodies[i].vy;

                if (bodies[i].x < X_L_M || bodies[i].x > X_U_M) {
                    bodies[i].vx = -bodies[i].vx;
                }

                if (bodies[i].y < Y_L_M || bodies[i].y > Y_U_M) {
                    bodies[i].vy = -bodies[i].vy;
                }
                
                result[current_step][i][0] = bodies[i].active;
                result[current_step][i][1] = (int)bodies[i].x;
                result[current_step][i][2] = (int)bodies[i].y;
            }
        }
        Destroy_Tree(rootnode);
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    printf("Maximum iteration: %d.\n", totalstep);
    printf("Number of bodies: %d.\n", N);
    printf("The total time for calculation is %f s.\n", (end_time.tv_sec - start_time.tv_sec) + 
                                            (end_time.tv_nsec - start_time.tv_nsec)/1000000000.0);

    Display  *disp;
	Window   win;
	XEvent   evt;
        int  scr;
        GC   gc;
	Pixmap   pm;

    disp = XOpenDisplay(NULL);
	if (disp==NULL) {
		fprintf(stderr,"Cannot open display\n");
		return 1;
	}

	scr = DefaultScreen(disp);
	win = XCreateSimpleWindow(disp,RootWindow(disp,scr),
				  10,10,X_RESN,Y_RESN,
				  1,WhitePixel(disp,scr),
				  BlackPixel(disp,scr));
	XSelectInput(disp,win,ExposureMask|KeyPressMask);
	XMapWindow(disp,win);
	gc = DefaultGC(disp,scr);
	pm = XCreatePixmap(disp,win,X_RESN,Y_RESN,DefaultDepth(disp,scr));
	XFillRectangle(disp,pm,gc,0,0,X_RESN,Y_RESN);
	XSetForeground(disp,gc,WhitePixel(disp,scr));
    
    for (i = 0; i < totalstep; i++) {
        char str[30];
        sprintf(str, "Iteration: %d", i);
        XSetForeground(disp,gc,0);
        XFillRectangle(disp,pm,gc,0,0,X_RESN,Y_RESN);
        XSetForeground(disp,gc,WhitePixel (disp, scr));
        XDrawString(disp, pm, gc, 25, 25, str, strlen(str)); 
        for (j = 0; j < N; j++) {
            if (result[i][j][0] == 1)
                XSetForeground(disp,gc, 0xFFFFFF/totalstep * (totalstep - j));
                XDrawPoint (disp, pm, gc, result[i][j][2], result[i][j][1]);
        }
        XCopyArea(disp,pm,win,gc,0,0,X_RESN,Y_RESN,0,0);
    }

    XFreePixmap(disp,pm);
    XCloseDisplay(disp);
    free(bodies);
    return 0;
}
