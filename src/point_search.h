/*___________________________________________________________________________________________________________________________*/
/* Given 5 million ranked points on a 2D plane, design a datastructure and an algorithm that can find the 20 most important
points inside any given rectangle. The solution has to be reasonably fast even in the worst case. It also shouldn't use an
unreasonably large amount of memory. Give me your thought process about how you would solve this problem (tell me about the
pros and cons of each approach).
Create a 32bit Windows DLL, that can be loaded by the test application. The DLL and the functions will be loaded dynamically.
The DLL should export the following functions: "create", "search" and "destroy". The signatures and descriptions of these
functions are given below. You can use any language or compiler, as long as the resulting DLL implements this interface. */
/*___________________________________________________________________________________________________________________________*/
typedef char I8; // 8 bit signed integer
typedef int I32; // 32 bit signed integer
typedef float F32; // IEEE 754 floating point number
#define STDCALL __stdcall // standard calling convention

/*___________________________________________________________________________________________________________________________*/
/* The following structs are packed with no padding. */
#pragma pack(push, 1)

/* Defines a point in 2D space with some additional attributes like id and rank. */
struct Point
{
	I8 id;
  I32 rank;
  F32 x;
  F32 y;
};

/* Defines a rectangle, where a point is inside, if x is in [lx, hx] and y is in [ly, hy]. */
struct Rect
{
	F32 lx;
	F32 ly;
	F32 hx;
	F32 hy;
};
#pragma pack(pop)

struct Object;

/*___________________________________________________________________________________________________________________________*/
/* Load the provided points into an internal data structure. The input points are only guaranteed to be valid for the
duration of the call. Return an object pointer that can be used for consecutive searches on the data. */
typedef Object* (STDCALL* Tcreate)(const Point* points_begin, const Point* points_end);

/* Search for "count" points with the smallest ranks inside "rect" and copy them ordered by smallest rank first in
"out_points". Return the number of points actually found. "out_points" points to a buffer owned by the caller that
can hold "count" number of Points. */
typedef I32 (STDCALL* Tsearch)(Object* o, const Rect rect, const I32 count, Point* out_points);

/* Release the resources associated with the object. Return 0 if successful. */
typedef I32 (STDCALL* Tdestroy)(Object* o);

/*___________________________________________________________________________________________________________________________*/
