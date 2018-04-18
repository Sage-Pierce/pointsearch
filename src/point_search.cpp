/*
Design and Implementation Notes and Ideas:

The first thing Object will need to have is a copy of all the points since "the
points are only valid for the duration of the call." The points will have to be
sorted by rank at some point, so let's sort the points by rank after copying
the points.

Here are some of my different algorithm thoughts:
- The easy way to do this would just be to look through all points when we call
  search and store pointers to all the points that are within "rect". Since we
  already sorted them by rank, the first "count" points in the "rect" are our
  answer
  - Downside here is that it would usually take O(n) accesses to search a
    small "rect"
  - Upside here is that this is easy to implement and probably on par with any
    other algorithms with a large "rect"

- Another approach is to use as much of the information we have in the "create"
  method to make data structures that allow us to more quickly choose points
  that might be in the "rect."
  - Add two data structures of pointers to the array of all points and sort one
    by x-coordinate and one by y-coordinate. This will allow logarithmically
    searching all the points that have either an x-coordiante or y-coordinate
    withing "rect"
  - Union the two collections and sort by rank. Return first "count" points
  - Downside is the "union and sort" will be difficult to implement without
    introducing a O(n*log(n)) time complexity.
  - Upside is this would likely be better for small "rects"

- A "fastest-on-average" approach might use a combo of above. The dependency
  would be on the size of rect

- Another way that occurred to me is using a quad-tree
(http://en.wikipedia.org/wiki/Quadtree).
  - On create, add all the points to the quadtree
  - On search, start at the center of the "rect" and remove the point that's
    located in that area. Repeat this process until all four points of the 
    "rect" are contained in the same area of the quad tree
  - Create would take O(n*log(n))
  - Search would take a time dependent on the size of the "rect"

I decided to tackle the more complicated of the first two. After attempting to 
do so, I think I may have overthought the problem and might have been better
off doing the more simpler algorithm.... X)

Some stats (based on 32-bit machine):
( m = size (in memory) of a Point; n = number of points)
-create
  - Allocates an object of size 1.8*m*n
  - Executes in 3*n*log(n) = O(n*log(n))
-search
  - Allocates a temporary array of size .4*m*n
  - Executes in 4*log(n) + O(n) + O(n*log(n)) = O(n*log(n))
*/


#include "stdafx.h"
#include "point_search.h"
#include <stdlib.h>

#define X_AXIS 0
#define Y_AXIS 1
#define GET_NUMBER(POINT_PTR,AXIS) ((AXIS == X_AXIS) ? (POINT_PTR)->x : (POINT_PTR)->y)
#define POINT_IN_RECT(P,R) ((P->x >= R.lx) && (P->x <= R.hx) && (P->y >= R.ly) && (P->y <= R.hy))

#define LINEAR
//#define DEBUG
#ifdef DEBUG
#include <iostream>
using std::cout;
using std::endl;
#endif

struct Object {
  I32 numPoints;
  Point* allPoints;
  Point** xSorted;
  Point** ySorted;
};

extern "C" Object* STDCALL create(const Point*, const Point*);
extern "C" I32 STDCALL search(Object*, const Rect, const I32, Point*);
extern "C" I32 STDCALL destroy(Object*);
Point** safeSearch(I32 axis, F32 number, bool low, Point** begin, Point** end);
Point** binarySearch(I32 axis, F32 number, bool low, Point** begin, Point** end);
int rankComp(const void*, const void*);
int pointerComp(const void*, const void*);
int xComp(const void*, const void*);
int yComp(const void*, const void*);

