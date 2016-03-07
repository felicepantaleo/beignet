#ifndef FKDTREE_FKDTREE_H_
#define FKDTREE_FKDTREE_H_

#include <vector>
#include <array>
#include <algorithm>
#include <cmath>
#include <utility>
#include <iostream>
#include <deque>
#include <cassert>

#include "KDPoint.h"
#include "FQueue.h"


template<class TYPE, int numberOfDimensions>
class FKDTree
{

public:

	FKDTree(const long int nPoints)
	{
		theNumberOfPoints = nPoints;
		theDepth = std::floor(log2(nPoints));
		for (auto& x : theDimensions)
			x.resize(theNumberOfPoints);
		theIntervalLength.resize(theNumberOfPoints, 0);
		theIntervalMin.resize(theNumberOfPoints, 0);
		theIds.resize(theNumberOfPoints);
		thePoints.reserve(theNumberOfPoints);

	}

	FKDTree(const long int nPoints,
			const std::vector<KDPoint<TYPE, numberOfDimensions> >& points)
	{
		theNumberOfPoints = nPoints;
		theDepth = std::floor(log2(nPoints));
		for (auto& x : theDimensions)
			x.resize(theNumberOfPoints);
		theIntervalLength.resize(theNumberOfPoints, 0);
		theIntervalMin.resize(theNumberOfPoints, 0);
		theIds.resize(theNumberOfPoints, 0);
		thePoints = points;

	}


	FKDTree()
	{
		theNumberOfPoints = 0;
		theDepth = 0;
		for (auto& x : theDimensions)
			x.clear();
		theIntervalLength.clear();
		theIntervalMin.clear();
		theIds.clear();
		thePoints.clear();
	}

	FKDTree(unsigned int capacity);

	FKDTree(const FKDTree<TYPE, numberOfDimensions>& v);

	FKDTree(FKDTree<TYPE, numberOfDimensions> && other)
	{
		theNumberOfPoints(std::move(other.theNumberOfPoints));
		theDepth(std::move(other.theDepth));

		theIntervalLength.clear();
		theIntervalMin.clear();
		theIds.clear();
		thePoints.clear();
		for (auto& x : theDimensions)
			x.clear();

		theIntervalLength = std::move(other.theIntervalLength);
		theIntervalMin = std::move(other.theIntervalMin);
		theIds = std::move(other.theIds);

		thePoints = std::move(other.thePoints);
		for (int i =0; i< numberOfDimensions; ++i)
			theDimensions = std::move(other.theDimensions);
	}

	FKDTree<TYPE, numberOfDimensions>& operator=(FKDTree<TYPE, numberOfDimensions>&& other)
	{

		if (this != &other)
		{
			theNumberOfPoints(std::move(other.theNumberOfPoints));
			theDepth(std::move(other.theDepth));

			theIntervalLength.clear();
			theIntervalMin.clear();
			theIds.clear();
			thePoints.clear();
			for (auto& x : theDimensions)
				x.clear();

			theIntervalLength = std::move(other.theIntervalLength);
			theIntervalMin = std::move(other.theIntervalMin);
			theIds = std::move(other.theIds);

			thePoints = std::move(other.thePoints);
			for (int i =0; i< numberOfDimensions; ++i)
				theDimensions = std::move(other.theDimensions);
		}
		return *this;

	}

	void push_back(const KDPoint<TYPE, numberOfDimensions>& point)
	{

		thePoints.push_back(point);
#pragma unroll
		for (int i = 0; i < numberOfDimensions; ++i)
			theDimensions.at(i).push_back(point[i]);
		theIds.push_back(point.getId());
	}


	void push_back(KDPoint<TYPE, numberOfDimensions>&& point)
	{

		thePoints.push_back(point);
#pragma unroll
		for (int i = 0; i < numberOfDimensions; ++i)
			theDimensions.at(i).push_back(point[i]);
		theIds.push_back(point.getId());
	}

	void add_at_position(const KDPoint<TYPE, numberOfDimensions>& point,
			const unsigned int position)
	{
		for (int i = 0; i < numberOfDimensions; ++i)
			theDimensions[i][position] = point[i];
		theIds[position] = point.getId();

	}

	void add_at_position(KDPoint<TYPE, numberOfDimensions> && point,
			const unsigned int position)
	{
		for (int i = 0; i < numberOfDimensions; ++i)
			theDimensions[i][position] = point[i];
		theIds[position] = point.getId();

	}

	KDPoint<TYPE, numberOfDimensions> getPoint(unsigned int index) const
	{

		KDPoint<TYPE, numberOfDimensions> point;

		for (int i = 0; i < numberOfDimensions; ++i)
			point.setDimension(i, theDimensions[i][index]);

		point.setId(theIds[index]);

		return point;
	}



	std::vector<unsigned int> search_in_the_box(
			const KDPoint<TYPE, numberOfDimensions>& ,
			const KDPoint<TYPE, numberOfDimensions>& ) const;

