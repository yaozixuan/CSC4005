/* MPI_dynamic Mandelbrot program */

#include "mpi.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#define         X_RESN  800       /* x resolution */
#define         Y_RESN  800       /* y resolution */
#define         chunk 1
// #define         MAX_CALCULATE_ITERATION  100
typedef struct complextype
        {
        float real, imag;
        } Compl;


int main (int argc, char * argv[])
{
    Window          win;                            /* initialization for a window */
    unsigned
    int             width, height,                  /* window size */
                    x, y,                           /* window position */
                    border_width,                   /*border width in pixels */
                    display_width, display_height,  /* size of screen */
                    screen;                         /* which screen */

    char            *window_name = "Mandelbrot Set", *display_name = NULL;
    GC              gc;
    unsigned
    long            valuemask = 0;
    XGCValues       values;
    Display         *display;
    XSizeHints      size_hints;
    
    XSetWindowAttributes attr[1];

    /* Mandlebrot variables */
    int i, j, k;
    Compl   z, c;
    float   lengthsq, temp;


    /* MPI variables */
    int numtasks, rank, len;
    double start, finish, totaltime;
    char hostname[MPI_MAX_PROCESSOR_NAME];
    int send_start, recv_start, send_finish;        /* Next execution start column */
    int stop = 0;       /* Need to be initialized to 0 */

    MPI_Init(&argc, &argv); // initialize MPI
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks); // get number of tasks
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // get my rank
    MPI_Get_processor_name(hostname, &len); // this one is obvious


    /* result buffer to store the calculated data (column + data) */
    int *result_buf = (int *)malloc(sizeof(int)*(Y_RESN + 1));;
    /* MPI_Send MPI_Recv status*/   
    MPI_Status status;      /* status.MPI_source tells the processor(rank) who sends the data (who you are going to send info back) when MPI_Recv is using MPI_ANY_SOURCE */
    /* Pass the global variable argument */ 
    int  MAX_CALCULATE_ITERATION;
    sscanf(argv[1], "%d", &MAX_CALCULATE_ITERATION);
    
    if (rank == 0){ //master
        
        /* connect to Xserver */

	    if (  (display = XOpenDisplay (display_name)) == NULL ) {
	       fprintf (stderr, "drawon: cannot connect to X server %s\n",
	                            XDisplayName (display_name) );
	    exit (EXIT_FAILURE);
	    }
	    
	    /* get screen size */

	    screen = DefaultScreen (display);
	    display_width = DisplayWidth (display, screen);
	    display_height = DisplayHeight (display, screen);

	    /* set window size */

	    width = X_RESN;
	    height = Y_RESN;

	    /* set window position */

	    x = 0;
	    y = 0;

	    /* create opaque window */

	    border_width = 4;
	    win = XCreateSimpleWindow (display, RootWindow (display, screen),
	                            x, y, width, height, border_width, 
	                            BlackPixel (display, screen), WhitePixel (display, screen));

	    size_hints.flags = USPosition|USSize;
	    size_hints.x = x;
	    size_hints.y = y;
	    size_hints.width = width;
	    size_hints.height = height;
	    size_hints.min_width = 300;
	    size_hints.min_height = 300;
	    
	    XSetNormalHints (display, win, &size_hints);
	    XStoreName(display, win, window_name);

	    /* create graphics context */

	    gc = XCreateGC (display, win, valuemask, &values);

	    XSetBackground (display, gc, WhitePixel (display, screen));
	    XSetForeground (display, gc, BlackPixel (display, screen));
	    XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);

	    attr[0].backing_store = Always;
	    attr[0].backing_planes = 1;
	    attr[0].backing_pixel = BlackPixel(display, screen);

	    XChangeWindowAttributes(display, win, CWBackingStore | CWBackingPlanes | CWBackingPixel, attr);

	    XMapWindow (display, win);
	    XSync(display, 0);
        /* End of screen initialization */  


	    int p = 0;
	    start = MPI_Wtime();
        for (p = 0; p < X_RESN/chunk + numtasks - 1; p++){
            // printf("\nnumber of tasks= %d my rank= %d \n", numtasks, rank);
            MPI_Recv(&result_buf[0], Y_RESN + 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            send_start = chunk * p;
            // printf("Process %d is available,", status.MPI_SOURCE);
            MPI_Send(&send_start, 1, MPI_INT, status.MPI_SOURCE, 1, MPI_COMM_WORLD);
            // printf("calculate from %d\n", send_start);

            for (int row = 1; row < Y_RESN+1; row++){
                k = result_buf[row];
                // printf("%d\n", result_buf[row]);
                XSetForeground(display, gc, 0xFFFFFF / MAX_CALCULATE_ITERATION * (MAX_CALCULATE_ITERATION - k));
                XDrawPoint (display, win, gc, result_buf[0], row);
                // if (k == MAX_CALCULATE_ITERATION) XDrawPoint (display, win, gc, result_buf[0], row);
            }
        }
        finish = MPI_Wtime();      
    }

    else { //slaves
        /* Calculate and draw points */
        send_finish = rank;     /* send the first message to activate the master, tell the master that I'm free (with the rank) */
        MPI_Send(&send_finish, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        while (stop == 0){
            // printf("\nnumber of tasks= %d my rank= %d \n", numtasks, rank);
            MPI_Recv(&recv_start, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
            // printf("start from %d.\n", recv_start);
            if (recv_start < X_RESN){
                for(i=recv_start; i < recv_start + chunk; i++){ //X_RESN
                    result_buf[0] = i;
                    for(j=0; j < Y_RESN; j++) {
                        z.real = z.imag = 0.0;
                        c.real = ((float) i - 400.0)/200.0;               /* scale factors for 800 x 800 window */
                        c.imag = ((float) j - 400.0)/200.0;
                        k = 0;

                        do{                                               /* iterate for pixel color */
                            temp = z.real*z.real - z.imag*z.imag + c.real;
                            z.imag = 2.0*z.real*z.imag + c.imag;
                            z.real = temp;
                            lengthsq = z.real*z.real+z.imag*z.imag;
                            k++;
                        } while (lengthsq < 4.0 && k <  MAX_CALCULATE_ITERATION);
                        result_buf[j+1] = k;
                        // printf("%d\n", result_buf[j+1]);
                    }
                }
                MPI_Send(&result_buf[0], Y_RESN + 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                // printf("Process %d finish.\n", result_buf[0]);
            }
            else{
                stop = 1;
                // printf("stop! %d", stop);
            }
        }        
    }  
    
    MPI_Finalize();// done with MPI    

    if (rank == 0){
        XFlush (display);    
        totaltime = (double)(finish - start);
        printf("\ntotal time : %f\n", totaltime);
        sleep (2);
    }
    return 0;
        /* Program Finished */
}
