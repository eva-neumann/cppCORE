#ifndef INTERVALTREE_H
#define INTERVALTREE_H

#include "cppCORE_global.h"
#include <iterator>
#include <QVector>
#include <QLinkedList>
#include <QSharedDataPointer>
#include <QSharedData>
#include <QScopedPointer>
// TODO remove include
#include <QTextStream>
#include "Helper.h"



/// Represents an interval with start and end position and an extra information field value, which can be used for storing the index of an interval
// TODO Reicht struct hier?
class CPPCORESHARED_EXPORT Interval
{
public:
    Interval();
    Interval(int s, int e, int v=-1);
    Interval(const Interval &other);
    Interval(const QVector<Interval>& intervals);
    bool isValid() const { return ((start_!=-1) && (end_!=-1)); }

    int end() const
    {
        return end_;
    }

    int start() const
    {
        return start_;
    }

    int value() const
    {
        return value_;
    }
protected:
    int start_;
    int end_;
    int value_;
};

template <class T>
class CPPCORESHARED_EXPORT MinStartPositionContainer : public std::binary_function<int,int,bool>
{
public:
    MinStartPositionContainer(const T& container) : container_(container)
    {}

    bool operator()(int a, int b ) const
    {
        return container_[a].start() < container_[b].start();
    }

protected:
    const T& container_;
};

template <class T>
class CPPCORESHARED_EXPORT MaxEndPositionContainer: public std::binary_function<int,int,bool>
{
public:
    MaxEndPositionContainer(const T& container) : container_(container)
    {}

    bool operator()(int a, int b ) const
    {
        return container_[a].end() < container_[b].end();
    }

protected:
    const T& container_;
};


struct CPPCORESHARED_EXPORT MinStartPositionInterval: public std::binary_function<Interval,Interval,bool>
{
    bool operator()(const Interval& a, const Interval& b ) const
    {
        return a.start() < b.start();
    }

};

struct CPPCORESHARED_EXPORT MaxEndPositionInterval : public std::binary_function<Interval,Interval,bool>
{
    bool operator()(const Interval& a, const Interval& b ) const
    {
        return a.end()< b.end();
    }
};

/// Represents the shared data object of the IntervalTree class. This class represents the actual interval tree.
/// Each node holds a center position, a vector of intervals, a pointer to a left and the right sub trees.
template <class T>
class CPPCORESHARED_EXPORT IntervalTreeData : public QSharedData
{
public:
    /// Default constructor for an empty tree
    IntervalTreeData();
    /// Constructor
    IntervalTreeData(const T& container,
                     std::list<int>& indices,
                     std::list<int>::iterator start_it,
                     std::list<int>::iterator end_it,
                     int leftextent = 0,
                     int rightextent = 0);

    /// Copy constructor (supports implicit sharing)
    IntervalTreeData(const IntervalTreeData& other);

    /// Determine recursively the overlapping intervals of [start, stop] in the interval tree
    void findOverlappingIntervals(int start, int stop, QVector<int>& matches, bool stop_at_first_match=false) const;

    void subtractTree(const IntervalTreeData& other, QVector<Interval>& remaining_intervals) const;

    void allIntervals(QVector<Interval>& intervals) const;


protected:
    /// The interval container
    const T& container_;
    /// The indices of the interval tree node at the certain position
    QVector<int> intervals_;
    /// Pointer to the left sub interval tree
    QScopedPointer<IntervalTreeData> left_;
    /// Pointer to the right sub interval tree
    QScopedPointer<IntervalTreeData> right_;
    /// The center position at this node in the tree, i.e. all intervals which are located left of this position
    /// are stored in the left subtree and intervals which are located right to this position are stored in the right
    /// subtree. However, the intervals overlapping with the center position are stored in intervals_.
    int center_;
    int size_;

};

/// Represents an balanced (using the central point) Interval tree data structure.
/// (Note: It is build upon a set of intervals, which cannot be changed after its initialization.)
template <class T>
class CPPCORESHARED_EXPORT IntervalTree
{
public:
    IntervalTree();
    IntervalTree(const T& container,
                 std::list<int> &indices,
                 int leftextent = 0,
                 int rightextent = 0);

    /// Copy constructor supports implicit sharing of the underlying shared data object IntervalTreeData
    IntervalTree(const IntervalTree& other);

    /// Returns true if the tree is empty
    bool isEmpty() { return !d_; }

    /// Determine the overlapping intervals of [start, stop] in the interval tree. If stop_at_first_match is true,
    /// only the first overlapping interval is returned
    void overlappingIntervals(int start, int stop, QVector<int>& matches, bool stop_at_first_match=false) const;

    void subtractTree(const IntervalTree& other,QVector<Interval>& remaining_intervals) const;

    void allIntervals(QVector<Interval>& intervals) const;


private:
    /// Shared pointer to the actual tree to support implicit sharing
    QSharedDataPointer<IntervalTreeData<T> > d_;