Object* STDCALL create(const Point* points_begin, const Point* points_end) {
    // Create Object on heap and allocate necessary memory for it
  Object* object = new Object;
  object->numPoints = points_end - points_begin; //+ 1;
  object->allPoints = new Point[object->numPoints];
#ifndef LINEAR
  object->xSorted = new Point*[object->numPoints];
  object->ySorted = new Point*[object->numPoints];
#endif
//  object->allPoints = (Point*) ::operator new((size_t)object->numPoints * sizeof(Point));
//  object->xSorted = (Point**) ::operator new((size_t)object->numPoints * sizeof(Point*));
//  object->ySorted = (Point**) ::operator new((size_t)object->numPoints * sizeof(Point*));

    // Put all the points in to the data structure
  const Point* iterator = points_begin;
  while (iterator != points_end) { //(points_end + 1)) {
    I32 index = iterator - points_begin;
    object->allPoints[index] = *iterator;
#ifndef LINEAR
    object->xSorted[index] = &object->allPoints[index];
    object->ySorted[index] = &object->allPoints[index];
#endif
    iterator++;
  }

    // Now sort the data structures
  qsort(object->allPoints, object->numPoints, sizeof(Point), rankComp); // important that this comes first
#ifndef LINEAR
  qsort(object->xSorted, object->numPoints, sizeof(Point*), xComp);
  qsort(object->ySorted, object->numPoints, sizeof(Point*), yComp);
#endif
  return object;
}

I32 STDCALL search(Object* o, const Rect rect, const I32 count, Point* out_points) {
    // Check boundary condition
  if (o->numPoints <= 0)
    return 0;

#ifdef LINEAR
  I32 numResults = 0;
  Point* iterator = o->allPoints;
  for (Point* iterator = o->allPoints; ((numResults < count) && (iterator != (o->allPoints+o->numPoints))); iterator++) {
    if (POINT_IN_RECT(iterator, rect))
      out_points[numResults++] = *iterator;
  }
#else
    // Get pointers to beginning and end Points that have either an x or y coordinate in bounds.
    // This will give us two collections for which we simply need to take the union
  Point** xBegin = safeSearch(X_AXIS, rect.lx, true,  o->xSorted, (o->xSorted + o->numPoints));
  Point** xEnd   = safeSearch(X_AXIS, rect.hx, false, o->xSorted, (o->xSorted + o->numPoints));
  Point** yBegin = safeSearch(Y_AXIS, rect.ly, true,  o->ySorted, (o->ySorted + o->numPoints));
  Point** yEnd   = safeSearch(Y_AXIS, rect.hy, false, o->ySorted, (o->ySorted + o->numPoints));

    // Create a data structure to store the merged results
  I32 numElements = (xEnd - xBegin) + (yEnd - yBegin);
  Point** mergedResults = new Point*[numElements];
//  Point** mergedResults = (Point**) ::operator new((size_t)numElements * sizeof(Point*));
  I32 index = 0;
  for (I32 i = 0; i < (xEnd - xBegin); i++)
    mergedResults[index++] = xBegin[i];
  for (I32 i = 0; i < (yEnd - yBegin); i++)
    mergedResults[index++] = yBegin[i];

    // Sort by rank (just compare pointer value) and any two adjacent pointers that are
    // equal are a valid result. Collect the first 'count' results or until we reach the
    // end of the results array.
  qsort(mergedResults, numElements, sizeof(Point*), pointerComp);
  I32 numResults = 0;
  for (I32 i = 0; ((i < numElements) && (numResults < count)); i++) {
    if (mergedResults[i] == mergedResults[i + 1])
      out_points[numResults++] = *mergedResults[i++];
  }

  delete[] mergedResults;
//  ::operator delete(mergedResults);
#endif
  return numResults;
}

I32 STDCALL destroy(Object* o) {
#ifndef LINEAR
  delete[] o->ySorted;
  delete[] o->xSorted;
#endif
  delete[] o->allPoints;
//  ::operator delete(o->ySorted);
//  ::operator delete(o->xSorted);
//  ::operator delete(o->allPoints);
  return 0;
}

// Check boundary conditions first
Point** safeSearch(I32 axis, F32 number, bool low, Point** begin, Point** end) {
  F32 beginNumber = GET_NUMBER(*begin, axis);
  F32 endNumber = GET_NUMBER(*(end - 1), axis);
  if (number <= beginNumber)    return begin;
  else if (number >= endNumber) return end;
  else                          return binarySearch(axis, number, low, begin, end);
}

