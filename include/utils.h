#ifndef UTILS_H
#define UTILS_H

#include "svm_cell.h"
#include <SFML/Graphics.hpp>

static bool verbose = false;

#define BLUE "\033[34m"
#define YELLOW "\033[33m"
#define GREEN "\033[32m"
#define RED "\033[31m"
#define RESET "\033[0m"
#define BOLD_ON "\033[1m"
#define BOLD_OFF "\033[21m"

#define VERBOSE(str)      \
    if (verbose)          \
    {                     \
        std::cout << str; \
    }                     \
    std::cout.flush();

sf::Color typeToColor(CellType type)
{
    switch (type)
    {
    case Original:
        return sf::Color::Red;
    case New:
        return sf::Color::Green;
    case Inter2:
    case Inter4:
        return sf::Color::Blue;
    }
}

#endif // UTILS_H
