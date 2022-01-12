#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H
#include <queue>
#include <QMutex>

template<class T>
class MessageQueue
{
public:
    MessageQueue()
    {

    }

    void push(T elem)
    {
        QMutexLocker locker(&mutex);
        q.push(elem);
    }


    T pop()
    {
        QMutexLocker locker(&mutex);
        T elem = q.front();
        q.pop();
        return elem;
    }

    T peek()
    {
        QMutexLocker locker(&mutex);
        return q.front();
    }

    bool empty()
    {
        return q.empty();
    }

    size_t size()
    {
        return q.size();
    }
private:
    QMutex mutex;
    typename std::queue<T> q;
};

#endif // MESSAGEQUEUE_H
