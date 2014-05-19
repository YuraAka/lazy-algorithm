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

template <typename _Range>
functional_range<_Range> wrap_functional(const _Range& range);
/// end of fwd

template <typename _Iterator, typename _Predicate>
class filtering_iterator_t
    : public std::iterator<
        std::input_iterator_tag,
        typename std::add_const<typename std::iterator_traits<_Iterator>::value_type>::type>
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

    bool operator!=(const filtering_iterator_t& other) const
    {
        return begin_ != other.begin_;
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

template <typename _Iterator>
class limiting_iterator_t
    : public std::iterator<
        std::input_iterator_tag,
        typename std::add_const<typename std::iterator_traits<_Iterator>::value_type>::type>
{
public:
    limiting_iterator_t(_Iterator begin, _Iterator end, std::size_t max)
        : begin_(max ? begin : end)
        , end_(end)
        , max_(max)
    {
    }

    public:
    typename limiting_iterator_t::reference operator*() const
    {
        assert(begin_ != end_);
        return *begin_;
    }

    typename limiting_iterator_t::pointer operator->() const
    {
        assert(begin_ != end_);
        return begin_;
    }

    limiting_iterator_t& operator++()
    {
        if (begin_ == end_)
        {
            return *this;
        }

        ++begin_;
        --max_;
        if (!max_)
        {
            begin_ = end_;
        }

        return *this;
    }

    bool operator!=(const limiting_iterator_t& other) const
    {
        return begin_ != other.begin_;
    }

private:
    _Iterator begin_;
    const _Iterator end_;
    std::size_t max_;
};

template <typename _Iterator>
limiting_iterator_t<_Iterator> make_limit_iterator(_Iterator begin, _Iterator end, std::size_t max)
{
    return limiting_iterator_t<_Iterator>(begin, end, max);
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
        auto begin = make_filter_iterator(range_.begin(), range_.end(), pred);
        auto end = make_filter_iterator(range_.end(), range_.end(), pred);
        return wrap_functional(make_range(begin, end));
    }

    template <typename _Value, typename _Transformer>
    functional_range<
        range_t<transforming_iterator_t<iterator, _Transformer, _Value> >
    >
    transform(_Transformer transformer) const
    {
        auto begin = make_transforming_iterator<_Value>(range_.begin(), range_.end(), transformer);
        auto end = make_transforming_iterator<_Value>(range_.end(), range_.end(), transformer);

        return wrap_functional(make_range(begin, end));
    }

    template <typename _OutputIterator>
    void copy(_OutputIterator out) const
    {
        std::copy(range_.begin(), range_.end(), out);
    }

    functional_range<
        range_t<limiting_iterator_t<iterator> >
    > limit(std::size_t max) const
    {
        auto begin = make_limit_iterator(range_.begin(), range_.end(), max);
        auto end = make_limit_iterator(range_.end(), range_.end(), max);
        return wrap_functional(make_range(begin, end));
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

template <typename _Range>
functional_range<_Range> wrap_functional(const _Range& range)
{
    return functional_range<_Range>(range);
}

template <typename _Iterator>
range_t<_Iterator> make_range(_Iterator begin, _Iterator end)
{
    return range_t<_Iterator>(begin, end);
}

template <typename _Element, std::size_t size>
functional_range<range_t<_Element*> > enumerate(_Element (&array)[size])
{
    return wrap_functional((make_range(array, array + size)));
}

template <typename _Container>
functional_range<range_t<typename _Container::const_iterator> > enumerate(const _Container& container)
{
    return wrap_functional(make_range(container.begin(), container.end()));
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

void print_char(char c)
{
    std::cout << "'" << c << "' ";
}

std::string tostr(int value)
{
    return std::to_string(value);
}

int toint(char value)
{
    return value;
}

struct add
{
    add(int delta)
        : Delta(delta)
    {
    }

    int operator()(int val) const
    {
        return val + Delta;
    }

    int Delta;
};

struct more_than
{
    more_than(int bound)
        : Bound(bound)
    {
    }

    bool operator()(int val) const
    {
        return val > Bound;
    }

    int Bound;
};

void array_test()
{
    int nums[] = {1,2,3,4,5,6};

    std::cout << "for_each: ";
    enumerate(nums).for_each(&print);
    std::cout << std::endl;

    std::cout << "accumulate: ";
    std::cout << enumerate(nums).accumulate() << std::endl;

    std::cout << "filter (odd): ";
    enumerate(nums).filter(&is_odd).for_each(&print);
    std::cout << std::endl;
    std::cout << "filter (> 2): ";
    enumerate(nums).filter(more_than(2)).for_each(&print);
    std::cout << std::endl;

    std::cout << "transform (to string): ";
    enumerate(nums).transform<std::string>(&tostr).for_each(&print_str);
    std::cout << std::endl;
    std::cout << "transform (+3): ";
    enumerate(nums).transform<int>(add(3)).for_each(&print);
    std::cout << std::endl;

    std::cout << "copy: ";
    std::vector<int> clone;
    enumerate(nums).copy(std::back_inserter(clone));
    std::for_each(clone.begin(), clone.end(), &print);
    std::cout << std::endl;

    std::cout << "limit: ";
    enumerate(nums).limit(3).for_each(&print);
    std::cout << std::endl;
}

void vector_test()
{
    std::vector<char> symbols = {'a', 'b', 'c', 'd', 'e'};

    std::cout << "for_each: ";
    enumerate(symbols).for_each(&print_char);
    std::cout << std::endl;

    std::cout << "filter (> b): ";
    enumerate(symbols).filter(more_than('b')).for_each(&print_char);
    std::cout << std::endl;

    std::cout << "transform (to int): ";
    enumerate(symbols).transform<int>(&toint).for_each(&print);
    std::cout << std::endl;
}

int main(int argc, const char * argv[])
{
    std::cout << "Array tests:" << std::endl;
    array_test();
    std::cout << std::endl;

    std::cout << "Vector tests:" << std::endl;
    vector_test();
    std::cout << std::endl;

    return 0;
}

