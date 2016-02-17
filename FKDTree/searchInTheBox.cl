#define MAX_SIZE 15

struct Queue
{
	Queue() :
			front(0), tail(0), size(0)
	{
	}

	unsigned int data[MAX_SIZE];
	unsigned int front;
	unsigned int tail;
	unsigned int size;
};
bool push_back(Queue* queue, unsigned int index)
{
	if (queue->size < MAX_SIZE)
	{
		queue->data[queue->tail] = index;
		queue->tail = (queue->tail + 1) % MAX_SIZE;
		queue->size++;
		return true;
	}
	return false;

}

unsigned int pop_front(Queue* queue)
{
	if(queue->size > 0)
	{
		unsigned int element = queue->front;
		queue->front=(queue->front+1)% MAX_SIZE;
		queue->size--;
		return element;
	}
}

__kernel void SearchInTheKDBox(unsigned int nPoints, __global float* dimensions, __global unsigned int* ids, __global unsigned int* results)
{

	unsigned int threadIdx = get_local_id(0);
	unsigned int blockIdx = get_group_id(0);
	unsigned int point_index = threadIdx + blockIdx * get_local_size(0);


	Queue indecesToVisit;
//		std::deque<unsigned int> indecesToVisit;
//		std::vector<KDPoint<TYPE, numberOfDimensions> > result;
//
//		indecesToVisit.push_back(0);
//
//		for (int depth = 0; depth < theDepth + 1; ++depth)
//		{
//
//			int dimension = depth % numberOfDimensions;
//			unsigned int numberOfIndecesToVisitThisDepth =
//					indecesToVisit.size();
//			for (unsigned int visitedIndecesThisDepth = 0;
//					visitedIndecesThisDepth < numberOfIndecesToVisitThisDepth;
//					visitedIndecesThisDepth++)
//			{
//
//				unsigned int index = indecesToVisit[visitedIndecesThisDepth];
////				assert(index >= 0 && index < theNumberOfPoints);
//				bool intersection = intersects(index, minPoint, maxPoint,
//						dimension);
//
//				if (intersection && isInTheBox(index, minPoint, maxPoint))
//					result.push_back(getPoint(index));
//
//				bool isLowerThanBoxMin = theDimensions[dimension][index]
//						< minPoint[dimension];
//
//				int startSon = isLowerThanBoxMin; //left son = 0, right son =1
//
//				int endSon = isLowerThanBoxMin || intersection;
//
//				for (int whichSon = startSon; whichSon < endSon + 1; ++whichSon)
//				{
//					unsigned int indexToAdd = leftSonIndex(index) + whichSon;
//
//					if (indexToAdd < theNumberOfPoints)
//					{
//
////						assert(
////								indexToAdd >= (1 << (depth + 1)) - 1
////										&& leftSonIndex(index) + whichSon
////												< ((1 << (depth + 2)) - 1));
//						indecesToVisit.push_back(indexToAdd);
//					}
//
//				}
//
//			}
////			std::cout << "starting " <<numberOfIndecesToVisitThisDepth<< std::endl;
////			std::cout << "finishing " <<numberOfIndecesToVisitThisDepth<< std::endl;
//
//			indecesToVisit.erase(indecesToVisit.begin(),
//					indecesToVisit.begin() + numberOfIndecesToVisitThisDepth);
//		}
//		return result;
//	}

}
