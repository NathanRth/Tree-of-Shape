#ifndef SVM_CELL_H
#define SVM_CELL_H

#include <cstddef> // for size_t

// A SVMCell represent a Set Value Map Cell, which compose a Set Value Map
// A SVMCell can be of four types:
enum CellType
{
    Original = 0, // a cell directly generated from the original 2D image, holding the value of the original pixel
    New = 1,      // a cell between two Original cell, holding the highest value of the Original cell in its neighbourhood
    Inter2 = 2,   // a cell between an Original and a New cell (up/down or left/right), holding the value range from the lowest value of the surround cells to the highest value
    Inter4 = 3    // a cell between an four Inter2 cells (up/down and left/right), holding the value range from the lowest value of the surround cells to the highest value
};

//   __________ ___________ __________ ____________ __________
//  | Original |  Inter2   |    New   |            |          |
//  |     8    |  (8..24)  |   (12)   |  (24..24)  |    12    |
//  |__________|___________|__________|____________|__________|
//  |   Inter2 |  Inter4   |  Inter2  |            |          |
//  |  (8..8)  |  {8..24}  | (12..24) |  {24..24}  | (12..24) |
//  |__________|___________|__________|____________|__________|
//  |    New   |   Inter2  |    New   |            |          |
//  |    (8)   |  (8..24)  |   (24)   |  (24..24)  |   (24)   |
//  |__________|___________|__________|____________|__________|
//  |          |           |          |            |          |
//  |  (0..8)  |  {8..24}  | (24..24) |  {24..24}  | (24..24) |
//  |__________|___________|__________|____________|__________|
//  |          |           |          |            |          |
//  |     0    |  (8..24)  |   (24)   | (24..24)   |    24    |
//  |__________|___________|__________|____________|__________|
//
template <typename T>
class SVMCell
{
public:
    SVMCell();
    SVMCell(CellType t, T a, T b = 0);

    // setters
    inline void type(CellType type);
    inline void value(T value);
    inline void min(T min);
    inline void max(T max);
    inline void visited(bool visited);
    inline void posX(int x);
    inline void posY(int y);
    inline void level(std::size_t l);
    inline void parent(SVMCell<T> *parent);
    inline void zpar(SVMCell<T> *zpar);

    // getters
    inline CellType type() const;
    inline T value() const;
    inline T min() const;
    inline T max() const;
    inline bool visited() const;
    inline int posX() const;
    inline int posY() const;
    inline std::size_t level() const;
    inline SVMCell<T> *parent() const;
    inline SVMCell<T> *zpar() const;

private:
    // image interpolation
    CellType m_type;     // type of the cell, determine if m_value or m_in/m_max is used
    bool m_visited;      // if the cell as been visited
    std::size_t m_level; // memorization of the level where the queue handled the face
    int m_x, m_y;        // cell coordinates in the SVMImage

    // tree generation
    SVMCell<T> *m_parent;
    SVMCell<T> *m_zpar;

    union {
        struct
        {
            T m_value; // the value of the cell, for Original and New type
        };
        struct
        {
            T m_min; // the value range, for Inter2/4 type
            T m_max;
        };
    };
};

template <typename T>
bool areSameVal(const SVMCell<T> &a, const SVMCell<T> &b);

#include "svm_cell.hpp"

#endif // SVM_CELL_H
