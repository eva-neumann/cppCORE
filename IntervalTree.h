#ifndef INTERVALTREE_H
#define INTERVALTREE_H

#include "cppCORE_global.h"
#include <iterator>
#include <QVector>
#include <QSharedDataPointer>
#include <QSharedData>
#include <QScopedPointer>



/// Represents an interval with start and end position and an extra information field value, which can be used for storing the index of an interval
// TODO Reicht struct hier?
struct CPPCORESHARED_EXPORT Interval
{
    Interval();
    Interval(int s, int e, int value = -1);
    Interval(const QVector<Interval>& intervals);
    bool isValid() const { return ((start!=-1) && (end!=-1)); }
    int start;
    int end;
    int value;
};

/// Represents the shared data object of the IntervalTree class. This class represents the actual interval tree.
/// Each node holds a center position, a vector of intervals, a pointer to a left and the right sub trees.
class CPPCORESHARED_EXPORT IntervalTreeData : public QSharedData
{
public:
    /// Default constructor for an empty tree
    IntervalTreeData();
    /// Constructor
    IntervalTreeData(QVector< Interval> & intervals,
                     int depth = 16,
                     int leftextent = 0,
                     int rightextent = 0,
                     int maxbucket = 512);
    /// Copy constructor (supports implicit sharing)
    IntervalTreeData(const IntervalTreeData& other);

    /// Returns the depth of the tree
    int depth() const;
    /// Returns the total number of intervals in the tree
    int numberIntervals() const;

    /// Determine recursively the overlapping intervals of [start, stop] in the interval tree
    void findOverlappingIntervals(int start, int stop, QVector<Interval>& matches, bool stop_at_first_match=false) const;

    void subtractTree(const IntervalTreeData& other, QVector<Interval>& remaining_intervals) const;

    void allIntervals(QVector<Interval>& intervals) const;


protected:
    /// The intervals below the root of the interval tree
    QVector<Interval> intervals_;
    /// Pointer to the left sub interval tree
    QScopedPointer<IntervalTreeData> left_;
    /// Pointer to the right sub interval tree
    QScopedPointer<IntervalTreeData> right_;
    /// The center position at this node in the tree, i.e. all intervals which are located left of this position
    /// are stored in the left subtree and intervals which are located right to this position are stored in the right
    /// subtree. However, the intervals overlapping with the center position are stored in intervals_.
    int center_;
    int size_;


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

/// Represents an balanced (using the central point) Interval tree data structure.
/// (Note: It is build upon a set of intervals, which cannot be changed after its initialization.)
class CPPCORESHARED_EXPORT IntervalTree
{
public:
    IntervalTree();
    IntervalTree(QVector< Interval> & intervals,
                 int depth = 16,
                 int leftextent = 0,
                 int rightextent = 0,
                 int maxbucket = 512);

    /// Copy constructor supports implicit sharing of the underlying shared data object IntervalTreeData
    IntervalTree(const IntervalTree& other);

    /// Returns true if the tree is empty
    bool isEmpty() { return !d_; }

    /// Determine the overlapping intervals of [start, stop] in the interval tree. If stop_at_first_match is true,
    /// only the first overlapping interval is returned
    void overlappingIntervals(int start, int stop, QVector<Interval>& matches, bool stop_at_first_match=false) const;

    void subtractTree(const IntervalTree& other,QVector<Interval>& remaining_intervals) const;

    void allIntervals(QVector<Interval>& intervals) const;


private:
    /// Shared pointer to the actual tree to support implicit sharing
    QSharedDataPointer<IntervalTreeData> d_;

    /// Number intervals in tree
    int size_;
};


#endif
