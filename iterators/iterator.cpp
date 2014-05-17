#include <iostream>
#include <vector>
#include <iterator>
#include <numeric>

template <typename _Range>
class range_algorithm_t
{
public:
    typedef typename _Range::value_type value_type;

public:
    explicit range_algorithm_t(const _Range& range)
        : range_(range)
    {
    }

public:
    value_type accumulate(const value_type& initial = value_type()) const
    {
        return std::accumulate(range_.begin(), range_.end(), initial);
    }

private:
    const _Range range_;
};

template <typename _Iterator>
class range_t
{
public:
    typedef typename std::iterator_traits<_Iterator>::value_type value_type;

public:
    range_t(_Iterator begin, _Iterator end)
        : begin_(begin)
        , end_(end)
    {
    }

    _Iterator begin() const
    {
        return begin_;
    }

    _Iterator end() const
    {
        return end_;
    }

private:
    const _Iterator begin_;
    const _Iterator end_;
};

template <typename _Range>
class functional_range : public range_algorithm_t<_Range>
{
public:
    explicit functional_range(const _Range& range)
        : range_algorithm_t<_Range>(range)
    {
    }
};

template <typename _Iterator>
range_t<_Iterator> make_range(_Iterator begin, _Iterator end)
{
    return range_t<_Iterator>(begin, end);
}

template <typename _Element, std::size_t size>
functional_range<range_t<_Element*> > enumerate(_Element (&nums)[size])
{
    return functional_range<range_t<_Element*> >(make_range(nums, nums + size));
}

int main(int argc, const char * argv[])
{
    int nums[] = {1,2,3,4};
    std::cout << "1+2+3+4=" << enumerate(nums).accumulate() << std::endl;
    return 0;
}

