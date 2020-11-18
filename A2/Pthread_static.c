/* Pthread_static Mandelbrot program */

#include <pthread.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define         X_RESN  800       /* x resolution */
#define         Y_RESN  800       /* y resolution */
// #define         Num_Pthreads  4
// #define         MAX_CALCULATE_ITERATION  100
typedef struct complextype
        {
        float real, imag;
        } Compl;


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


/* Global variables control Pthreads */
int Num_Pthreads;
int MAX_CALCULATE_ITERATION;
       

void *Calculate_Draw ( void *threadid ){
    /* Mandlebrot variables */
    int i = 0, j, k;
    Compl   z, c;
    float   lengthsq, temp;
    int tid;
    tid = (int) threadid;

    /* Calculate and draw points */

    for(i=X_RESN/Num_Pthreads*tid; i < X_RESN/Num_Pthreads*(tid+1); i++){
        for(j=0; j < Y_RESN; j++) {

          z.real = z.imag = 0.0;
          c.real = ((float) i - 400.0)/200.0;               /* scale factors for 800 x 800 window */
          c.imag = ((float) j - 400.0)/200.0;
          k = 0;

          do  {                                             /* iterate for pixel color */

            temp = z.real*z.real - z.imag*z.imag + c.real;
            z.imag = 2.0*z.real*z.imag + c.imag;
            z.real = temp;
            lengthsq = z.real*z.real+z.imag*z.imag;
            k++;

          } while (lengthsq < 8.0 && k < MAX_CALCULATE_ITERATION);

        // XSetForeground(display, gc, 0xFFFFFF / MAX_CALCULATE_ITERATION * (MAX_CALCULATE_ITERATION - k));
        // XDrawPoint (display, win, gc, i, j);                                      /* Colorful */
        if (k == MAX_CALCULATE_ITERATION) XDrawPoint (display, win, gc, i, j);       /* Black and White */
        }
    }     
}

int main (int argc, char * argv[])
{
     XInitThreads();

    /* connect to Xserver */

    if (  (display = XOpenDisplay (display_name)) == NULL ) {
       fprintf (stderr, "drawon: cannot connect to X server %s\n",
                            XDisplayName (display_name) );
    exit (-1);
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
    

    /* Pthread execution time collection*/
    struct timespec start, finish;
    double totaltime;
    clock_gettime(CLOCK_MONOTONIC, &start);

    /*  Pass the argument Num_Pthreads, MAX_CALCULATION_ITERATION, chunk.  */
    sscanf(argv[1], "%d", &Num_Pthreads);
    sscanf(argv[2], "%d", &MAX_CALCULATE_ITERATION);
    Num_Pthreads = Num_Pthreads < X_RESN ? Num_Pthreads : X_RESN;
    printf("Num_Pthreads: %d, MAX_CALCULATE_ITERATION: %d\n ", Num_Pthreads, MAX_CALCULATE_ITERATION);
    
    /*  create Num_Pthreads */
    pthread_t threads[Num_Pthreads];
    int rc;
    long p;
    for(p =0; p<Num_Pthreads; p++){
        rc = pthread_create(&threads[p], NULL, Calculate_Draw, (void*)p);
        if(rc){
            printf("ERROR: return code from pthread_create() is %d", rc);
            exit(1);
        }
    }
    
    for (p = 0; p < Num_Pthreads; p++) {
        pthread_join(threads[p], NULL);
    }
     
    XFlush (display);
    clock_gettime(CLOCK_MONOTONIC, &finish);
    totaltime =  finish.tv_sec - start.tv_sec + (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Totaltime: %f\n", totaltime);
    sleep (2);

    /* Program Finished */
    return 0;
}