	bool test_correct_build(unsigned int index = 0, int dimension = 0) const
	{

		unsigned int leftSonIndexInArray = 2 * index + 1;
		unsigned int rightSonIndexInArray = leftSonIndexInArray + 1;
		if (rightSonIndexInArray >= theNumberOfPoints
				&& leftSonIndexInArray >= theNumberOfPoints)
		{
			return true;
		}
		else
		{
			if (leftSonIndexInArray < theNumberOfPoints)
			{
				if (theDimensions[dimension][index]
						>= theDimensions[dimension][leftSonIndexInArray])
				{
					test_correct_build(leftSonIndexInArray,
							(dimension + 1) % numberOfDimensions);

				}
				else
					return false;
			}

			if (rightSonIndexInArray < theNumberOfPoints)
			{
				if (theDimensions[dimension][index]
						<= theDimensions[dimension][rightSonIndexInArray])
				{
					test_correct_build(rightSonIndexInArray,
							(dimension + 1) % numberOfDimensions);

				}
				else
					return false;

			}

		}

	}

	bool test_correct_search(const std::vector<unsigned int> foundPoints, const KDPoint<TYPE, numberOfDimensions>& minPoint,
			const KDPoint<TYPE, numberOfDimensions>& maxPoint) const
	{
		for(unsigned int i= 0; i<theNumberOfPoints; ++i)
		{
			if(std::find(foundPoints.begin(), foundPoints.end(), thePoints[i].theId)!=foundPoints.end())
			{
				bool inTheBox = true;
				for (int dim = 0; dim < numberOfDimensions; ++dim)
					{
						inTheBox &= (thePoints[i][dim] <= maxPoint[dim]
								&& thePoints[i][dim] >= minPoint[dim]);
					}

				if(!inTheBox)
				{
					std::cerr << "Point " << thePoints[i].theId << " was wrongly found to be in the box." << std::endl;


					return false;

				}
			} else
			{

				bool inTheBox = true;
				for (int dim = 0; dim < numberOfDimensions; ++dim)
					{
						inTheBox &= (thePoints[i][dim] <= maxPoint[dim]
								&& thePoints[i][dim] >= minPoint[dim]);
					}

				if(inTheBox)
				{
					std::cerr << "Point " << thePoints[i].theId << " was wrongly found to be outside the box." << std::endl;


				}

			}


		}
		std::cout << "Search correctness test completed successfully." << std::endl;
		return true;
	}


	std::vector<TYPE> getDimensionVector(const int dimension) const
	{
		if (dimension < numberOfDimensions)
			return theDimensions[dimension];
	}

	std::vector<unsigned int> getIdVector() const
	{
		return theIds;
	}
	void build();
private:

	unsigned int partition_complete_kdtree(unsigned int length)
	{
		if (length == 1)
			return 0;
		unsigned int index = 1 << ((int) log2(length));

		if ((index / 2) - 1 <= length - index)
			return index - 1;
		else
			return length - index / 2;

	}

	unsigned int leftSonIndex(unsigned int index) const;
	unsigned int rightSonIndex(unsigned int index) const;
	bool intersects(unsigned int index,
			const KDPoint<TYPE, numberOfDimensions>& minPoint,
			const KDPoint<TYPE, numberOfDimensions>& maxPoint,
			int dimension) const;
	bool isInTheBox(unsigned int index,
			const KDPoint<TYPE, numberOfDimensions>& minPoint,
			const KDPoint<TYPE, numberOfDimensions>& maxPoint) const;
	long int theNumberOfPoints;
	int theDepth;
	std::vector<KDPoint<TYPE, numberOfDimensions> > thePoints;
	std::array<std::vector<TYPE>, numberOfDimensions> theDimensions;
	std::vector<unsigned int> theIntervalLength;
	std::vector<unsigned int> theIntervalMin;
	std::vector<unsigned int> theIds;

};


template<class TYPE, int numberOfDimensions>
std::vector<unsigned int> FKDTree<TYPE,numberOfDimensions>::search_in_the_box(
		const KDPoint<TYPE, numberOfDimensions>& minPoint,
		const KDPoint<TYPE, numberOfDimensions>& maxPoint) const
{
//			std::deque<unsigned int> indecesToVisit;
	FQueue<unsigned int> indecesToVisit(128);
	std::vector<unsigned int> result;
	result.reserve(16);
	indecesToVisit.push_back(0);

	for (int depth = 0; depth < theDepth + 1; ++depth)
	{

		int dimension = depth % numberOfDimensions;
		unsigned int numberOfIndecesToVisitThisDepth =
				indecesToVisit.size();
		for (unsigned int visitedIndecesThisDepth = 0;
				visitedIndecesThisDepth < numberOfIndecesToVisitThisDepth;
				visitedIndecesThisDepth++)
		{

			unsigned int index = indecesToVisit[visitedIndecesThisDepth];
			//				assert(index >= 0 && index < theNumberOfPoints);
			bool intersection = intersects(index, minPoint, maxPoint,
					dimension);

			if (intersection && isInTheBox(index, minPoint, maxPoint))
				result.push_back(index);

			bool isLowerThanBoxMin = theDimensions[dimension][index]
					< minPoint[dimension];

			int startSon = isLowerThanBoxMin; //left son = 0, right son =1

			int endSon = isLowerThanBoxMin || intersection;

			for (int whichSon = startSon; whichSon < endSon + 1; ++whichSon)
			{
				unsigned int indexToAdd = leftSonIndex(index) + whichSon;

				if (indexToAdd < theNumberOfPoints)
				{

					//						assert(
					//								indexToAdd >= (1 << (depth + 1)) - 1
					//										&& leftSonIndex(index) + whichSon
					//												< ((1 << (depth + 2)) - 1));
					indecesToVisit.push_back(indexToAdd);
				}

			}

		}

		indecesToVisit.pop_front(numberOfIndecesToVisitThisDepth);
//					indecesToVisit.erase(indecesToVisit.begin(),
//							indecesToVisit.begin() + numberOfIndecesToVisitThisDepth);
	}
	return result;
}



