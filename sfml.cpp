#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <cstdio>
#include <pthread.h>

// constants for RK
const short s = 4;
const float c[] = {0, 1.0/2, 1.0/2, 1};
const float b[] = {1.0/8, 3.0/8, 3.0/8, 1.0/8};
const float a[][3] = {{1.0/2}, {0, 1.0/2}, {0, 0, 1}};

using namespace std;
using namespace sf;

vector<vector<Vertex> > curves;

static double xmin, xmax, ymin, ymax;
static unsigned int rows, cols;
static double stepsize;

typedef struct solution_id {
    double x;
    double y;
    unsigned int row;
    unsigned int col;
} solution_id;

double f(double x, double y)
{
    return x + sin(y);
}

void rk(double x_i, double y_i, bool direction)
{
    vector<Vertex> pts;

    double x = x_i, y = y_i;
    double sz = direction ? stepsize : -stepsize;
    double k[s];

    while (1) {
        pts.push_back(Vertex(Vector2f(x, y)));

        if ((x < xmin || xmax < x) || (y < ymin || ymax < y)) {
            curves.push_back(pts);
            return;
        }

        for (int i = 0; i < s; i++) {
            double sum = 0;

            for(int j = 0; j < i; j++)
                sum += a[i-1][j] * k[j];

            k[i] = f(x + c[i] * stepsize, y + sum * sz);
        }

        double avg = 0;
        for (int i = 0; i < s; i++)
            avg += b[i] * k[i];

        x += sz;
        y += avg * sz;
    }
}

int main(int argc, char** argv)
{
    int windowWidth = 1000;
    int windowHeight = 1000;
    RenderWindow window(VideoMode(windowWidth, windowHeight), "Test");

    double xgap, ygap;

    if (argc != 8) {
        printf("usage: ./plotter <xmin> <xmax> <ymin> <ymax> <rows> <cols> <stepsize>\n");
        return 1;
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

    // leftmost col
    for (int i = 0; i <= rows; i++)
        rk(xmin, i*ygap + ymin, true);

    // set y of last row to ymax manually for precision, rather than (rows + 1)*ygap + ymin
    rk(xmin, ymax, true);

    // middle cols

    for (int j = 1; j <= cols; j++) {
        double x = j * xgap + xmin;
        unsigned int col = j;

        for (int i = 0; i <= rows; i++) {
            double y = i * ygap + ymin;
            unsigned int row = i;

            rk(x, y, true);
            rk(x, y, false);
        }

        rk(x, ymax, true);
        rk(x, ymax, false);
    }

    // rightmost col
    for (int i = 0; i <= rows; i++)
        rk(xmax, i*ygap + ymin, false);

    rk(xmax, ymax, false);

    for (int i = 0; i < curves.size(); i++) {
        for (int j = 0; j < curves[i].size(); j++) {
            curves[i][j].position.x -= xmin;
            curves[i][j].position.x *= windowWidth / (xmax - xmin);

            curves[i][j].position.y -= ymin;
            curves[i][j].position.y *= windowHeight / (ymax - ymin);
        }
    }

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event))
            if (event.type == Event::Closed)
                window.close();

        window.clear();

        for (vector<Vertex> pts : curves)
            window.draw(&pts[0], pts.size(), LineStrip);

        window.display();
    }

    return 0;
}
