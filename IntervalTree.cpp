#include <IntervalTree.h>
#include <algorithm>
#include <QSharedDataPointer>


Interval::Interval()
    : start(-1)
    , end(-1)
    , value(-1)
{

}

Interval::Interval(int s, int e, int v)
    : start(s)
    , end(e)
    , value(v)
{

}



Interval::Interval(const QVector<Interval>& intervals)
{
    if (!intervals.empty())
    {
        int i_start = intervals[0].start;
        int i_stop = intervals[0].end;
        int i = 1;
        while (i<intervals.size())
        {
            if ((intervals[i].start < i_start) && (intervals[i].end >= i_start))
            {
                i_start = intervals[i].start;

                if (intervals[i].end > i_stop)
                {
                    i_stop = intervals[i].end;
                }
            }

            if ((i_start < intervals[i].start) && (i_stop >= intervals[i].start))
            {
                if (intervals[i].end > i_stop)
                {
                    i_stop = intervals[i].end;
                }
            }
            ++i;

        }
        start=i_start;
        end = i_stop;
    }
    else
    {
        start=-1;
        end = -1;
    }
    value = -1;
}



IntervalTreeData::IntervalTreeData()
{

}


IntervalTreeData::IntervalTreeData(QVector< Interval>& ivals,
                                   int depth,
                                   int leftextent,
                                   int rightextent,
                                   int maxbucket)
    : center_(0),
      size_(ivals.size())
{
    --depth;
    if (depth == 0 || (ivals.size() < maxbucket))
    {
        // if either the maximum depth or the maximum number of intervals stored per node are reached, the intervals are only sorted by start position
        std::sort(ivals.begin(), ivals.end(),startPositionComparator);
        intervals_=ivals;
    }
    else
    {
        if (leftextent == 0 && rightextent == 0)
        {
            // before we build the interval tree, all intervals need to be sorted by their start positions
            std::sort(ivals.begin(), ivals.end(),startPositionComparator);
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
            leftp = ivals.front().start;
            QVector<int> stops;
            stops.resize(ivals.size());
            std::transform(ivals.begin(), ivals.end(), stops.begin(), endPosition);
            rightp = *std::max_element(stops.begin(), stops.end());
        }

        centerp = ivals.at(ivals.size() / 2).start;
        center_ = centerp;

        QVector< Interval> lefts;
        QVector< Interval> rights;

        foreach (const Interval& i, ivals)
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
            left_.reset(new IntervalTreeData(lefts, depth, leftp, centerp));
        }
        if (!rights.empty())
        {
            right_.reset(new IntervalTreeData(rights, depth, centerp, rightp));
        }
    }
}


IntervalTreeData::IntervalTreeData(const IntervalTreeData &other)
    : QSharedData(other),
      intervals_(other.intervals_),
      center_(other.center_)
{
    if (!other.left_.isNull())
    {
        left_.reset(new IntervalTreeData(*other.left_));
    }
    if (!other.right_.isNull())
    {
        right_.reset(new IntervalTreeData(*other.right_));
    }
}


void IntervalTreeData::findOverlappingIntervals(int start, int stop, QVector<Interval>& matches, bool stop_at_first_match) const
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

void IntervalTreeData::subtractTree(const IntervalTreeData& other, QVector<Interval>& remaining_intervals) const
{
    if (!intervals_.empty())
    {
        foreach (const Interval& interval, intervals_)
        {

            QVector<Interval> overlapping_intervals;
            int start_this = interval.start;
            int stop_this = interval.end;
            other.findOverlappingIntervals(start_this, stop_this, overlapping_intervals);
            std::sort(overlapping_intervals.begin(),overlapping_intervals.end(),startPositionComparator);

            if (!overlapping_intervals.empty())
            {
                int start_other=overlapping_intervals[0].start;
                int stop_other=overlapping_intervals[0].end;
                int i=1;

                // proceed until either the whole current_merged interval is subtracted or all overlapping intervals are processed
                while ((start_this < stop_this) && (i < overlapping_intervals.count()))
                {
                    // go ahead if the matching interval of other is covered by the current interval
                    if (overlapping_intervals[i].end <= stop_other)
                    {
                        ++i;
                        continue;
                    }
                    // merge the current and the next interval if they overlap
                    if (overlapping_intervals[i].start <= stop_other)
                    {
                        ++i;
                        stop_other = overlapping_intervals[i].end;
                        continue;
                    }

                    // left part is remaining
                    if (start_other > start_this)
                    {
                        //create new region (left part)
                        remaining_intervals.append(Interval(start_this,start_other-1));
                    }
                    start_this=stop_other;
                    start_other=overlapping_intervals[i].start;
                    stop_other=overlapping_intervals[i].end;
                    ++i;
                }
                // left part is remaining
                if (start_other > start_this)
                {
                    //create new region (left part)
                    remaining_intervals.append(Interval(start_this,start_other-1));
                }
                // create new region for the remaining right part of the interval
                if (stop_other < stop_this)
                {
                    remaining_intervals.append(Interval(stop_other+1,stop_this));
                }

            }
            // if no overlapping intervals exisit in other nothing is subtracted
            else
            {
                remaining_intervals.append(interval);
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

void IntervalTreeData::allIntervals(QVector<Interval>& intervals) const
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


IntervalTree::IntervalTree() : d_(), size_(0)
{
}


IntervalTree::IntervalTree(QVector< Interval>& ivals,
                           int depth,
                           int leftextent,
                           int rightextent,
                           int maxbucket)
    : size_(ivals.size())
{
    d_ = new IntervalTreeData(ivals, depth, leftextent, rightextent, maxbucket);
}


IntervalTree::IntervalTree(const IntervalTree& other) : d_(other.d_), size_(other.size_)
{

}

// TODO use vector as reference parameter in function
void IntervalTree::overlappingIntervals(int start, int stop, QVector<Interval>& matches, bool stop_at_first_match) const
{
    matches.clear();
    d_->findOverlappingIntervals(start,stop,matches,stop_at_first_match);
}

void IntervalTree::subtractTree(const IntervalTree& other, QVector<Interval>& remaining_intervals) const
{
    remaining_intervals.clear();
    d_->subtractTree(*(other.d_), remaining_intervals);
}

void IntervalTree::allIntervals(QVector<Interval>& intervals) const
{
    intervals.clear();
    intervals.reserve(size_);

    d_->allIntervals(intervals);

}


