#include "pqueue.h"

template <typename T>
PQueue<T>::PQueue()
{
}

template <typename T>
void PQueue<T>::push(SVMCell<T> *cell, std::size_t level)
{
    try {
        m_pqueue.at(level);
    } catch (const std::out_of_range &oor) {
        m_pqueue.insert(std::pair<unsigned int,std::deque<SVMCell<T>*>>(level,std::deque<SVMCell<T> *>()));
    }
    m_pqueue.at(level).push_back(cell);
}

template <typename T>
SVMCell<T> *PQueue<T>::pop(std::size_t level)
{
    // retrieve the front element
    SVMCell<T> *cell = m_pqueue.at(level).front();
    // delete the front element
    m_pqueue.at(level).pop_front();

    return cell;
}

template <typename T>
void PQueue<T>::priority_push(SVMCell<T> *cell, std::size_t level)
{
    std::size_t lower;
    std::size_t upper;
    std::size_t levelToPush;

    if (cell->type() == CellType::Inter2 || cell->type() == CellType::Inter4)
    {
        lower = static_cast<std::size_t>(cell->min());
        upper = static_cast<std::size_t>(cell->max());
    }
    else
    {
        lower = static_cast<std::size_t>(cell->value());
        upper = static_cast<std::size_t>(cell->value());
    }

    if (lower > level)
    {
        levelToPush = lower;
    }
    else if (upper < level)
    {
        levelToPush = upper;
    }
    else
    {
        levelToPush = level;
    }

    push(cell, levelToPush);
}

template <typename T>
SVMCell<T> *PQueue<T>::priority_pop(std::size_t *level)
{
    //std::size_t currentLevel = *level;
    while (m_pqueue[*level].size() == 0)
    {
        *level = (*level + 1) % m_pqueue.size();
    }
    return (pop(*level));
}

template <typename T>
std::size_t PQueue<T>::size() const
{
    return m_pqueue.size();
}

template <typename T>
bool PQueue<T>::empty() const
{
    if(m_pqueue.size() == 0) {
        return true;
    }

    for(auto it = m_pqueue.begin(); it != m_pqueue.end(); it++)
    {
        if (!(*it).second.empty())
        {
            return false;
        }
    }
    return true;
}
