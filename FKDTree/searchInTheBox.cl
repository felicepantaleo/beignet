#define MAX_SIZE 15
#define NUM_DIMENSIONS 3
#define RANGE 0.1
#define MAX_RESULT_SIZE 100
typedef struct
{

	unsigned int data[MAX_SIZE];
	unsigned int front;
	unsigned int tail;
	unsigned int size;
} Queue;
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
	if (queue->size > 0)
	{
		unsigned int element = queue->front;
		queue->front = (queue->front + 1) % MAX_SIZE;
		queue->size--;
		return element;
	}
}

void erase_first_n_elements(Queue* queue, unsigned int n)
{
	unsigned int elementsToErase = queue->size - n > 0 ? n : queue->size;
	queue->front = (queue->front + elementsToErase) % MAX_SIZE;

}


unsigned int leftSonIndex(unsigned int index) const
{
	return 2 * index + 1;
}


unsigned int rightSonIndex(unsigned int index) const
{
	return 2 * index + 2;
}


bool intersects(unsigned int index, float* theDimensions, unsigned int nPoints,
		float* minPoint, float* maxPoint, int dimension) const
{
	return (theDimensions[nPoints * dimension + index] <= maxPoint[dimension]
			&& theDimensions[nPoints * dimension + index] >= minPoint[dimension]);
}


bool isInTheBox(unsigned int index, float* theDimensions, unsigned int nPoints,
		float* minPoint, float* maxPoint) const
{
	bool inTheBox = true;
	for (int i = 0; i < NUM_DIMENSIONS; ++i)
	{
		inTheBox &= (theDimensions[nPoints * i + index] <= maxPoint[i]
				&& theDimensions[i][index] >= minPoint[i]);
	}

	return inTheBox;
}
__kernel void SearchInTheKDBox(unsigned int nPoints, __global float* dimensions, __global unsigned int* ids, __global unsigned int* results)
{

	unsigned int threadIdx = get_local_id(0);
	unsigned int blockIdx = get_group_id(0);
	unsigned int point_index = threadIdx + blockIdx * get_local_size(0);

	if(point_index < nPoints)
	{
		results[point_index] = 0;

		int theDepth = floor(log2((float)nPoints));

		float minPoint[NUM_DIMENSIONS];
		float maxPoint[NUM_DIMENSIONS];
		for(int i = 0; i<NUM_DIMENSIONS; ++i)
		{
			minPoint[i] = dimensions[nPoints*i+point_index] - RANGE;
			maxPoint[i] = dimensions[nPoints*i+point_index] + RANGE;

		}

		Queue indecesToVisit;
		indecesToVisit.front = indecesToVisit.tail =indecesToVisit.size =0;
		unsigned int pointsFound=0;
		unsigned int resultIndex = nPoints*(point_index+1);
		push_back(&indecesToVisit, 0);

		for (int depth = 0; depth < theDepth + 1; ++depth)
		{
			int dimension = depth % NUM_DIMENSIONS;
			unsigned int numberOfIndecesToVisitThisDepth =
			indecesToVisit.size;
			for (unsigned int visitedIndecesThisDepth = 0;
					visitedIndecesThisDepth < numberOfIndecesToVisitThisDepth;
					visitedIndecesThisDepth++)
			{

				unsigned int index = indecesToVisit.data[indecesToVisit.front+visitedIndecesThisDepth];

				bool intersection = intersects(index,dimensions, nPoints, minPoint, maxPoint,
						dimension);

				if(intersection && isInTheBox(index, dimensions, nPoints, minPoint, maxPoint))
				{
					if(pointsFound < MAX_RESULT_SIZE)
					{
						results[resultIndex] = index;
						resultIndex++;
						pointsFound++;
					}

				}

				bool isLowerThanBoxMin = theDimensions[nPoints*dimension + index]
				< minPoint[dimension];
				int startSon = isLowerThanBoxMin; //left son = 0, right son =1

				int endSon = isLowerThanBoxMin || intersection;

				for (int whichSon = startSon; whichSon < endSon + 1; ++whichSon)
				{
					unsigned int indexToAdd = leftSonIndex(index) + whichSon;

					if (indexToAdd < nPoints)
					{
						push_back(&indecesToVisit,indexToAdd);

					}
				}
			}

			erase_first_n_elements(&indecesToVisit,numberOfIndecesToVisitThisDepth );
		}

	}

}

