#include <SFML/Graphics.hpp>
#include <cmath>

int window_width = 1200;
int window_height = 800;

double xmin = -20;
double xmax = 20;
double ymin = -2;
double ymax = 2;
double xrange = xmax - xmin;
double yrange = ymax - ymin;

double f(double x)
{
    return std::sin(x);
}

sf::Vector2f plot_to_screen_xform(sf::Vector2f v)
{
    double screen_x = (v.x - xmin) * window_width/xrange;
    double screen_y = window_height/2 + window_height*(-v.y / yrange);

    return sf::Vector2f(screen_x, screen_y);
}

sf::VertexArray gen_curve(sf::Vector2f start, double xstepsize)
{
    sf::VertexArray left(sf::LineStrip, 1);
    for (double x = start.x; x > xmin; x -= xstepsize) {
        left.append(sf::Vector2f(x, start.y + f(x)));
    }

    sf::VertexArray right(sf::LineStrip, 1);
    for (double x = start.x; x < xmax; x += xstepsize) {
        right.append(sf::Vector2f(x, start.y + f(x)));
    }

    int left_count = left.getVertexCount();
    int right_count = right.getVertexCount();
    sf::VertexArray curve(sf::LineStrip, left_count + right_count);

    int idx = 0;
    for (int i = left_count - 1; i >= 0; i--) {
        curve[idx] = plot_to_screen_xform(left[i].position);
        idx++;
    }
    for (int i = 0; i < right_count; i++) {
        curve[idx] = plot_to_screen_xform(right[i].position);
        idx++;
    }

    return curve;
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "SFML works!");

    int curve_resolution = 100;
    double xstepsize = xrange / curve_resolution;

    sf::VertexArray curve = gen_curve(sf::Vector2f(1, -1), xstepsize);

//    for (int step = 0; step < curve_resolution; step++) {
//        double x = xmin + step*xstepsize;
//        double y = std::sin(x);
//
//        curve[step] = plot_to_screen_xform(sf::Vector2f(x, y));
//    }

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(curve);
        window.display();
    }

    return 0;
}
