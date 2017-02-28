#ifndef INTERVALTREE_H
#define INTERVALTREE_H

#include "cppCORE_global.h"
#include <QVector>
#include <QPair>
#include <QScopedPointer>



/// Represents an interval with start and end position and an extra information field value, which can be used for storing the index of an interval
struct CPPCORESHARED_EXPORT Interval
{
    Interval();
    Interval(int s, int e, int value = 0);
    int start;
    int end;
    int value;
};

/// Represents an balanced Interval tree data structure.
/// (Note: It is build upon a set of intervals, which cannot be changed after its initialization.)
class CPPCORESHARED_EXPORT IntervalTree
{

public:
    /// Default constructor for an empty tree
    IntervalTree();
    /// Constructor
    IntervalTree(const QVector< Interval> & intervals,
                  int depth = 16,
                  int leftextent = 0,
                  int rightextent = 0,
                  int maxbucket = 512);

    /// Copy constructor (deep copy)
    IntervalTree(const IntervalTree& other);
    /// Assignment operator (deep copy)
    IntervalTree& operator=(const IntervalTree& other);
    /// Determine the overlapping intervals of [start, stop] in the interval tree. If stop_at_first_match is true,
    /// only the first overlapping interval is returned
    QVector<Interval> overlappingIntervals(int start, int stop, bool stop_at_first_match=false) const;

 protected:
    /// The intervals below the root of the interval tree
    QVector<Interval> intervals_;
    /// Pointer to the left sub interval tree
    QScopedPointer<IntervalTree> left_;
    /// Pointer to the right sub interval tree
    QScopedPointer<IntervalTree> right_;
    /// Center start position of all sorted intervals in the tree
    int center_;


    /// Determine recursively the overlapping intervals of [start, stop] in the interval tree
    void findOverlappingIntervals(int start, int stop, QVector<Interval>& matches, bool stop_at_first_match) const;

    /// Allows the sorting of intervals by start position
    static bool startPositionComparator(const Interval& a, const Interval& b)
    {
        return	a.start< b.start;
    }

    /// Return the end position of an interval
    static int endPosition(const Interval& a)
    {
        return	a.end;
    }
};

#endif
