#include "img_handler.h"
#include "pqueue.h"
#include "svm_cell.h"
#include "svm_img.h"
#include "tos.h"
#include <Common/Image.h>
#include <SFML/Graphics.hpp>
#include <chrono>
#include <getopt.h>
#include <iostream>

void drawUI(sf::RenderWindow &window, const sf::View &view);

void help()
{
    std::cout << BOLD_ON << "\nUsage:\n"
              << RESET
              << " tos [options] infile [options]\n\n"
              << "Compute infile's tree of shape.\n\n"
              << BOLD_ON << "Options\n"
              << RESET
              << " -n, --no-uninterpolation Deactivate the uninterpolation step\n"
              << " -f, --file <infile>      The file to process, ignore non-option infile\n"
              << " -v, --verbose            Display step description output\n"
              << " -d, --display            Open the graphical interface\n\n"
              << " -h, --help               Display this help\n"
              << " -V, --version            Display version\n"
              << std::endl;
}

int main(int argc, char *argv[])
{
    bool file_provided = false;
    bool uninterpolate = true;
    bool display = false;
    int file_arg_pos = 1;

    static struct option long_options[] = {
        {"no-uninterpolation", no_argument, nullptr, 'n'},
        {"file", required_argument, nullptr, 'f'},
        {"verbose", no_argument, nullptr, 'v'},
        {"display", no_argument, nullptr, 'd'},
        {"help", no_argument, nullptr, 'h'},
        {"version", no_argument, nullptr, 'V'},
        {nullptr, 0, nullptr, 0}};

    int c;

    if (argc == 1)
    {
        help();
        exit(EXIT_FAILURE);
    }

    while ((c = getopt_long(argc, argv, "nf:hVvd", long_options, nullptr)) != -1)
    {
        // Option argument
        switch (c)
        {
        case 'n': // No uninterpolation
            uninterpolate = false;
            break;
        case 'h': // display help
            help();
            exit(EXIT_SUCCESS);
        case 'v': // verbose
            verbose = true;
            break;
        case 'd': // verbose
            display = true;
            break;
        case 'V': // display version
            std::cout << "tos, Tree of Shape, by Méline Bourg-Lang, Morgane Ritter & Nathan Roth" << std::endl;
            exit(EXIT_SUCCESS);
        case 'f':
            file_provided = true;
            file_arg_pos = optind - 1;
            break;
        default:
            help();
            exit(EXIT_FAILURE);
        }
    }

    if (optind == argc - 1 && file_provided == false)
    {
        file_provided = true;
        file_arg_pos = optind;
    }
    else
    {
        help();
        exit(EXIT_FAILURE);
    }

    if (!file_provided)
    {
        std::cout << "PGM image is missing" << std::endl;
        help();
        exit(EXIT_FAILURE);
    }

    auto start = std::chrono::high_resolution_clock::now();

    // Image is a generic class templated by the image points' type
    LibTIM::Image<unsigned char> im;
    if (LibTIM::Image<LibTIM::U8>::load(argv[file_arg_pos], im))
    {
        VERBOSE("PGM image is loaded\n")
    }
    else
    {
        return EXIT_FAILURE;
    }

    VERBOSE(BLUE << "Creating SVM Object.\n")
    SVMImage<LibTIM::U8> svm_img(im);
    VERBOSE(GREEN << "SVM object created\n"
                  << RESET)

    VERBOSE(BLUE << "Creating tree of shape.\n")
    TOS<LibTIM::U8> tree(svm_img);
    VERBOSE(GREEN << "Tree created\n"
                  << RESET)

    VERBOSE(YELLOW << "Image uninterpolation... ")
    if (uninterpolate)
        svm_img.uninterpolate(&tree);
    VERBOSE(GREEN << "done.\n"
                  << RESET)

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    std::cout << "Tree computation executed in " << duration << " milliseconds" << std::endl;

    if (display)
    {
        sf::ContextSettings settings;
        settings.antialiasingLevel = 8;
        sf::RenderWindow window(sf::VideoMode(800, 600), "Tree of Shape", sf::Style::Default, settings);

        // A view is a "2D camera" that display a certain region of the 2D world. Moving or scaling a view
        // enables the user to navgate in the world.
        sf::View view(window.getView());

        // Needed to render the SVMImage on the screen (as a texture on a sprite)
        VERBOSE(BLUE << "Image handler... ")
        ImgHandler<LibTIM::U8> handler(svm_img);
        VERBOSE(GREEN << "initialized.\n")

        // Variables needed to compute mouse position changes (panning)
        sf::Vector2f newPos, oldPos;
        sf::Vector2f mousePos;

        // Flag which indicates when the lef button is in the down position
        bool mouseButtonDown = false;

        while (window.isOpen())
        {

            sf::Event event;
            while (window.pollEvent(event))
            {
                // the user closes the window
                if (event.type == sf::Event::Closed)
                    window.close();

                // a keyboard key is pressed
                if (event.type == sf::Event::KeyPressed)
                {
                    switch (event.key.code)
                    {
                    case sf::Keyboard::Escape:
                        window.close();
                        break;
                    default:
                        break;
                    }
                }

                // The window is resized
                if (event.type == sf::Event::Resized)
                {
                    view.setSize(event.size.width, event.size.height);
                    window.setView(view);
                }

                // A mouse button is pressed
                if (event.type == sf::Event::MouseButtonPressed)
                {
                    // Left button
                    if (event.mouseButton.button == sf::Mouse::Button::Left)
                    {
                        // The left button is now in the DOWN position
                        mouseButtonDown = true;
                        // Retrieving the mouse position at the event time
                        oldPos = window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
                        continue;
                    }
                }
                if (event.type == sf::Event::MouseButtonReleased)
                {
                    if (event.mouseButton.button == sf::Mouse::Button::Left)
                    {
                        // Left button now in the UP position
                        mouseButtonDown = false;
                    }
                }

                // Scrolling event ==> zoom in/out
                if (event.type == sf::Event::MouseWheelMoved)
                {
                    float delta = event.mouseWheel.delta;
                    if (delta > 0)
                        view.zoom(0.9f);
                    else
                        view.zoom(1.1f);
                    window.setView(view);
                }

                if (event.type == sf::Event::MouseMoved)
                {
                    mousePos = window.mapPixelToCoords({event.mouseMove.x, event.mouseMove.y});
                }

                if (mouseButtonDown)
                {
                    newPos = window.mapPixelToCoords({event.mouseMove.x, event.mouseMove.y});
                    sf::Vector2f deltaPos = oldPos - newPos;
                    // the view is moved by the vector representing the change in position of the mouse cursor
                    view.setCenter(view.getCenter() + deltaPos);
                    window.setView(view);
                    oldPos = window.mapPixelToCoords({event.mouseMove.x, event.mouseMove.y});
                }
            }

            // Draw calls
            window.clear(sf::Color(255, 255, 255));
            handler.draw(window);
            tree.drawParents(window, mousePos);
            drawUI(window, view);

            window.display();
        }
    }

    return 0;
}

void drawUI(sf::RenderWindow &window, const sf::View &view)
{
    sf::Vertex X[] = {
        sf::Vertex(sf::Vector2f(-(view.getSize().x - view.getCenter().x), 0.f), sf::Color::Red),
        sf::Vertex(sf::Vector2f((view.getSize().x + view.getCenter().x), 0.f), sf::Color::Red)};
    sf::Vertex Y[] = {
        sf::Vertex(sf::Vector2f(0.f, (view.getSize().y + view.getCenter().y)), sf::Color::Green),
        sf::Vertex(sf::Vector2f(0.f, -(view.getSize().y - view.getCenter().y)), sf::Color::Green)};

    window.draw(X, 2, sf::Lines);
    window.draw(Y, 2, sf::Lines);
}
