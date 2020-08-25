#ifndef SVM_IMG_H
#define SVM_IMG_H

#include "svm_cell.h"
#include <Common/Image.h>
#include <SFML/Graphics/Image.hpp>
#include <vector>

template <typename T>
class TOS;

// A SVMImage represent a Set Value Map image and is composed of SVMCell.
template <typename T>
class SVMImage
{
public:
    SVMImage(const LibTIM::Image<T> &img);

    // read only public acces to image data
    const std::vector<SVMCell<T> *> &data() const;

    inline void width(std::size_t w);
    inline void height(std::size_t h);
    inline std::size_t width() const;
    inline std::size_t height() const;

    // getter for a pixel @ pos [i,j]
    SVMCell<T> *operator()(std::size_t i, std::size_t j);
    void uninterpolate(TOS<T> *tree);

private:
    // add 1 pixel at the border of value median(Image)
    void extend();

    // interpolate the image
    void interpolate();

private:
    std::size_t m_height, m_width;

    std::vector<SVMCell<T> *> m_image;
    LibTIM::Image<T> m_original;
};

#include "svm_img.hpp"

#endif
