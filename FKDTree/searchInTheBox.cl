__kernel SearchInTheKDBox(__global float** dimensions, __global unsigned int* ids, __global float* minPoint, __global float* maxPoint, __global unsigned int* results)
{
	std::vector<KDPoint<TYPE, numberOfDimensions> > search_in_the_box(
			const KDPoint<TYPE, numberOfDimensions>& minPoint,
			const KDPoint<TYPE, numberOfDimensions>& maxPoint) const
	{
		std::deque<unsigned int> indecesToVisit;
		std::vector<KDPoint<TYPE, numberOfDimensions> > result;

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
					result.push_back(getPoint(index));

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
//			std::cout << "starting " <<numberOfIndecesToVisitThisDepth<< std::endl;
//			std::cout << "finishing " <<numberOfIndecesToVisitThisDepth<< std::endl;

			indecesToVisit.erase(indecesToVisit.begin(),
					indecesToVisit.begin() + numberOfIndecesToVisitThisDepth);
		}
		return result;
	}














}
