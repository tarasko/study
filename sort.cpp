#include <iostream>
#include <algorithm>
#include <type_traits>

using namespace std;

namespace std {
    
template<typename Iterator>
Iterator begin(const pair<Iterator, Iterator>& range)
{
    return range.first;
}

template<typename Iterator>
Iterator end(const pair<Iterator, Iterator>& range)
{
    return range.second;
}

}

namespace {

// Use class instead of template function, that allow to pass it as
// template parameter
struct quick_sort
{
    template<typename Range>
    static void apply(Range&& rng)
    {
        typedef typename remove_reference<decltype(begin(rng))>::type iterator_type;
        typedef typename remove_reference<decltype(*begin(rng))>::type value_type;

        size_t sz = distance(begin(rng), end(rng));

        if (sz < 2)
            return;

        value_type median = (*begin(rng) + *end(rng))/2; // take some random value
        
        iterator_type cur_left = begin(rng);
        iterator_type cur_right = end(rng) - 1;

        bool cont;
        do
        {
            while (*cur_left < median) ++cur_left;
            while (*cur_right > median) --cur_right;

            if (cont = (cur_left < cur_right))
            {
                iter_swap(cur_left, cur_right);
                ++cur_left;
                --cur_right;
            }
        } while (cont);

        apply(make_pair(begin(rng), cur_left));
        apply(make_pair(cur_left, end(rng)));
    }
};
    
template<typename Range>
void dump_range(Range&& rng, const char *name)
{
    typedef decltype(*begin(rng)) value_type;
    cout << name << ": ";
    for_each(begin(rng), end(rng), [](value_type v) { cout << v << ' '; });
    cout << endl;
}

template<typename SortFunc>
void test_sort(const char *name)
{
    int arr[] = { 123, 12, 5, 67, 2, 645, 7, 43, 65, 423, 6, 77, 66, 55, 477 };
    dump_range(arr, "Initial");
    SortFunc::apply(arr);
    dump_range(arr, name);
}

}

int main(int argc, char* argv[])
{   
    test_sort<quick_sort>("quick_sort");
    return 0;
}
