//
//  main.cpp
//  iterators
//
//  Created by Юра on 05.05.14.
//  Copyright (c) 2014 yura. All rights reserved.
//

#include <iostream>
#include <vector>
#include <functional>
#include <iterator>

template <typename Container>
class WeakContainerRange
{
public:
    WeakContainerRange(Container& c)
        : C(c)
    {
    }

    typename Container::iterator Begin()
    {
        return C.begin();
    }

    typename Container::iterator End()
    {
        return C.end();
    }
    
    typedef typename Container::iterator It;

private:
    Container& C;
};

template <typename Range, typename Predicate>
class FilteredRange
{
private:
    typedef std::iterator<std::input_iterator_tag, typename Range::It::value_type> IteratorBase;
    class Iterator : public IteratorBase
    {
    public:
        Iterator(Range range, const Predicate& p)
            : Current(range.Begin())
            , End(range.End())
            , Pred(&p)
        {
            SkipBad();
        }
                  
        Iterator(typename Range::It end)
            : Current(end)
            , End(end)
            , Pred(NULL)
        {
        }
        
        typename IteratorBase::reference operator*()
        {
            return *Current;
        }

        typename Range::It operator->()
        {
            return Current;
        }

        Iterator& operator++()
        {
            SkipBad();
            return *this;
        }

        bool operator!=(Iterator it) const
        {
            return !(it->Current == Current && it->End == End && it->Pred == Pred);
        }

                  
    private:
        void SkipBad()
        {
            while (Current != End && !(*Pred)(*Current))
            {
                ++Current;
            }
        }
        
        typename Range::It Current;
        typename Range::It End;
        const Predicate* Pred;
    };
    
public:
    FilteredRange(const Predicate& p, const Range& range)
        : Pred(p)
        , R(range)
    {
    }
    
    Iterator Begin()
    {
        return Iterator(R, Pred);
    }
    
    Iterator End()
    {
        return Iterator(R.End());
    }

    typedef Iterator It;

private:
    const Predicate& Pred;
    Range R;
};

template <typename Container>
WeakContainerRange<Container> Iterate(Container& c)
{
    return WeakContainerRange<Container>(c);
}

template <typename Range, typename Predicate>
FilteredRange<Range, Predicate> Filter(Predicate p, Range r)
{
    return FilteredRange<Range, Predicate>(p, r);
}

template <typename Range>
void Print(Range range)
{
    //for (typename Range::It it = range.Begin(); it != range.End(); ++it)
    //{
    //    std::cout << *it << std::endl;
   // }
   typename Range::It it = range.Begin();
   *it;
   ++it;
   it != range.End();
}

//limit(count, filter(rule, iterate(shops)))
//pair<begin, end> range = filter(shops.begin(), shops.end(), rule)
//range = limit(count, range)

struct MoreThanTwo : public std::unary_function<int, bool>
{
public:
    bool operator()(int arg) const
    {
        return arg > 2;
    }
};

int main(int argc, const char * argv[])
{
    std::vector<int> shops;
    shops.push_back(1);
    shops.push_back(2);
    shops.push_back(3);
    shops.push_back(4);
    // insert code here...
    Print(Filter(MoreThanTwo(), Iterate(shops)));
    std::cout << "Hello, World!\n";
    return 0;
}

