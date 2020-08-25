#include "svm_img.h"
#include "utils.h"
#include <algorithm>
#include <omp.h>
#include <vector>
template <typename T>
SVMImage<T>::SVMImage(const LibTIM::Image<T> &img) : m_original(img)
{
    m_width = img.getSizeX();
    m_height = img.getSizeY();

    VERBOSE(YELLOW << " - Extend image... ")
    extend();
    VERBOSE(GREEN << "done.\n")

    VERBOSE(YELLOW << " - Image interpolation... ")
    interpolate();
    VERBOSE(GREEN << "done.\n"
                  << RESET)
}

template <typename T>
void SVMImage<T>::extend()
{
    // compute meidan value of the image
    std::vector<T> vec(m_original.begin(), m_original.end());

    std::size_t size = vec.size() - 1;
    // we assume the image is not of size 0,0
    std::sort(vec.begin(), vec.end());
    T median;
    if (size % 2 == 0)
    {
        median = (vec[size / 2] + vec[(size / 2) + 1]) / 2;
    }
    else
    {
        median = vec[size / 2];
    }

    // extends the image with the median value
    LibTIM::TSize newSizeX = m_original.getSizeX() + 2;
    LibTIM::TSize newSizeY = m_original.getSizeY() + 2;

    std::vector<SVMCell<T> *> e_img;

    for (LibTIM::TSize j = 0; j < newSizeY; ++j)
    {
        for (LibTIM::TSize i = 0; i < newSizeX; ++i)
        {
            if (i <= 0 || i >= newSizeX - 1 || j <= 0 || j >= newSizeY - 1)
            {
                e_img.push_back(new SVMCell<T>(CellType::Original, median));
            }
            else
            {
                e_img.push_back(new SVMCell<T>(CellType::Original, m_original(i - 1, j - 1)));
            }
        }
    }
    m_image = e_img;
    m_width = newSizeX;
    m_height = newSizeY;
}

