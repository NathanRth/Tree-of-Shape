#include "img_handler.h"

template <typename T>
ImgHandler<T>::ImgHandler(SVMImage<T> &img) : m_svmImage(img)
{
    m_image.create(m_svmImage.width(), m_svmImage.height());
    feed();
    m_texture.loadFromImage(m_image);
    m_sprite.setTexture(m_texture);
}

template <typename T>
void ImgHandler<T>::draw(sf::RenderWindow &window)
{
    window.draw(m_sprite);
}

template <typename T>
void ImgHandler<T>::feed()
{
    for (int j = 0; j < m_svmImage.height(); j++) //y
    {
        for (int i = 0; i < m_svmImage.width(); i++) //x
        {
            SVMCell<T> *cell = m_svmImage.data().at(j * m_svmImage.width() + i);
            if (cell->type() == CellType::Original || cell->type() == CellType::New)
            {
                sf::Uint8 val = static_cast<sf::Uint8>(cell->value());
                sf::Color col(val, val, val);
                m_image.setPixel(i, j, col);
            }
            else
            {
                sf::Uint8 val = static_cast<sf::Uint8>((cell->min() + cell->max()) / static_cast<T>(2));
                sf::Color col(val, val, val);
                m_image.setPixel(i, j, col);
            }
        }
    }
}