    /// Number intervals in tree
    int size_;
};




template <class T>
IntervalTreeData<T>::IntervalTreeData()
{
    QTextStream outstream(stdout);
    outstream << "IntervalTreeData::IntervalTreeData default constructor " << endl;
}


template <class T>
IntervalTreeData<T>::IntervalTreeData(const T& container,
                                      std::list<int>& indices,
                                      std::list<int>::iterator indices_start,
                                      std::list<int>::iterator indices_end,
                                      int leftextent,
                                      int rightextent)
    : container_(container),
      center_(0)
{
//    QTime timer;
//    QTextStream outstream(stdout);

    int leftp = 0;
    int rightp = 0;
    int centerp = 0;

    if (leftextent || rightextent)
    {
        leftp = leftextent;
        rightp = rightextent;
    }
    else
    {
        // timer.restart();
        leftp = container_[*indices_start].start();
        rightp = container_[*(std::max_element(indices_start, indices_end, MaxEndPositionContainer<T>(container_)))].end();
        // outstream <<"determine max end element" + Helper::elapsedTime(timer) << endl;
    }

    // determine the center position (unfortunately list does not support direct access to the elements by position
    // timer.restart();
    int number_intervals = std::distance(indices_start, indices_end);
    int center_index = number_intervals/2;
    std::list<int>::iterator it = indices_start;
    while (center_index > 0)
    {
        ++it;
        --center_index;
    }
    centerp = container_[*it].start();
    center_ = centerp;
    //outstream <<"find center position " + Helper::elapsedTime(timer) << endl;


    // iterate through the defined range of the list
    it = indices_start;
    // iterators, which define the range of intervals left to the center start position
    std::list<int>::iterator left_start = indices_start;
    std::list<int>::iterator left_end = indices_start;
    // iterators, which define the range of intervals right to the center start position
    std::list<int>::iterator right_start = indices_end;
    std::list<int>::iterator right_end = indices_end;
    // stop when either we've seen all intervals or if we have already found the first interval which is right of
    // the center start position
    // timer.restart();
    while ((it != indices_end) &&  (right_start == indices_end))
    {
        int index =*it;
        if (container_[index].end() < center_)
        {
            left_end=it;
            ++left_end;
        }
        else
        {
            if (container_[index].start() > center_)
            {
                right_start = it;
            }
            else
            {
                // assign the interval to the current node of the tree, since it overlaps the center start position
                // outstream << " append Interval " << endl;
                intervals_.append(index);
                //outstream << " append Interval done " << endl;
                bool removed_left_start = (it == left_start);
                bool removed_left_end = (it == left_end);
                // and remove it from the list
                it=indices.erase(it);
                // if the first element was removed, we need to redefine the iterators of the left range
                if (removed_left_start)
                {
                    left_start=it;
                }
                if (removed_left_end)
                {
                    left_end=it;
                }
                // don't increment the iterator again, since after the removal of "it" the iterator
                // points to the next element
                continue;
            }
        }
        ++it;
    }
//    if (intervals_.count()>1)
//    {

//        outstream << "number intervals at node " << intervals_.count() << endl;
//    }

    //outstream <<"categorize intervals " + Helper::elapsedTime(timer) << endl;

    if (left_start != left_end)
    {
        //timer.restart();
        left_.reset(new IntervalTreeData(container, indices, left_start, left_end, leftp, centerp));
        // outstream << "recursive left sub tree" +  Helper::elapsedTime(timer) << endl;
    }
    if (right_start != right_end)
    {
        //timer.restart();
        right_.reset(new IntervalTreeData(container, indices, right_start, right_end, centerp, rightp));
        // outstream << "recursive left sub tree" +  Helper::elapsedTime(timer) << endl;
    }

}


template <class T>
IntervalTreeData<T>::IntervalTreeData(const IntervalTreeData &other)
    : QSharedData(other),
      intervals_(other.intervals_),
      center_(other.center_)
{
//    QTextStream outstream(stdout);
//    outstream << "IntervalTreeData::IntervalTreeData copy constructor " << endl;

    if (!other.left_.isNull())
    {
        left_.reset(new IntervalTreeData(*other.left_));
    }
    if (!other.right_.isNull())
    {
        right_.reset(new IntervalTreeData(*other.right_));
    }
}

template <class T>
void IntervalTreeData<T>::findOverlappingIntervals(int start, int stop, QVector<int>& matches, bool stop_at_first_match) const
{
    if (!intervals_.empty() && ! (stop < container_[intervals_.front()].start()))
    {
        foreach (const int& index, intervals_)
        {
            if (container_[index].end() >= start && container_[index].start() <= stop)
            {
                matches.append(index);
                if (stop_at_first_match)
                {
                    return;
                }
            }
        }
    }


    if (!left_.isNull() && start <= center_)
    {
        left_->findOverlappingIntervals(start, stop, matches, stop_at_first_match);
    }

    if (!right_.isNull() && stop >= center_)
    {
        right_->findOverlappingIntervals(start, stop, matches, stop_at_first_match);
    }
}

