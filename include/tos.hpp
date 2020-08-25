#include "tos.h"

template <typename T>
TOS<T>::TOS(SVMImage<T> &img) : m_image(img)
{
    VERBOSE(YELLOW << " - Sort pixels... ")
    sortedPixels = sort();
    VERBOSE(GREEN << "done\n")

    VERBOSE(YELLOW << " - Union find algorithm... ")
    unionFind();
    VERBOSE(GREEN << "done.\n")

    VERBOSE(YELLOW << " - Canonize tree... ")
    canonize();
    VERBOSE(GREEN << "done.\n")
}

template <typename T>
void TOS<T>::unionFind()
{
    for (int i = sortedPixels.size() - 1; i >= 0; i--)
    {
        SVMCell<T> *currentP = sortedPixels[i];
        currentP->parent(currentP);
        currentP->zpar(currentP);

        std::vector<SVMCell<T> *> neighbours;

        for (int j = currentP->posY() - 1; j <= currentP->posY() + 1; j++)
        {
            if (j < 0 || j >= m_image.height())
            {
                continue;
            }

            for (int k = currentP->posX() - 1; k <= currentP->posX() + 1; k++)
            {
                if ((j != currentP->posY() || k != currentP->posX()) && k >= 0 && k < m_image.width())
                {
                    neighbours.push_back(m_image(k, j));
                }
            }
        }

        for (auto neighbour : neighbours)
        {
            if (neighbour->zpar() != nullptr)
            {
                SVMCell<T> *root = findRoot(neighbour);
                if (root != currentP)
                {
                    root->parent(currentP);
                    root->zpar(currentP);
                }
            }
        }
    }
}
template <typename T>
SVMCell<T> *TOS<T>::findRoot(SVMCell<T> *current)
{
    if (current->zpar() == current)
    {
        return current;
    }
    else
    {
        current->zpar(findRoot(current->zpar()));
        return current->zpar();
    }
}

template <typename T>
std::vector<SVMCell<T> *> TOS<T>::sort()
{
    PQueue<T> q;
    std::vector<SVMCell<T> *> order;

    std::vector<SVMCell<T> *> neighbours;
    SVMCell<T> *n;

    // get first level
    SVMCell<T> *borderFace = m_image(0, 0); // p_infinite
    T borderValue = borderFace->value();    // l_infinite
    std::size_t initialLevel = static_cast<std::size_t>(borderValue);

    std::size_t l = initialLevel;

    q.push(borderFace, borderValue);

    while (!q.empty())
    {
        SVMCell<T> *currentFace = q.priority_pop(&l); // h

        currentFace->visited(true);
        currentFace->level(l);
        order.push_back(currentFace);

        // get neighbours
        for (int j = currentFace->posY() - 1; j <= currentFace->posY() + 1; j++)
        {
            if (j < 0 || j >= m_image.height())
            {
                continue;
            }

            for (int k = currentFace->posX() - 1; k <= currentFace->posX() + 1; k++)
            {
                if ((j != currentFace->posY() || k != currentFace->posX()) && k >= 0 && k < m_image.width())
                {

                    n = m_image(k, j);

                    if (!n->visited())
                    {
                        neighbours.push_back(n);
                    }
                }
            }
        }

        // add neighbourhood to queue
        for (unsigned int j = 0; j < neighbours.size(); j++)
        {
            q.priority_push(neighbours[j], l);
            neighbours[j]->visited(true);
        }
        neighbours.clear();
    }

    return order;
}

