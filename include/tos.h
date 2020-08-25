#ifndef TOS_H
#define TOS_H

#include "pqueue.h"
#include "svm_img.h"
#include "utils.h"
#include <SFML/Graphics.hpp>
#include <vector>

template <typename T>
class TOS
{
public:
    TOS(SVMImage<T> &img);

    std::vector<SVMCell<T> *> sort();
    void unionFind();
    void canonize();

    // remove pointers to non-origina cells
    void clean();

    // draw the parenting path
    void drawParents(sf::RenderWindow &window, const sf::Vector2f &pos);

private:
    SVMCell<T> *findRoot(SVMCell<T> *current);

    SVMImage<T> &m_image;
    std::vector<SVMCell<T> *> sortedPixels; // R in the article
};

#include "tos.hpp"

#endif // TOS_H
