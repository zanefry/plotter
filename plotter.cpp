#include <SFML/Graphics.hpp>
#include <cmath>

const int window_width = 1400;
const int window_height = 1400;

double scale = 20;
double stepsize = 0.05;

double f(sf::Vector2f v)
{
    return std::sin(v.x) + sin(v.y*v.y);
}

sf::Vector2f kbd_move_view()
{
    double pan_speed        = 0.002;
    double fast_pan_speed   = 0.005;
    double zoom_speed       = 0.0025;
    double fast_zoom_speed  = 0.005;

    sf::Vector2f disp(0, 0);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
            disp.y += fast_pan_speed * scale;
        else
            disp.y += pan_speed * scale;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
            disp.y -= fast_pan_speed * scale;
        else
            disp.y -= pan_speed * scale;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
            disp.x -= fast_pan_speed * scale;
        else
            disp.x -= pan_speed * scale;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
            disp.x += fast_pan_speed * scale;
        else
            disp.x += pan_speed * scale;
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

    return disp;
}

sf::Vector2f rk4(sf::Vector2f v, bool direction)
{
    // "Butcher's Table" constants for the RK4 integration alg.
    const int s = 4;
    const float c[] = {0, 0.5, 0.5, 1};
    const float b[] = {1.0/8, 3.0/8, 3.0/8, 1.0/8};
    const float a[][3] = {{0.5}, {0, 0.5}, {0, 0, 1}};

    double step = direction ? stepsize : -stepsize;

    double k[s];
    for (int i = 0; i < s; i++) {
        double sum = 0;
        for (int j = 0; j < i; j++)
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
    double cap = 100;

    double slope = v.y/v.x;
    double sat = 255 - std::min(std::abs(slope) * 255/cap, 255.);
    if (slope >= 0)
        return sf::Color(sat, sat, sat);
    else
        return sf::Color(sat, sat, sat);
}

sf::VertexArray gen_curve(sf::Vector2f start, sf::Vector2f xbounds, sf::Vector2f ybounds)
{
    double xmin, xmax, ymin, ymax;
    xmin = xbounds.x, xmax = xbounds.y, ymin = ybounds.x, ymax = ybounds.y;

    sf::VertexArray left(sf::LineStrip);
    sf::VertexArray right(sf::LineStrip);
    sf::Vector2f v(start);

    left.append(v);

    while (xmin <= v.x && v.x <= xmax && ymin <= v.y && v.y <= ymax) {
        sf::Vector2f old_v = sf::Vector2f(v);
        v = rk4(old_v, false);

        sf::Vector2f dv = v - old_v;
        left.append(sf::Vertex(v, color(dv)));
    }

    // Go back and color basepoint
    left[0].color = color(left[1].position - left[0].position);

    v = start;
    while (xmin <= v.x && v.x <= xmax && ymin <= v.y && v.y <= ymax) {
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

    double hspacing = 1;
    double vspacing = 1;
    int hlimit = 10;
    int vlimit = 10;

    double xmin, xmax, ymin, ymax;
    xmin = -10, xmax = 10;
    ymin = -10, ymax = 10;

    std::vector<sf::VertexArray> curves;
    for (int r = -vlimit; r <= vlimit; r++)
        for (int c = -hlimit; c <= hlimit; c++) {
            sf::Vector2f xbounds(xmin, xmax);
            sf::Vector2f ybounds(ymin, ymax);
            sf::Vector2f bp(c*hspacing, r*vspacing);

            curves.push_back(gen_curve(bp, xbounds, ybounds));
        }

    std::vector<sf::VertexArray> grid;
    for (int x = std::ceil(xmin); x <= std::floor(xmax); x++) {
        sf::VertexArray line(sf::LineStrip);
        line.append(sf::Vertex(sf::Vector2f(x, ymin), sf::Color(50, 0, 0)));
        line.append(sf::Vertex(sf::Vector2f(x, ymax), sf::Color(50, 0, 0)));
        grid.push_back(line);
    }
    for (int y = std::ceil(ymin); y <= std::floor(ymax); y++) {
        sf::VertexArray line(sf::LineStrip);
        line.append(sf::Vertex(sf::Vector2f(xmin, y), sf::Color(50, 0, 0)));
        line.append(sf::Vertex(sf::Vector2f(xmax, y), sf::Color(50, 0, 0)));
        grid.push_back(line);
    }


    while (window.isOpen()) {
        sf::Event event;

        while (window.pollEvent(event))
            if (event.type == sf::Event::Closed)
                window.close();

        view_origin += kbd_move_view();

        ///// Draw /////

        window.clear();

        for (sf::VertexArray gridline : grid)
            window.draw(plot_to_screen_curve(gridline, view_origin));
        for (sf::VertexArray curve : curves)
            window.draw(plot_to_screen_curve(curve, view_origin));

        window.display();
    }

    return 0;
}