template <typename T>
void TOS<T>::canonize()
{
    // for all p in [R in reverse order]
    for (long int i = sortedPixels.size() - 1; i >= 0; i--)
    {
        // the pixel for which we are looking for a canonical parent
        SVMCell<T> *p = sortedPixels[i];

        // q <- parent(p)
        SVMCell<T> *q = p->parent();

        // we check that the parent pixel is of Original type
        // if it is not the case -> we go through its parents
        // until we get to a pixel of Original type
        while (q->parent()->type() != CellType::Original)
        {
            q->parent(q->parent()->parent());
        }

        // we check that the pixel and its parent belong to the same shape
        if (q->level() == p->level())
        {
            // we save the path we made to find the canonical pixel
            std::vector<SVMCell<T> *> qSave;
            qSave.push_back(q);

            // if the pixel and its parent are in the same shape
            // and if the pixel and its parent aren't the SAME pixel
            while (q->parent()->level() == q->level() && q->parent() != q) // See svm_cell.h
            {
                // we continue through its parents
                q = q->parent();
                qSave.push_back(q);
            }

            // if the pixel and its parent are the same pixel
            if (q->parent() == q)
            {
                // if it is of Original type, we can consider
                // it is the parent we were looking for
                if (q->parent()->type() == CellType::Original)
                {
                    // it is now the parent of the current pixel
                    p->parent(q->parent());
#pragma omp parallel for
                    // and the parent of the pixels we went through
                    for (unsigned int j = 0; j < qSave.size(); j++)
                    {
                        qSave[j]->parent(q->parent());
                    }
                }
                // otherwise, the pixel and its parent are not in the same shape anymore
            }
            else if (q->parent()->level() != q->level())
            {
                // if the pixel is of Original type
                if (q->parent()->type() == CellType::Original)
                {
                    p->parent(q->parent());
#pragma omp parallel for
                    for (unsigned int j = 0; j < qSave.size(); j++)
                    {
                        qSave[j]->parent(q->parent());
                    }
                }
                else
                {
                    // otherwise we go through its parents
                    // until we get to an Original pixel
                    // and we take this one as a parent
                    while (q->parent()->type() != CellType::Original)
                    {
                        q->parent(q->parent()->parent());
                    }
                    p->parent(q->parent());
#pragma omp parallel for
                    for (unsigned int j = 0; j < qSave.size(); j++)
                    {
                        qSave[j]->parent(q->parent());
                    }
                }
            }
            // otherwise the pixel and its parent are not in the same shape anymore
            // we simply set the parent
        }
        else
        {
            p->parent(q->parent());
        }
    }
}

template <typename T>
void TOS<T>::clean()
{
    sortedPixels.erase(std::remove_if(sortedPixels.begin(), sortedPixels.end(), [](SVMCell<T> *cell) { return cell->type() != CellType::Original; }), sortedPixels.end());
}

template <typename T>
void TOS<T>::drawParents(sf::RenderWindow &window, const sf::Vector2f &pos)
{
    if (pos.x > 0 && pos.x < m_image.width() && pos.y > 0 && pos.y < m_image.height())
    {
        std::vector<sf::Vertex> path;
        sf::Vertex square[5];

        SVMCell<T> *cell = m_image(static_cast<std::size_t>(pos.x), static_cast<std::size_t>(pos.y));
        SVMCell<T> *current = cell;

        do
        {
            path.push_back(sf::Vertex(sf::Vector2f(current->posX() + 0.5f, current->posY() + 0.5f), typeToColor(current->type())));
            sf::Color col = typeToColor(current->type());
            square[0] = sf::Vertex(sf::Vector2f(current->posX(), current->posY()), col);
            square[1] = sf::Vertex(sf::Vector2f(current->posX() + 1, current->posY()), col);
            square[2] = sf::Vertex(sf::Vector2f(current->posX() + 1, current->posY() + 1), col);
            square[3] = sf::Vertex(sf::Vector2f(current->posX(), current->posY() + 1), col);
            square[4] = square[0];
            window.draw(square, 5, sf::LinesStrip);
            current = current->parent();
        } while (current->parent() != current);

        sf::Color col = typeToColor(cell->type());
        square[0] = sf::Vertex(sf::Vector2f(cell->posX(), cell->posY()), col);
        square[1] = sf::Vertex(sf::Vector2f(cell->posX() + 1, cell->posY()), col);
        square[2] = sf::Vertex(sf::Vector2f(cell->posX() + 1, cell->posY() + 1), col);
        square[3] = sf::Vertex(sf::Vector2f(cell->posX(), cell->posY() + 1), col);
        square[4] = square[0];

        window.draw(path.data(), path.size(), sf::LinesStrip);
        window.draw(square, 5, sf::LinesStrip);
    }
}