template <class T>
void IntervalTreeData<T>::subtractTree(const IntervalTreeData& other, QVector<Interval>& remaining_intervals) const
{
    const T& container_other = other.container_;
    if (!intervals_.empty())
    {
        foreach (const int& index, intervals_)
        {

            QVector<int> overlapping_intervals;
            int start_this = container_[index].start();
            int stop_this = container_[index].end();
            other.findOverlappingIntervals(start_this, stop_this, overlapping_intervals);
            std::sort(overlapping_intervals.begin(),overlapping_intervals.end(),MinStartPositionContainer<T>(other.container_));

            if (!overlapping_intervals.empty())
            {
                int start_other=container_other[overlapping_intervals[0]].start();
                int stop_other=container_other[overlapping_intervals[0]].end();
                int i=1;

                // proceed until either the whole current_merged interval is subtracted or all overlapping intervals are processed
                while ((start_this < stop_this) && (i < overlapping_intervals.count()))
                {
                    int index_other = overlapping_intervals[i];
                    // go ahead if the matching interval of other is covered by the current interval
                    if (container_other[index_other].end() <= stop_other)
                    {
                        ++i;
                        continue;
                    }
                    // merge the current and the next interval if they overlap
                    if (container_other[index_other].start() <= stop_other)
                    {
                        stop_other = container_other[index_other].end();
                        ++i;
                        continue;
                    }

                    // left part is remaining
                    if (start_other > start_this)
                    {
                        //create new region (left part)
                        remaining_intervals.append(Interval(start_this,start_other-1,-1));
                    }
                    start_this=stop_other;
                    start_other=container_other[index_other].start();
                    stop_other=container_other[index_other].end();
                    ++i;
                }
                // left part is remaining
                if (start_other > start_this)
                {
                    //create new region (left part)
                    remaining_intervals.append(Interval(start_this,start_other-1,-1));
                }
                // create new region for the remaining right part of the interval
                if (stop_other < stop_this)
                {
                    remaining_intervals.append(Interval(stop_other+1,stop_this,-1));
                }

            }
            // if no overlapping intervals exisit in other nothing is subtracted
            else
            {
                remaining_intervals.append(Interval(container_[index].start(), container_[index].end(), index));
            }
        }
    }

    if (!left_.isNull())
    {
        left_->subtractTree(other, remaining_intervals);
    }

    if (!right_.isNull())
    {
        right_->subtractTree(other, remaining_intervals);
    }


}

template <class T>
void IntervalTreeData<T>::allIntervals(QVector<Interval>& intervals) const
{
    if (left_.isNull())
    {
        foreach(const Interval& interval, intervals_)
        {
            intervals.append(interval);
        }
        if (!right_.isNull())
        {
            right_->allIntervals(intervals);
        }
    }
    else
    {
        left_->allIntervals(intervals);
    }

}


template <class T>
IntervalTree<T>::IntervalTree() : d_(), size_(0)
{
//    QTextStream outstream(stdout);
//    outstream << "IntervalTree default constructor"  << endl;
}


template <class T>
IntervalTree<T>::IntervalTree(const T& container,
                              std::list<int>& indices,
                              int leftextent,
                              int rightextent)
    : size_(indices.size())
{
    QTextStream outstream(stdout);
    QTime timer;
    timer.start();
    indices.sort(MinStartPositionContainer<T>(container));
   //outstream << "sort all intervals " +  Helper::elapsedTime(timer) << endl;
    timer.restart();
    d_ = new IntervalTreeData<T>(container, indices, indices.begin(), indices.end(), leftextent, rightextent);
    //outstream << "build tree end" +  Helper::elapsedTime(timer) << endl;
}


template <class T>
IntervalTree<T>::IntervalTree(const IntervalTree<T>& other) : d_(other.d_), size_(other.size_)
{
//    QTextStream outstream(stdout);
//    outstream << "IntervalTree copy constructor"  << endl;
}

// TODO use vector as reference parameter in function
template <class T>
void IntervalTree<T>::overlappingIntervals(int start, int stop, QVector<int>& matches, bool stop_at_first_match) const
{
    matches.clear();
    d_->findOverlappingIntervals(start,stop,matches,stop_at_first_match);
}

template <class T>
void IntervalTree<T>::subtractTree(const IntervalTree& other, QVector<Interval>& remaining_intervals) const
{
    remaining_intervals.clear();
    remaining_intervals.reserve(size_);
    d_->subtractTree(*(other.d_), remaining_intervals);
}

template <class T>
void IntervalTree<T>::allIntervals(QVector<Interval>& intervals) const
{
    intervals.clear();
    intervals.reserve(size_);

    d_->allIntervals(intervals);

}





#endif
