#include <iostream>
#include <vector>
#include <iterator>
#include <numeric>
#include <cassert>

/// forward declaration
template <typename _Range>
class functional_range;

template <typename _Iterator>
class range_t;

template <typename _Iterator>
range_t<_Iterator> make_range(_Iterator begin, _Iterator end);
/// end of fwd

template <typename _Iterator, typename _Predicate>
class filtering_iterator_t
    : public std::iterator<std::input_iterator_tag, typename std::iterator_traits<_Iterator>::value_type>
{
public:
    filtering_iterator_t(_Iterator begin, _Iterator end, _Predicate filter)
        : begin_(begin)
        , end_(end)
        , filter_(filter)
    {
        skip_bad();
    }

public:
    typename filtering_iterator_t::reference operator*() const
    {
        assert(begin_ != end_);
        return *begin_;
    }

    typename filtering_iterator_t::pointer operator->() const
    {
        assert(begin_ != end_);
        return begin_;
    }

    filtering_iterator_t& operator++()
    {
        ++begin_;
        skip_bad();
        return *this;
    }

    bool operator==(const filtering_iterator_t& other) const
    {
        return begin_ == other.begin_;
    }

    bool operator!=(const filtering_iterator_t& other) const
    {
        return !(*this == other);
    }

private:
    void skip_bad()
    {
        while (begin_ != end_ && !filter_(*begin_))
        {
            ++begin_;
        }
    }

private:
    _Iterator begin_;
    const _Iterator end_;
    const _Predicate filter_;
};

template <typename _Iterator, typename _Predicate>
filtering_iterator_t<_Iterator, _Predicate> make_filter_iterator(_Iterator begin, _Iterator end, _Predicate filter)
{
    return filtering_iterator_t<_Iterator, _Predicate>(begin, end, filter);
}

template <typename _Range>
class range_algorithm_t
{
public:
    typedef typename _Range::value_type value_type;
    typedef typename _Range::iterator iterator;

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

    template <typename _Operation>
    void for_each(const _Operation& op) const
    {
        std::for_each(range_.begin(), range_.end(), op);
    }

    template <typename _Predicate>
    functional_range<
        range_t<
            filtering_iterator_t<iterator, _Predicate> > >
    filter(const _Predicate& pred) const
    {
        typedef functional_range<range_t<filtering_iterator_t<iterator, _Predicate> > > filtering_range_type;
        auto begin = make_filter_iterator(range_.begin(), range_.end(), pred);
        auto end = make_filter_iterator(range_.end(), range_.end(), pred);
        return filtering_range_type(make_range(begin, end));
    }

private:
    const _Range range_;
};

template <typename _Iterator>
class range_t
{
public:
    typedef typename std::iterator_traits<_Iterator>::value_type value_type;
    typedef _Iterator iterator;

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

bool is_odd(int value)
{
    return !!(value % 2);
}

void print(int value)
{
    std::cout << value << " ";
}

int main(int argc, const char * argv[])
{
    int nums[] = {1,2,3,4};
    std::cout << "accumulate: ";
    std::cout << enumerate(nums).accumulate() << std::endl;

    std::cout << "for_each: ";
    enumerate(nums).for_each(&print);
    std::cout << std::endl;

    std::cout << "filter: ";
    enumerate(nums).filter(&is_odd).for_each(&print);
    std::cout << std::endl;

    //std::istream_iterator<int> eos;
    //std::istream_iterator<int> in(std::cin);
    //std::cout << "max: " << *std::max_element(in, eos) << std::endl;

    return 0;
}

