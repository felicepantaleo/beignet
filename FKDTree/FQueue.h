/*
 * Queue.h
 *
 *  Created on: Mar 1, 2016
 *      Author: fpantale
 */

#ifndef FKDTREE_QUEUE_H_
#define FKDTREE_QUEUE_H_

#include <vector>
template <class T>
class  FQueue
{
public:
    FQueue();
    FQueue(unsigned int capacity);
    FQueue(const FQueue<T> & v);
    ~FQueue();

    unsigned int capacity() const;
    unsigned int size() const;
    bool empty() const;
    T & front();
    T & tail();
    void push_back(const T & value);
    void pop_front();
    void pop_front(const unsigned int numberOfElementsToPop);

    void reserve(unsigned int capacity);
    void resize(unsigned int capacity);

    T & operator[](unsigned int index);
    FQueue<T> & operator=(const FQueue<T> &);
    void clear();
private:
    unsigned int theSize;
    unsigned int theFront;
    unsigned int theTail;
    std::vector<T> theBuffer;

};

// Your code goes here ...
template<class T>
FQueue<T>::FQueue()
{
	theSize=0;
	theBuffer(0);
	theFront = 0;
	theTail = 0;

}

template<class T>
FQueue<T>::FQueue(const FQueue<T> & v)
{
	theSize = v.theSize;
	theBuffer = v.theBuffer;
	theFront = v.theFront;
	theTail = v.theTail;
}

template<class T>
FQueue<T>::FQueue(unsigned int capacity)
{
    theBuffer.resize(capacity);
	theSize=0;
	theFront = 0;
	theTail = 0;
}


template<class T>
FQueue<T> & FQueue<T>::operator = (const FQueue<T> & v)
{
	theBuffer.clear();
	theSize = v.theSize;
	theBuffer = v.theBuffer;
	theFront = v.theFront;
	theTail = v.theTail;
}



template<class T>
T& FQueue<T>::front()
{
    return theBuffer[theFront];
}

template<class T>
T& FQueue<T>::tail()
{
    return theBuffer[theTail];
}

template<class T>
void FQueue<T>::push_back(const T & v)
{
	if(theSize >= theBuffer.capacity())
	{
		auto oldCapacity = theBuffer.capacity();
		theBuffer.resize(2*oldCapacity);
		if(theFront != 0)
		{
			std::copy(theBuffer.begin(), theBuffer.begin() + theTail, theBuffer.begin()+oldCapacity);
			theTail += oldCapacity;
		}
	}


	theBuffer.at(theTail) = v;
	theTail = (theTail +1) % theBuffer.capacity();
	theSize++;


}

template<class T>
void FQueue<T>::pop_front()
{
	if(theSize>0)
	{
		theFront = (theFront + 1) % theBuffer.capacity();
		theSize--;
	}
}

template<class T>
void FQueue<T>::reserve(unsigned int capacity)
{
	theBuffer.reserve(capacity);
}

template<class T>
unsigned int FQueue<T>::size()const//
{
    return theSize;
}

template<class T>
void FQueue<T>::resize(unsigned int capacity)
{
    theBuffer.resize(capacity);


}

template<class T>
T& FQueue<T>::operator[](unsigned int index)
{
    return theBuffer.at((theFront + index)%theBuffer.capacity());
}

template<class T>
unsigned int FQueue<T>::capacity()const
{
    return theBuffer.capacity();
}

template<class T>
FQueue<T>::~FQueue()
{

}

template <class T>
void FQueue<T>::clear()
{
    theBuffer.clear();
	theSize=0;
	theFront = 0;
	theTail = 0;
}


template <class T>
void FQueue<T>::pop_front(const unsigned int numberOfElementsToPop)
{
	unsigned int elementsToErase = theSize - numberOfElementsToPop > 0 ? numberOfElementsToPop : theSize;
	theSize -=elementsToErase;
	theFront = (theFront + elementsToErase) % theBuffer.capacity();
}






#endif
