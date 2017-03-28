#include <IntervalTree.h>
#include <algorithm>
#include <QSharedDataPointer>



Interval::Interval()
    : start_(-1)
    , end_(-1)
    , value_(-1)
{
//    QTextStream outstream(stdout);
//    outstream << "Interval::Interval default constructor " << endl;
}

Interval::Interval(int s, int e, int v)
    : start_(s)
    , end_(e)
    , value_(v)
{
//    QTextStream outstream(stdout);
//    outstream << "Interval::Interval constructor " << endl;
}

Interval::Interval(const Interval& other)
    : start_(other.start_)
    , end_(other.end_)
    , value_(other.value_)
{
//    QTextStream outstream(stdout);
//    outstream << "Interval::Interval copy constructor " << endl;
}



Interval::Interval(const QVector<Interval>& intervals)
{
    if (!intervals.empty())
    {
        int i_start = intervals[0].start();
        int i_stop = intervals[0].end();
        int i = 1;
        while (i<intervals.size())
        {
            if ((intervals[i].start() < i_start) && (intervals[i].end() >= i_start))
            {
                i_start = intervals[i].start();

                if (intervals[i].end() > i_stop)
                {
                    i_stop = intervals[i].end();
                }
            }

            if ((i_start < intervals[i].start()) && (i_stop >= intervals[i].start()))
            {
                if (intervals[i].end() > i_stop)
                {
                    i_stop = intervals[i].end();
                }
            }
            ++i;

        }
        start_=i_start;
        end_ = i_stop;
    }
    else
    {
        start_=-1;
        end_ = -1;
    }
    value_ = -1;
}





