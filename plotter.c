// This program uses RK4 with constant stepsize to integrate an ODE starting from a grid of points in a range.
// It takes min and max x and y to specify the range, numbers of rows and columns for the grid, and the stepsize.
// If told 1 row and 1 column, the points to integrate from will be arranged like this in the range:
//    x----x----x
//    -         -
//    -         -
//    x    x    x
//    -         -
//    -         -
//    x----x----x
// For 0 rows and 0 cols the only basepoints will be the corners of the range.
// If a basepoint is on the left edge of the range, it need only be integrated forward. Similarly for the right edge.
// A basepoint between the left and right edge is integrated forward and backward in parallel, creating two files.
// Each curve is integrated until it leaves the range.
//
// Assumptions this program makes:
// a dir called curves exists in the current dir and is writeable
// the definition of FUNCTION is a valid expression
// the arguments to the program are within reasonable expectations
//
// Sometimes it crashes with a large number of basepoints or small stepsize. It may work if you run it again.

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include "rk.h"

#define FUNCTION x + y

static double xmin, xmax, ymin, ymax;
static unsigned int rows, cols;
static double stepsize;

struct solution_id {
    double x;
    double y;
    unsigned int row;
    unsigned int col;
};

double f (double x, double y) {
    return FUNCTION;
}

void* integrate_forward (void* basepoint) {
    struct solution_id* id = basepoint;
    FILE* output;
    char filename[50];

    sprintf (filename, "curves/curve_%d_%df.dat", id->row, id->col);
    output = fopen (filename, "w");

    rk (&f, id->x, id->y, xmin, xmax, ymin, ymax, stepsize, output);

    fclose (output);
    free (basepoint);
    pthread_exit(NULL);
}

void* integrate_backward (void* basepoint) {
    struct solution_id* id = basepoint;
    FILE* output;
    char filename[20];

    sprintf (filename, "curves/curve_%d_%db.dat", id->row, id->col);
    output = fopen (filename, "w");

    rk (&f, id->x, id->y, xmin, xmax, ymin, ymax, -stepsize, output);

    fclose (output);
    free (basepoint);
    pthread_exit (NULL);
}

void integrate (double x, double y, unsigned int row, unsigned int col) {
    pthread_t fthread;
    pthread_t bthread;
    struct solution_id* fbasepoint = malloc (sizeof (struct solution_id));
    struct solution_id* bbasepoint = malloc (sizeof (struct solution_id));

    fbasepoint->x = bbasepoint->x = x;
    fbasepoint->y = bbasepoint->y = y;
    fbasepoint->col = bbasepoint->col = col;
    fbasepoint->row = bbasepoint->row = row;

    pthread_create (&fthread, NULL, integrate_forward, (void*) fbasepoint);
    pthread_create (&bthread, NULL, integrate_backward, (void*) bbasepoint);
}


void main (int argc, char* argv[]) {
    double xgap, ygap;

    struct solution_id** fbasepoints;
    struct solution_id** bbasepoints;
    pthread_t* fthreads;
    pthread_t* bthreads;

    if (argc != 8) {
        printf("usage: ./plotter <xmin> <xmax> <ymin> <ymax> <rows> <cols> <stepsize>\n");
        return;
    }

    sscanf (argv[1], "%lf", &xmin);
    sscanf (argv[2], "%lf", &xmax);
    sscanf (argv[3], "%lf", &ymin);
    sscanf (argv[4], "%lf", &ymax);
    sscanf (argv[5], "%u", &rows);
    sscanf (argv[6], "%u", &cols);
    sscanf (argv[7], "%lf", &stepsize);

    xgap = (xmax - xmin) / (cols + 1);
    ygap = (ymax - ymin) / (rows + 1);

    // start threads to integrate in columns from the left
    // if rows and cols are 0 the corners of the range are taken as basepoints.

    fthreads =  malloc ((rows + 2) * sizeof (pthread_t));
    bthreads =  malloc ((rows + 2) * sizeof (pthread_t));

    fbasepoints = malloc ((rows + 2) * sizeof (struct solution_id*));
    bbasepoints = malloc ((rows + 2) * sizeof (struct solution_id*));

    // leftmost col

    for (int i = 0; i <= rows + 1; i++) {
        fbasepoints[i] = malloc (sizeof (struct solution_id));
        bbasepoints[i] = malloc (sizeof (struct solution_id));
    }

    for (int i = 0; i <= rows; i++) {
        fbasepoints[i]->x = xmin;
        fbasepoints[i]->y = i * ygap + ymin;
        fbasepoints[i]->col = 0;
        fbasepoints[i]->row = i;

        pthread_create (&fthreads[i], NULL, integrate_forward, (void*) fbasepoints[i]);
    }

    // set y of last row to ymax manually for precision, rather than (rows + 1)*ygap + ymin

    fbasepoints[rows + 1]->x = xmin;
    fbasepoints[rows + 1]->y = ymax;
    fbasepoints[rows + 1]->col = 0;
    fbasepoints[rows + 1]->row = rows + 1;

    pthread_create (&fthreads[rows + 1], NULL, integrate_forward, (void*) fbasepoints[rows + 1]);

    // middle cols

    for (int j = 1; j <= cols; j++) {
        double x = j * xgap + xmin;
        unsigned int col = j;

        for (int i = 0; i <= rows; i++) {
            double y = i * ygap + ymin;
            unsigned int row = i;

            integrate (x, y, row, col);
        }

        integrate (x, ymax, rows + 1, col);
    }

    // rightmost col

    for (int i = 0; i <= rows; i++) {
        bbasepoints[i]->x = xmax;
        bbasepoints[i]->y = i * ygap + ymin;
        bbasepoints[i]->col = cols + 1;
        bbasepoints[i]->row = i;

        pthread_create (&bthreads[i], NULL, integrate_backward, (void*) bbasepoints[i]);
    }

    bbasepoints[rows + 1]->x = xmax;
    bbasepoints[rows + 1]->y = ymax;
    bbasepoints[rows + 1]->col = cols + 1;
    bbasepoints[rows + 1]->row = rows + 1;

    pthread_create (&bthreads[rows + 1], NULL, integrate_backward, (void*) bbasepoints[rows + 1]);

    // free manually allocated basepoints and threads

    free (fbasepoints);
    free (bbasepoints);
    free (fthreads);
    free (bthreads);

    pthread_exit(NULL);
}
