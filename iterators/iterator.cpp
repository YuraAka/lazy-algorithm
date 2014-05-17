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
        if (begin_ != end_)
        {
            ++begin_;
            skip_bad();
        }

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

template <typename _Iterator, typename _Transformer, typename _Value>
class transforming_iterator_t
    : public std::iterator<std::input_iterator_tag, typename std::add_const<_Value>::type>
{
public:
    transforming_iterator_t(_Iterator begin, _Iterator end, _Transformer transformer)
        : begin_(begin)
        , end_(end)
        , transformer_(transformer)
    {
        apply_transformation();
    }

public:
    typename transforming_iterator_t::reference operator*() const
    {
        assert(begin_ != end_);
        return value_;
    }

    typename transforming_iterator_t::pointer operator->() const
    {
        assert(begin_ != end_);
        return &value_;
    }

    transforming_iterator_t& operator++()
    {
        if (begin_ != end_)
        {
            ++begin_;
            apply_transformation();
        }

        return *this;
    }

    bool operator==(const transforming_iterator_t& other) const
    {
        return begin_ == other.begin_;
    }

    bool operator!=(const transforming_iterator_t& other) const
    {
        return !(*this == other);
    }

private:
    void apply_transformation()
    {
        if (begin_ != end_)
        {
            value_ = transformer_(*begin_);
        }
    }

private:
    _Iterator begin_;
    const _Iterator end_;
    const _Transformer transformer_;
    /// todo make optional
    typename std::remove_cv<_Value>::type value_;
};

template <typename _Value, typename _Iterator, typename _Transformer>
transforming_iterator_t<_Iterator, _Transformer, _Value>
make_transforming_iterator(_Iterator begin, _Iterator end, _Transformer transformer)
{
    return transforming_iterator_t<_Iterator, _Transformer, _Value>(begin, end, transformer);
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
        typedef functional_range<range_t<filtering_iterator_t<iterator, _Predicate> > > result_type;

        auto begin = make_filter_iterator(range_.begin(), range_.end(), pred);
        auto end = make_filter_iterator(range_.end(), range_.end(), pred);
        return result_type(make_range(begin, end));
    }

    template <typename _Value, typename _Transformer>
    functional_range<
        range_t<transforming_iterator_t<iterator, _Transformer, _Value> >
    >
    transform(_Transformer transformer) const
    {
        typedef functional_range<range_t<transforming_iterator_t<iterator, _Transformer, _Value> > > result_type;

        auto begin = make_transforming_iterator<_Value>(range_.begin(), range_.end(), transformer);
        auto end = make_transforming_iterator<_Value>(range_.end(), range_.end(), transformer);

        return result_type(make_range(begin, end));
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

void print_str(std::string str)
{
    std::cout << '"' << str << '"' << " ";
}

/// todo make for values
std::string tostr(int value)
{
    return std::to_string(value);
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

    std::cout << "transform: ";
    enumerate(nums).transform<std::string>(&tostr).for_each(&print_str);
    std::cout << std::endl;

    return 0;
}

