#include <SFML/Graphics.hpp>
#include <cmath>

int window_width = 1200;
int window_height = 800;

double xmin = -20;
double xmax = 20;
double ymin = -20;
double ymax = 20;
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
    sf::VertexArray left(sf::LineStrip, 0);
    for (double x = start.x; x > xmin; x -= xstepsize) {
        left.append(sf::Vector2f(x, start.y + f(x)));
    }

    sf::VertexArray right(sf::LineStrip, 0);
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

    int curve_resolution = 300;
    double xstepsize = xrange / curve_resolution;

    int num_curves = 130;
    std::vector<sf::VertexArray> curves(num_curves);

    float percent_oof = 0.45;
    double ystep = (percent_oof + 1)*yrange / (num_curves - 1);
    for (int i = 0; i < num_curves; i++) {
        curves.push_back(gen_curve(sf::Vector2f(0, ymin - percent_oof*yrange / 2 + i*ystep), xstepsize));
    }

    while (window.isOpen()) {
        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();

        for (sf::VertexArray curve : curves)
            window.draw(curve);

        window.display();
    }

    return 0;
}