template <typename T>
void SVMImage<T>::interpolate()
{
    VERBOSE("\n") // keep it there!

    std::size_t n = m_width;
    std::size_t m = m_height;

    std::size_t nbCol = 2 * (n * 2 - 1) - 1;
    std::size_t nbLine = 2 * (m * 2 - 1) - 1;
    std::size_t size = (nbCol * nbLine) - 1;
    std::vector<SVMCell<T> *> i_img(size + 1);

    // fill old pixels
#pragma omp parallel for
    for (unsigned int l = 0; l < m_height; l++)
    {
        for (unsigned int c = 0; c < m_width; c++)
        {
            SVMCell<T> *cell = new SVMCell<T>(CellType::Original, m_image.at(l * m_width + c)->value());
            cell->posX(c * 4);
            cell->posY(l * 4);
            cell->visited(false);
            i_img.at((l * 4) * nbCol + (c * 4)) = cell;
        }
    }
    VERBOSE("   + old pixels\n")

    // fill in new pixels
#pragma omp parallel for
    for (std::size_t l = 0; l < nbLine; l += 2)
    {
        for (std::size_t c = (l + 2) % 4; c < nbCol; c += 4)
        {
            if (l % 4 == 2)
            {
                // max of both neighboor original pixels on the same column
                SVMCell<T> *cell = new SVMCell<T>(CellType::New, std::max(i_img.at((l - 2) * nbCol + c)->value(),
                                                                          i_img.at((l + 2) * nbCol + c)->value()));
                cell->posX(c);
                cell->posY(l);
                cell->visited(false);
                i_img.at(l * nbCol + c) = cell;
            }
            else if (l % 4 == 0)
            {
                // max of neighboor original pixel on the same line
                SVMCell<T> *cell = new SVMCell<T>(CellType::New, std::max(i_img.at(l * nbCol + c - 2)->value(),
                                                                          i_img.at(l * nbCol + c + 2)->value()));
                cell->posX(c);
                cell->posY(l);
                cell->visited(false);
                i_img.at(l * nbCol + c) = cell;
            }
        }
    }

    // fill the remaining values (arbitrary order)
#pragma omp parallel for
    for (unsigned int l = 2; l < nbLine; l += 4)
    {
        for (unsigned int c = 2; c < nbCol; c += 4)
        {
            T m = std::max(std::max(i_img.at((l - 2) * nbCol + c)->value(), i_img.at((l + 2) * nbCol + c)->value()),
                           std::max(i_img.at(l * nbCol + c - 2)->value(), i_img.at(l * nbCol + c + 2)->value()));
            SVMCell<T> *cell = new SVMCell<T>(CellType::New, m);
            cell->posX(c);
            cell->posY(l);
            cell->visited(false);
            i_img.at(l * nbCol + c) = cell;
        }
    }
    VERBOSE("   + new pixels\n")

    // fill the interpixels
#pragma omp parallel for
    for (std::size_t l = 0; l < nbLine; l++)
    {
        for (std::size_t c = (l + 1) % 2; c < nbCol; c += 2)
        {
            if (l % 2 == 1)
            {
                // max and min of both neighboor original or new pixels on the same column
                T mi, ma;
                T valU = i_img.at((l - 1) * nbCol + c)->value();
                T valD = i_img.at((l + 1) * nbCol + c)->value();
                if (valU < valD)
                {
                    mi = valU;
                    ma = valD;
                }
                else
                {
                    mi = valD;
                    ma = valU;
                }
                SVMCell<T> *cell = new SVMCell<T>(CellType::Inter2, mi, ma);
                cell->posX(c);
                cell->posY(l);
                cell->visited(false);
                i_img.at(l * nbCol + c) = cell;
            }
            else if (l % 2 == 0)
            {
                // max and min of neighboor original or new pixel on the same line
                std::size_t id = l * nbCol + c;
                T mi, ma;
                T valL = i_img.at(id - 1)->value();
                T valR = i_img.at(id + 1)->value();
                if (valL < valR)
                {
                    mi = valL;
                    ma = valR;
                }
                else
                {
                    mi = valR;
                    ma = valL;
                }
                SVMCell<T> *cell = new SVMCell<T>(CellType::Inter2, mi, ma);
                cell->posX(c);
                cell->posY(l);
                cell->visited(false);
                i_img.at(l * nbCol + c) = cell;
            }
        }
    }

    // arbitrary order
#pragma omp parallel for
    for (unsigned int l = 1; l < nbLine; l += 2)
    {
        for (unsigned int c = 1; c < nbCol; c += 2)
        {

            T values[] = {i_img.at((l - 1) * nbCol + c)->value(),
                          i_img.at((l + 1) * nbCol + c)->value(),
                          i_img.at(l * nbCol + c - 1)->value(),
                          i_img.at(l * nbCol + c + 1)->value()};
            T mi = values[0];
            T ma = mi;
            for (std::size_t i = 1; i < 4; i++)
            {
                if (values[i] < mi)
                {
                    mi = values[i];
                }
                else if (values[i] > ma)
                {
                    ma = values[i];
                }
            }
            SVMCell<T> *cell = new SVMCell<T>(CellType::Inter4, mi, ma);
            cell->posX(c);
            cell->posY(l);
            cell->visited(false);
            i_img.at(l * nbCol + c) = cell;
        }
    }
    VERBOSE("   + interpixels\n")

    m_image = i_img;
    m_height = nbLine;
    m_width = nbCol;
}

template <typename T>
void SVMImage<T>::uninterpolate(TOS<T> *tree)
{
    // clean sortedPixel in tree to remove pointers to non-original cells
    tree->clean();

    // remove all non-original cells
    m_image.erase(std::remove_if(m_image.begin(), m_image.end(), [](SVMCell<T> *cell) { return cell->type() != CellType::Original; }), m_image.end());

    for (auto cell : m_image)
    {
        cell->posX(cell->posX() / 4);
        cell->posY(cell->posY() / 4);
    }

    m_width = m_original.getSizeX() + 2;
    m_height = m_original.getSizeY() + 2;
}

template <typename T>
const std::vector<SVMCell<T> *> &SVMImage<T>::data() const
{
    return m_image;
}

template <typename T>
SVMCell<T> *SVMImage<T>::operator()(std::size_t i, std::size_t j)
{
    return m_image.at(m_width * j + i);
}

template <typename T>
inline void SVMImage<T>::width(std::size_t w) { m_width = w; }
template <typename T>
void SVMImage<T>::height(std::size_t h) { m_height = h; }
template <typename T>
std::size_t SVMImage<T>::width() const { return m_width; }
template <typename T>
std::size_t SVMImage<T>::height() const { return m_height; }
