#include <SFML/Graphics.hpp>
#include <cmath>
//#include <algorithm>

const int window_width = 1800;
const int window_height = 1200;

double scale = 10;
double stepsize = 0.01;

double f(sf::Vector2f v)
{
    return -v.y + std::sin(v.x*10);
}

sf::Vector2f rk4(sf::Vector2f v, bool direction)
{
    // "Butcher's Table" constants for the RK4 integration alg.
    const int s = 4;
    const float c[] = {0, 0.5, 0.5, 1};
    const float b[] = {1.0/8, 3.0/8, 3.0/8, 1.0/8};
    const float a[][3] = {{0.5}, {0, 0.5}, {0, 0, 1}};

    double step;
    if (direction)
        step = stepsize;
    else
        step = -stepsize;

    double k[s];
    for(int i = 0; i < s; i++) {
        double sum = 0;
        for(int j = 0; j < i; j++)
            sum += a[i-1][j] * k[j];

        k[i] = f(sf::Vector2f(v.x + c[i] * step, v.y + sum * step));
    }

    double avg = 0;
    for (int i = 0; i < s; i++)
        avg += b[i] * k[i];

    v += sf::Vector2f(step, avg * step);

    return v;
}

sf::Vector2f plot_to_screen_point(sf::Vector2f v, sf::Vector2f origin)
{
    v -= origin;

    double screen_x = (v.x + scale/2) * window_width/scale;
    double screen_y = (-v.y + scale/2) * window_height/scale;

    return sf::Vector2f(screen_x, screen_y);
}

sf::VertexArray plot_to_screen_curve(sf::VertexArray curve, sf::Vector2f origin)
{
    int num_verts = curve.getVertexCount();
    sf::VertexArray screen_curve = sf::VertexArray(sf::LineStrip, num_verts);

    for (int i = 0; i < num_verts; i++) {
        sf::Vector2f screen_point = plot_to_screen_point(curve[i].position, origin);
        screen_curve[i] = sf::Vertex(screen_point, curve[i].color);
    }

    return screen_curve;
}

// Color saturates linearly from 0 @ 0 slope to 255 @ cap slope and above
sf::Color color(sf::Vector2f v)
{
    double cap = 8;

    double slope = v.y/v.x;
    double sat = 255 - std::min(std::abs(slope) * 255/cap, 255.);
    if (slope >= 0)
        return sf::Color(sat, sat, sat);
    else
        return sf::Color(sat, sat, sat);
}

sf::VertexArray gen_curve(sf::Vector2f start)
{
    sf::VertexArray left(sf::LineStrip);
    sf::VertexArray right(sf::LineStrip);
    sf::Vector2f v(start);

    left.append(v);

    while (-scale/2 < v.x && -scale/2 < v.y && v.y < scale/2) {
        sf::Vector2f old_v = sf::Vector2f(v);
        v = rk4(old_v, false);

        sf::Vector2f dv = v - old_v;
        left.append(sf::Vertex(v, color(dv)));
    }

    // Go back and color basepoint
    left[0].color = color(left[1].position - left[0].position);

    v = start;
    while (v.x < scale/2 && -scale/2 < v.y && v.y < scale/2) {
        sf::Vector2f old_v = sf::Vector2f(v);
        v = rk4(old_v, true);

        sf::Vector2f dv = v - old_v;
        right.append(sf::Vertex(v, color(dv)));
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

    int num_rows = 20;
    int num_cols = 20;
    std::vector<sf::VertexArray> curves(num_rows * num_cols);

    for (int r = 0; r < num_rows; r++)
        for (int c = 0; c < num_cols; c++) {
            double x = -scale/2 + c*(scale/(num_cols-1));
            double y = -scale/2 + r*(scale/(num_rows-1));
            curves.push_back(gen_curve(sf::Vector2f(x, y)));
        }

    ///// Controls /////

    double pan_speed        = 0.002;
    double fast_pan_speed   = 0.005;
    double zoom_speed       = 0.0025;
    double fast_zoom_speed  = 0.005;

    while (window.isOpen()) {
        sf::Event event;

        while (window.pollEvent(event))
            if (event.type == sf::Event::Closed)
                window.close();

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
                view_origin.y += fast_pan_speed * scale;
            else
                view_origin.y += pan_speed * scale;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
                view_origin.y -= fast_pan_speed * scale;
            else
                view_origin.y -= pan_speed * scale;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
                view_origin.x -= fast_pan_speed * scale;
            else
                view_origin.x -= pan_speed * scale;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
                view_origin.x += fast_pan_speed * scale;
            else
                view_origin.x += pan_speed * scale;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Comma))
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
                scale *= 1 + fast_zoom_speed;
            else
                scale *= 1 + zoom_speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Period))
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
                scale *= 1 - fast_zoom_speed;
            else
                scale *= 1 - zoom_speed;

        ///// Draw /////

        window.clear();

        for (sf::VertexArray curve : curves)
            window.draw(plot_to_screen_curve(curve, view_origin));

        window.display();
    }

    return 0;
}