template<class TYPE, int numberOfDimensions>

void FKDTree<TYPE,numberOfDimensions>::build()
{
	//gather kdtree building
	int dimension;
	theIntervalMin[0] = 0;
	theIntervalLength[0] = theNumberOfPoints;

	for (int depth = 0; depth < theDepth; ++depth)
	{

		dimension = depth % numberOfDimensions;
		unsigned int firstIndexInDepth = (1 << depth) - 1;
		for (int indexInDepth = 0; indexInDepth < (1 << depth);
				++indexInDepth)
		{
			unsigned int indexInArray = firstIndexInDepth + indexInDepth;
			unsigned int leftSonIndexInArray = 2 * indexInArray + 1;
			unsigned int rightSonIndexInArray = leftSonIndexInArray + 1;

			unsigned int whichElementInInterval = partition_complete_kdtree(
					theIntervalLength[indexInArray]);
			std::nth_element(
					thePoints.begin() + theIntervalMin[indexInArray],
					thePoints.begin() + theIntervalMin[indexInArray]
							+ whichElementInInterval,
					thePoints.begin() + theIntervalMin[indexInArray]
							+ theIntervalLength[indexInArray],
					[dimension](const KDPoint<TYPE,numberOfDimensions> & a, const KDPoint<TYPE,numberOfDimensions> & b) -> bool
					{
						if(a[dimension] == b[dimension])
						return a.getId() < b.getId();
						else
						return a[dimension] < b[dimension];
					});
			add_at_position(
					thePoints[theIntervalMin[indexInArray]
							+ whichElementInInterval], indexInArray);

			if (leftSonIndexInArray < theNumberOfPoints)
			{
				theIntervalMin[leftSonIndexInArray] =
						theIntervalMin[indexInArray];
				theIntervalLength[leftSonIndexInArray] =
						whichElementInInterval;
			}

			if (rightSonIndexInArray < theNumberOfPoints)
			{
				theIntervalMin[rightSonIndexInArray] =
						theIntervalMin[indexInArray]
								+ whichElementInInterval + 1;
				theIntervalLength[rightSonIndexInArray] =
						(theIntervalLength[indexInArray] - 1
								- whichElementInInterval);
			}
		}
	}

	dimension = theDepth % numberOfDimensions;
	unsigned int firstIndexInDepth = (1 << theDepth) - 1;
	unsigned int indexInArray = firstIndexInDepth;
	for (unsigned int indexInArray = firstIndexInDepth;
			indexInArray < theNumberOfPoints; ++indexInArray)
	{
		add_at_position(thePoints[theIntervalMin[indexInArray]],
				indexInArray);

	}

}

template<class TYPE, int numberOfDimensions>

unsigned int FKDTree<TYPE,numberOfDimensions>::leftSonIndex(unsigned int index) const
{
	return 2 * index + 1;
}

template<class TYPE, int numberOfDimensions>

unsigned int FKDTree<TYPE,numberOfDimensions>::rightSonIndex(unsigned int index) const
{
	return 2 * index + 2;
}

template<class TYPE, int numberOfDimensions>

bool FKDTree<TYPE,numberOfDimensions>::intersects(unsigned int index,
		const KDPoint<TYPE, numberOfDimensions>& minPoint,
		const KDPoint<TYPE, numberOfDimensions>& maxPoint,
		int dimension) const
{
	return (theDimensions[dimension][index] <= maxPoint[dimension]
			&& theDimensions[dimension][index] >= minPoint[dimension]);
}

template<class TYPE, int numberOfDimensions>

bool FKDTree<TYPE,numberOfDimensions>::isInTheBox(unsigned int index,
		const KDPoint<TYPE, numberOfDimensions>& minPoint,
		const KDPoint<TYPE, numberOfDimensions>& maxPoint) const
{
	bool inTheBox = true;
#pragma unroll
	for (int i = 0; i < numberOfDimensions; ++i)
	{
		inTheBox &= (theDimensions[i][index] <= maxPoint[i]
				&& theDimensions[i][index] >= minPoint[i]);
	}

	return inTheBox;
}

#endif /* FKDTREE_FKDTREE_H_ */