Point** binarySearch(I32 axis, F32 number, bool low, Point** begin, Point** end) {
  F32 beginNumber = GET_NUMBER(*begin, axis);
  F32 endNumber = GET_NUMBER(*(end - 1), axis);

  if (number < beginNumber)    return begin;
  else if (number > endNumber) return end;
  else if ((end - begin) <= 2) {
    if (low)
      return (number < beginNumber) ? (end - 1) : begin;
    else
      return (number > beginNumber) ? end : (begin + 1);
  }
  else if (number == beginNumber) {
    if (low) {
      while ((GET_NUMBER(*(begin - 1), axis)) == number)
        begin--;
    }
    else {
      while ((GET_NUMBER(*begin, axis)) == number)
        begin++;
    }
    return begin;
  }
  else if (number == endNumber) {
    if (low) {
      while ((GET_NUMBER(*(end - 1), axis)) == number)
        end--;
    }
    else {
      while ((GET_NUMBER(*end, axis)) == number)
        end++;
    }
    return end;
  }
  else {
    Point** halfWay = begin + ((end - begin) / 2);
    F32 halfWayNumber = GET_NUMBER(*halfWay, axis);
    if (halfWayNumber <= number)
      return binarySearch(axis, number, low, halfWay, end);
    else
      return binarySearch(axis, number, low, begin, halfWay);
  }
}

int rankComp(const void* a, const void* b) {
  Point* aPt = (Point*)a;
  Point* bPt = (Point*)b;
  if (aPt->rank != bPt->rank)
    return aPt->rank - bPt->rank;
  else if (aPt->id != bPt->id)
    return aPt->id - bPt->id;
  else if (aPt->x != bPt->x)
    return (aPt->x > bPt->x) ? 1 : -1;
  else
    return (aPt->y > bPt->y) ? 1 : -1;
}

int pointerComp(const void* a, const void* b) {
  return *((Point**)a) - *((Point**)b); // Not a typo. Address is directly related to ranking
//  return rankComp(*((Point**)a), *((Point**)b));
}

int xComp(const void* a, const void* b) {
  Point* aPt = *((Point**)a);
  Point* bPt = *((Point**)b);
  return (aPt->x > bPt->x) ? 1 : -1;
}

int yComp(const void* a, const void* b) {
  Point* aPt = *((Point**)a);
  Point* bPt = *((Point**)b);
  return (aPt->y > bPt->y) ? 1 : -1;
}

#ifdef DEBUG
int main(int argc, char** argv) {
  int NUM_POINTS = 50;
  Point* points = new Point[NUM_POINTS];
  for (int i = 0; i < NUM_POINTS; i++) {
    points[i].id = i;
    points[i].rank = rand();
    points[i].x = (F32)(rand()%10);
    points[i].y = (F32)(rand()%10);
  }

  Object* obj = create(points, &points[NUM_POINTS]);
  cout << "Points: (" << NUM_POINTS << ")" << endl;
  for (I32 i = 0; i < NUM_POINTS; i++) {
    cout << "Point ID: " << (int)points[i].id << " Rank: " << (int)points[i].rank << "; " << "X: " << points[i].x << "; " << "Y: " << points[i].y << endl;
  }
  Rect rect;
  rect.lx = 2;
  rect.ly = 2;
  rect.hx = 8;
  rect.hy = 8;
  Point results[20];
  I32 numResults = search(obj, rect, 20, results);


  cout << "RESULTS: (" << numResults << ")" << endl;
  for (I32 i = 0; i < numResults; i++) {
    cout << "Point ID: " << (int)results[i].id << " Rank: " << (int)results[i].rank << "; " << "X: " << results[i].x << "; " << "Y: " << results[i].y << endl;
  }

  destroy(obj);
}
#endif

