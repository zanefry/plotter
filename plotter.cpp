#include <SFML/Graphics.hpp>
#include <cmath>

int window_width = 1200;
int window_height = 1200;

double xrange = 40;
double yrange = 40;

double f(double x)
{
    return std::sin(x);
}

sf::Vector2f plot_to_screen_point(sf::Vector2f v, sf::Vector2f origin)
{
    double xmin = origin.x - xrange/2;

    double screen_x = (v.x - xmin) * window_width/xrange;
    double screen_y = window_height/2 + window_height*(-v.y / yrange) + origin.y;

    return sf::Vector2f(screen_x, screen_y);
}

sf::VertexArray plot_to_screen_curve(sf::VertexArray curve, sf::Vector2f origin)
{
    int num_verts = curve.getVertexCount();
    sf::VertexArray screen_curve = sf::VertexArray(sf::LineStrip, num_verts);

    for (int i = 0; i < num_verts; i++)
        screen_curve[i] = plot_to_screen_point(curve[i].position, origin);

    return screen_curve;
}

sf::VertexArray gen_curve(sf::Vector2f start, double xstepsize)
{
    sf::VertexArray left(sf::LineStrip);
    for (double x = start.x; x > -xrange/2; x -= xstepsize) {
        left.append(sf::Vector2f(x, start.y + f(x)));
    }

    sf::VertexArray right(sf::LineStrip);
    for (double x = start.x; x < xrange/2; x += xstepsize) {
        right.append(sf::Vector2f(x, start.y + f(x)));
    }

    int left_count = left.getVertexCount();
    int right_count = right.getVertexCount();
    sf::VertexArray curve(sf::LineStrip, left_count + right_count);

    int idx = 0;
    for (int i = left_count - 1; i >= 0; i--) {
        curve[idx] = left[i];
        idx++;
    }
    for (int i = 0; i < right_count; i++) {
        curve[idx] = right[i];
        idx++;
    }

    return curve;
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Plotter");
    sf::Vector2f view_origin = sf::Vector2f(0, 0);

    int curve_resolution = 300;
    double xstepsize = xrange / curve_resolution;

    int num_curves = 130;
    std::vector<sf::VertexArray> curves(num_curves);

    float percent_oof = 0.1;
    double ystep = (percent_oof + 1)*yrange / (num_curves - 1);
    for (int i = 0; i < num_curves; i++) {
        curves.push_back(gen_curve(sf::Vector2f(0, -yrange/2 - percent_oof*yrange / 2 + i*ystep), xstepsize));
    }

    while (window.isOpen()) {
        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            view_origin.y += 0.1*yrange;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            view_origin.y -= 0.1*yrange;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            view_origin.x -= 0.01*xrange;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            view_origin.x += 0.01*xrange;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Comma)) {
            xrange *= 1.01;
            yrange *= 1.01;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Period)) {
            xrange *= 0.99;
            yrange *= 0.99;
        }

        window.clear();

        for (sf::VertexArray curve : curves)
            window.draw(plot_to_screen_curve(curve, view_origin));

        window.display();
    }

    return 0;
}
