#include <IntervalTree.h>
#include <QVector>
#include <QPair>
#include <QScopedPointer>
#include <algorithm>

Interval::Interval()
    : start(-1)
    , end(-1)
    , value(-1)
{}

Interval::Interval(int s, int e, int v)
    : start(s)
    , end(e)
    , value(v)
{}

IntervalTree::IntervalTree()
    : left_(Q_NULLPTR)
    , right_(Q_NULLPTR)
    , center_(0)
{ }


IntervalTree::IntervalTree(const QVector< Interval>& ivals,
                           int depth,
                           int leftextent,
                           int rightextent,
                           int maxbucket)
    : left_(Q_NULLPTR),
      right_(Q_NULLPTR),
      center_(0)

{
    --depth;
    if (depth == 0 || (ivals.size() < maxbucket))
    {
        // if either the maximum depth or the maximum number of intervals stored per node are reached, the intervals are only sorted by start position
        intervals_ = ivals;
        std::sort(intervals_.begin(), intervals_.end(), startPositionComparator);
    }
    else
    {
        QVector<Interval> intervals = ivals;
        if (leftextent == 0 && rightextent == 0)
        {
            // before we build the interval tree, all intervals need to be sorted by their start positions
            std::sort(intervals.begin(), intervals.end(), startPositionComparator);
        }

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
            leftp = intervals.front().start;
            QVector<int> stops;
            stops.resize(intervals.size());
            std::transform(intervals.begin(), intervals.end(), stops.begin(), endPosition);
            rightp = *std::max_element(stops.begin(), stops.end());
        }

        centerp = intervals.at(intervals.size() / 2).start;
        center_ = centerp;

        QVector< Interval> lefts;
        QVector< Interval> rights;

        foreach (const Interval& i, intervals)
        {
            if (i.end < center_)
            {
                lefts.append(i);
            }
            else
            {
                if (i.start > center_)
                {
                    rights.append(i);
                }
                else
                {
                    intervals_.append(i);
                }
            }
        }

        if (!lefts.empty())
        {
            left_.reset(new IntervalTree(lefts, depth, leftp, centerp));
        }
        if (!rights.empty())
        {
            right_.reset(new IntervalTree(rights, depth, centerp, rightp));
        }
    }
}


IntervalTree::IntervalTree(const IntervalTree &other)
    : intervals_(other.intervals_),
      left_(Q_NULLPTR),
      right_(Q_NULLPTR),
      center_(other.center_)
{
    if (!other.left_.isNull())
    {
        left_.reset(new IntervalTree(*other.left_));
    }
    if (!other.right_.isNull())
    {
        right_.reset(new IntervalTree(*other.right_));
    }
}



IntervalTree& IntervalTree::operator=(const IntervalTree& other)
{
    center_ = other.center_;
    intervals_ = other.intervals_;
    if (!other.left_.isNull())
    {
        left_.reset(new IntervalTree(*other.left_));
    }
    else
    {
        left_.reset();
    }
    if (!other.right_.isNull())
    {
        right_.reset(new IntervalTree(*other.right_));
    }
    else
    {
        right_.reset();
    }
    return *this;
}


void IntervalTree::findOverlappingIntervals(int start, int stop, QVector<Interval>& matches, bool stop_at_first_match) const
{
    if (!intervals_.empty() && ! (stop < intervals_.front().start))
    {
        foreach (const Interval& i, intervals_)
        {
            if (i.end >= start && i.start <= stop)
            {
                matches.append(i);
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


QVector<Interval> IntervalTree::overlappingIntervals(int start, int stop, bool stop_at_first_match) const
{
    QVector<Interval> matches;
    findOverlappingIntervals(start,stop,matches,stop_at_first_match);

    return matches;
}


