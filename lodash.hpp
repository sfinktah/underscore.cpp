﻿#ifndef UNDERSCORE_UNDERSCORE_H_
#define UNDERSCORE_UNDERSCORE_H_
#define UNDERSCORE_BONUS
#define _VECTOR(...) <std::vector<__VA_ARGS__>>
#define _IDENTITY_LAMBDA [](const auto& _) { return _; }

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iterator>
#include <map>
#include <numeric>
#include <utility>
#include <vector>
#include <random>
#include <sstream>
#include <stdexcept>
#include <type_traits>

// SO answers by: https://stackoverflow.com/users/1120273/dietmar-k%C3%BChl (someone not to argue with on stackoverflow)
// https://stackoverflow.com/questions/12662891/passing-a-member-function-as-an-argument-in-c/12662961#12662961
// https://stackoverflow.com/questions/8777603/what-is-the-simplest-way-to-convert-array-to-vector/8778892#8778892

// using namespace sfinktah::functional;

////  not used at present, but a really handy example of dealing with multiple argument passing and rvalue and perfect forwarding
////  (from Scott Meyers, in his "Effective Modern C++")
// template <typename Function, typename... Args>
// inline auto reallyAsync(Function&& f, Args&&... params) {
//    // Maybe we could use it with std::apply - see http://en.cppreference.com/w/cpp/utility/apply
//    // and http://en.cppreference.com/w/cpp/utility/functional/invoke
//    return std::async(std::launch::async, std::forward<Function>(f),
//        std::forward<Args>(params)...);
//}
//

namespace _ {
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

    namespace detail {
        template <typename, template <typename...> typename T, typename... Args>
        struct is_detected : std::false_type {
        };

        template <template <typename...> typename T, typename... Args>
        struct is_detected<std::void_t<T<Args...>>, T, Args...> : std::true_type {
        };
    }  // namespace detail

    template <template <typename...> typename T, typename... Args>
    struct is_detected : detail::is_detected<void, T, Args...> {
    };

    namespace traits {
        template <typename Container>
        using push_back_t = decltype(std::declval<Container&>().push_back(std::declval<typename Container::value_type>()));

        template <typename Container>
        using has_push_back = is_detected<push_back_t, Container>;

        template <typename Container>
        using mapped_type_t = typename Container::mapped_type;

        template <typename Container>
        using has_mapped_type = is_detected<mapped_type_t, Container>;

        template <typename Container>
        using is_object_t = decltype(std::declval<Container&>().is_object());

        template <typename Container>
        using has_is_object = is_detected<is_object_t, Container>;

        // template<typename T> struct has_mapped_type {
        // private:
        //    template<typename U, typename = typename U::mapped_type>
        //    static int detect(U &&);
        //    static void detect(...);
        // public:
        //    static constexpr bool value =
        //        std::is_integral<decltype(detect(std::declval<T>()))>::value;
        //};
    }

    // template<class BasicJsonType, class CompatibleObjectType>
    // struct is_compatible_object_type
    //{
    //    static auto constexpr value = is_compatible_object_type_impl <
    //        conjunction<negation<std::is_same<void, CompatibleObjectType>>,
    //        has_mapped_type<CompatibleObjectType>,
    //        has_key_type<CompatibleObjectType>>::value,
    //        typename BasicJsonType::object_t, CompatibleObjectType >::value;
    //};

    namespace helper {
        template <typename Container>
        typename std::enable_if<traits::has_push_back<Container>::value, void>::type add_to_container(
            Container& container, const typename Container::value_type& value)
        {
            container.push_back(value);
        }

        template <typename Container>
        typename std::enable_if<!traits::has_push_back<Container>::value, void>::type add_to_container(
            Container& container, const typename Container::value_type& value)
        {
            container.insert(value);
        }

        template <typename Container>
        void add_to_container(
            Container& container, const typename Container::value_type& value, const typename Container::iterator position)
        {
            container.insert(position, value);
        }


        template<typename T>
        constexpr const T& clamp(const T& val, const T& lo, const T& hi)
        {
            return 
                (val < lo) ? lo : 
                (val > hi) ? hi : val;
        }


        // https://stackoverflow.com/questions/9044866/how-to-get-the-number-of-arguments-of-stdfunction/9044927#9044927
        template <typename T>
        struct count_arg;

        template <typename R, typename... Args>
        struct count_arg<std::function<R(Args...)>> {
            static const size_t value = sizeof...(Args);
        };

        //#include <type_traits>
        //#include <utility>
        //#include <map>

        // int main()
        //{
        //    std::map<int, int> foo{
        //        { 1, 2 },
        //        { 3, 4 },
        //    };

        //    do_stuff(foo.begin());
        //    return 0;
        //}
    }  // namespace helper

    // Collections

    // each/for_each

    // http://en.cppreference.com/w/cpp/algorithm/for_each
    template <typename Container, typename Function>
    void each(Container& container, Function&& function)
    {
        // This version is required for associative container
        //if constexpr(traits::has_mapped_type<Container>::value) { 
        //    each(tuple_values _VECTOR(typename Container::value_type) (container, std::forward<Function>(function)));
        //}
        //else {
            std::for_each(std::begin(container), std::end(container), std::forward<Function>(function));
        //}

    }

    template <typename Container, typename Function>
    void each_with_distance(const Container& container, Function&& function)
    {
        // The signature of the function should be equivalent to the following :

        //    void fun(const Type &a, const size_t d);
        for (auto i = container.begin(); i != container.end(); ++i) {
            function(*i, std::distance(container.begin(), i));
        }
    }

    template <typename Container, typename Function>
    void each_iter(const Container& container, Function&& function)
    {
        // The signature of the function should be equivalent to the following :

        //    void fun(const Type &a, const size_t d);
        for (auto i = container.begin(); i != container.end(); ++i) {
            function(i);
        }
    }

    // template <class T, class = void>
    // struct is_iterator : std::false_type { };

    // template <class T>
    // struct is_iterator<T, std::void_t<
    //    typename std::iterator_traits<T>::iterator_category
    //    >> : std::true_type { };

    // template <typename T, typename Function>
    // typename std::enable_if<is_iterator<T>::value, void>::type each_key_value(T iterator, Function function)
    //{
    //    for (; iterator != container.end(); ++i) {
    //        auto key = i->first;
    //        auto value = i->second;
    //        function(value, key, container);
    //    }
    //}

    //  The full power of `each`.  Each invocation of iteratee is called
    //  with three arguments: (element, index, list). If list is an object,
    //  iteratee's arguments will be (value, key, list).  (MDN)
    template <typename Container, typename Function>
    void each_key_value(const Container& container, Function&& function)
    {
        for (auto i = container.begin(); i != container.end(); ++i) {
            function(i->second, i->first, container);
        }
    }

    template <typename Iterator, typename Function>
    void each_key_value(Iterator i, Iterator end, Function&& function)
    {
        for (; i != end; ++i) {
            auto key   = i->first;
            auto value = i->second;
            function(value, key);
        }
    }

    //  each - for nlohmann::json associative containers. iteratee has three arguments: (value, key).
    template <typename Container, typename Function>
    void each_json(const Container& container, Function&& function)
    {
        for (auto i = container.begin(); i != container.end(); ++i) {
            //auto key   = i.key();
            //auto value = i.value();
            function(i.value(), i.key());
        }
    }

    //template <typename Container, typename Function>
    //typename std::enable_if<traits::has_is_object<Container>::value, void>::type 
    //json_each(Container& container, Function&& iteratee)
    //{
    //    auto& value = container;
    //    if (value.is_object()) {
    //        for (auto i = std::begin(container); i != std::end(container); i++) {
    //            iteratee(i.value(), i.key());
    //        }
    //    }
    //    // technically if it's just an array, the user could/should have called `each_with_distance`
    //    //else if (value.is_array()) {
    //    //    each_with_distance(container, iteratee);
    //    //        //for (std::size_t i = 0, len = value.size(); i < siz; i++) {
    //    //        //    iteratee(value.m_value.array->operator[](i), json(i));
    //    //        //}
    //    //}
    //    else {
    //        throw std::runtime_error("json type must be ~array~ or object");
    //    }
    //}

    // template <typename Container, typename... Args>
    // void each_json_magic(Container& container, std::function<void(Args...)> function)
    //{
    //    auto iteratee = [](auto value, auto key, auto& container) {};
    //    switch (helper::count_arg<function>::value) {
    //        case 3: iteratee = [&] { function(value, key, container); }; break;
    //        case 2: iteratee = [&] { function(value, key); }; break;
    //        case 1: iteratee = [&] { function(value); }; break;
    //    }

    //    for (auto i = container.begin(); i != container.end(); ++i) {
    //        auto key = i.key();
    //        auto value = i.value();
    //        iteratee(value, key, container);
    //    }
    //}

    template <typename Container, typename Function>
    void for_each(const Container& container, Function&& function)
    {
        each(container, std::forward<Function>(function));
    }

    // map/collect
    template <typename ResultContainer, typename Container, typename Function>
    ResultContainer map(const Container& container, Function&& function)
    {
        ResultContainer result;

        // This version is required for associative container
        //if constexpr(traits::has_mapped_type<Container>::value) { 
        //    return map<ResultContainer>(tuple_values<std::vector<typename Container::value_type>>(container));
        //    //for (const typename Container::value_type& item : container) helper::add_to_container(result, item);
        //}
        //else {
            std::transform(std::begin(container), std::end(container), std::back_inserter(result), std::forward<Function>(function));
        //}

        return result;
    }

    // mapObject - Creates an array of values by running each element in collection thru iteratee.
    // The iteratee is invoked with two arguments: (value, key). -- sfink
    template <typename ResultContainer, typename Container, typename Function>
    ResultContainer mapObject(const Container& container, Function&& function)
    {
        return map(container, [&](auto& key) { return function(container[key], key); });
    }

    // remove (lodash) - Removes all elements from array that predicate returns truthy for and returns an array of the removed
    // elements.
    // Note: Unlike _.filter, this method mutates array. Use _.pull to pull elements from an array by value.
    // Note: for ease of use, this function does not return removed elements, use `removeAndReturn` instead
    template <typename Container, typename Function>
    void remove(Container& container, Function&& function)
    {
        for (auto i = container.begin(); i != container.end();) function(*i) ? i = container.erase(i) : ++i;
    }

    // remove (lodash) - Removes all elements from array that predicate returns truthy for and returns an array of the removed
    // elements.
    // Note: Unlike _.filter, this method mutates array. Use _.pull to pull elements from an array by value.
    template <typename ResultContainer, typename Container, typename Function>
    ResultContainer removeAndReturn(Container& container, Function function)
    {
        ResultContainer result;
        for (auto i = container.begin(); i != container.end();) {
            if (function(*i))
                helper::add_to_container(result, *i), i = container.erase(i);
            else
                ++i;
        }
        return result;
    }

    // pull (lodash) - Removes all given values from array using SameValueZero for equality comparisons.
    // Note: Unlike `without`, this method mutates array. Use _.remove to remove elements from an array by predicate.
    template <typename Container>
    void pull(Container& container, typename Container::value_type const& value)
    {
        for (auto i = container.begin(); i != container.end();) (*i == value) ? i = container.erase(i) : ++i;
    }

    // pullAll (lodash) - This method is like _.pull except that it accepts an array of values to remove.
    // Note: Unlike _.difference, this method mutates array.
    template <typename Container1, typename Container2>
    void pullAll(Container1& container, Container2 const& values)
    {

        // Hmmm.... if it's similar to difference, maybe we could leverage the existing `difference` function...
        // However, that function looks complicated. Lets leverage `contains` instead. It's possibly less
        // efficient that using `difference` but simplicity wins today.
        for (auto i = container.begin(); i != container.end();) contains(values, *i) ? i = container.erase(i) : ++i;
    }

    // filter/select
    template <typename ResultContainer, typename Container, typename Predicate>
    ResultContainer filter(Container& container, Predicate&& predicate)
    {
        ResultContainer result;

        each(container, [&](auto& value) {
            if (predicate(value)) {
                helper::add_to_container(result, value);
            }
        });

        return result;
    }

    // without - Creates an array excluding all given values using SameValueZero for equality comparisons.
    // Note: Unlike `pull`, this method returns a new array.
    template <typename ResultContainer, typename Container>
    ResultContainer without(Container const& container, typename Container::value_type const& value)
    {
        // sorry, you'll have to work out your own checks for C++17
        return filter<ResultContainer>(container, [value](const auto& _) {
            return value != _;
            // return std::not_equal_to<typename Container::value_type>(_, value);
        });
#if 0
        // deprecated in c++11, removed in c++17
        return filter<ResultContainer>(
            container, std::bind2nd(std::not_equal_to<typename Container::value_type>(), value));
#endif
    }

    template <typename ResultContainer, typename Container, typename Function>
    ResultContainer collect(const Container& container, Function&& function)
    {
        return map<ResultContainer>(container, std::forward<Function>(function));
    }

    template <typename ResultContainer, typename Container>
    ResultContainer tuple_keys(const Container& container)
    {
        return get_item<ResultContainer, 0>(container);
    }

    template <typename ResultContainer, typename Container>
    ResultContainer tuple_values(const Container& container)
    {
        return get_item<ResultContainer, 1>(container);
    }


    template <typename ResultContainer, typename Container>
    ResultContainer arrayValues(const Container& container)
    {
        // @brick This fails on associative containers
        return ResultContainer(std::begin(container), std::end(container));

        // ResultContainer result;
        // for (const auto& item : container) helper::add_to_container(result, item);
        // return result;
    }

    template <typename ResultContainer, typename Container>
    ResultContainer objectValues(const Container& container)
    {
        // @brick This fails on associative containers
        return tuple_values<ResultContainer>(container);

        // ResultContainer result;
        // for (const auto& item : container) helper::add_to_container(result, item);
        // return result;
    }


    //template <typename Container>
    //typename Container::value_type sample(const Container& container)
    //{
    //}
    // sfink - values
    template <typename ResultContainer, typename Container>
    ResultContainer values(const Container& container)
    {
        if constexpr(traits::has_mapped_type<Container>::value) { 
            return objectValues<ResultContainer>(container); 
        }
        else {
            return arrayValues<ResultContainer>(container);
        }
    }
    // sfink - needed to process json objects, and any other crap that doesn't fully comply to STL
    template <typename ResultContainer, typename Container>
    ResultContainer values2(const Container& container)
    {
        ResultContainer result;
        for (const typename Container::value_type& item : container) helper::add_to_container(result, item);
        return result;
    }

    // template <class T, class Enable = void>
    // class value_type_from
    //{
    //    typedef T type;
    //};

    // template <class T>
    // class value_type_from<T, typename std::enable_if_has_type<typename T::value_type>::type>
    //{
    //    typedef typename T::value_type type;
    //};
    // typename Container::iterator

    template <typename ResultContainer, std::size_t I, typename Container>
    ResultContainer get_item(const Container& container)
    {
        return map<ResultContainer>(container, [](auto& value) { return std::get<I>(value); });
    }

    // template <typename ResultContainer, typename Container>
    // ResultContainer values(const Container& container)
    //{
    //    return tuple_values<ResultContainer>(container);
    //}

    template <typename ResultContainer, typename Container>
    ResultContainer keys(const Container& container)
    {
        return tuple_keys<ResultContainer>(container);
    }

    // sfink - keys2
    template <typename Container>
    auto keys2(const Container& container)
    {
        return keys<std::vector<typename Container::key_type>>(container);
    }

    // template<class Iterator, typename = typename std::enable_if<is_pair<typename Iterator::value_type>::value, Iterator>::type>
    // decltype(auto) do_stuff(Iterator&& iterator) {

    //    //access of iterator->second ok.
    //}

    // MDN - The slice() method returns a shallow copy of a portion of an array into a
    // new array object selected from begin to end (end not included).
    // The original array will not be modified.
    template <typename ResultContainer, typename Container>
    ResultContainer slice(const Container& container, long long begin = 0, long long end = MAXINT)
    {
        // begin Optional
        //    Zero - based index at which to begin extraction.
        //    A negative index can be used, indicating an offset from the end of the sequence.slice(-2) extracts the last two
        //    elements in the sequence.
        //    If begin is undefined, slice begins from index 0.

        // end Optional
        //    Zero - based index before which to end extraction.slice extracts up to but not including end.
        //    For example, slice(1, 4) extracts the second element through the fourth element(elements indexed 1, 2, and 3).
        //    A negative index can be used, indicating an offset from the end of the sequence.slice(2, -1) extracts the third
        //    element through the second - to - last element in the sequence.
        //    If end is omitted, slice extracts through the end of the sequence(arr.length).
        //    If end is greater than the length of the sequence, slice extracts through the end of the sequence(arr.length).

        ResultContainer result;

        const size_t len = container.size();
        if (len == 0)
            return result;

        if (begin >= 0 && static_cast<size_t>(begin) > (len - 1))
            return result;

        //LOG_DEBUG(__FUNCTION__ ":1. len: %lli, begin: %lli, end: %lli", len, begin, end);
        if (end < 1) end = len + end;
        if (begin < 0) begin = len + begin;

        begin = helper::clamp<long long>(begin, 0, len - 1);
        end   = helper::clamp<long long>(end, begin, len);
        auto count = end - begin;
        //LOG_DEBUG(__FUNCTION__ ":2. len: %lli, begin: %lli, end: %lli, count: %lli", len, begin, end, count);
        // paranoia?
        count = helper::clamp<long long>(count, 0, len - begin);
        //LOG_DEBUG(__FUNCTION__ ":3. len: %lli, begin: %lli, end: %lli, count: %lli", len, begin, end, count);


        if (count > 0)
        for (auto it = std::next(std::begin(container), begin); count; count--, it++) {
            helper::add_to_container(result, *it);
        }


        //long long       _index = 0;
        //for (auto i = container.begin(); i != container.end(); ++i) {
        //    auto index = _index++;
        //    if (index >= end) break;
        //    if (index >= begin) helper::add_to_container(result, *i);
        //}
        return result;
    }

    /// <summary>Mutates the contents of an array by removing existing elements and/or adding new elements</summary>
    /// <param name="container">The container.</param>
    /// <param name="start">Index at which to start changing the array (with origin 0). If greater than the length of the array, actual starting index will be set to the length of the array. If negative, will begin that many elements from the end of the array (with origin -1) and will be set to 0 if absolute value is greater than the length of the array.</param>
    /// <param name="deleteCount">The number of old array elements to remove. 
    /// If deleteCount is omitted, or if its value is larger than array.length - start (that is, if it is greater than the number of elements left in the array, starting at start), then all of the elements from start through the end of the array will be deleted.
    /// If deleteCount is 0 or negative, no elements are removed.In this case, you should specify at least one new element(see below).</param>
    /// <param name="items">The elements to add to the array, beginning at the start index. If you don't specify any elements, splice() will only remove elements from the array.</param>
    /// <returns>An array containing the deleted elements. If only one element is removed, an array of one element is returned. If no elements are removed, an empty array is returned.</returns>
    /// <example><code>
    /// // see: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/splice
    /// </code></example>
    template <typename Container>
    Container splice(Container& container, long long start, long long deleteCount = MAXINT, const Container items = {})
    {
        // begin Optional
        //    Zero - based index at which to begin extraction.
        //    A negative index can be used, indicating an offset from the end of the sequence.slice(-2) extracts the last two
        //    elements in the sequence.
        //    If begin is undefined, slice begins from index 0.

        // end Optional
        //    Zero - based index before which to end extraction.slice extracts up to but not including end.
        //    For example, slice(1, 4) extracts the second element through the fourth element(elements indexed 1, 2, and 3).
        //    A negative index can be used, indicating an offset from the end of the sequence.slice(2, -1) extracts the third
        //    element through the second - to - last element in the sequence.
        //    If end is omitted, slice extracts through the end of the sequence(arr.length).
        //    If end is greater than the length of the sequence, slice extracts through the end of the sequence(arr.length).

        /*
        start
            Index at which to start changing the array (with origin 0). 
            If greater than the length of the array, actual starting index will be set to the length of the array. 
            If negative, will begin that many elements from the end of the array (with origin -1) and will be set to 0 if absolute value is greater than the length of the array.

        deleteCount Optional
            An integer indicating the number of old array elements to remove.
            If deleteCount is omitted, or if its value is larger than `array.length - start` 
                (that is, if it is greater than the number of elements left in the array, starting at start), 
                then all of the elements from start through the end of the array will be deleted.
            If deleteCount is 0 or negative, no elements are removed. In this case, you should specify at least one new element (see below).
        */

        Container result;

        const size_t len = container.size();

        if (deleteCount < 1) deleteCount = 0;
        if (start < 0) start = len + start;

        start = helper::clamp<size_t>(start, 0, len);
        size_t end = start + deleteCount;
        end   = helper::clamp<size_t>(end, start, len);
        deleteCount = end - start;

        if (deleteCount > 0)
        for (auto it = std::next(std::begin(container), start); deleteCount; deleteCount--) {
            helper::add_to_container(result, *it), it = container.erase(it);
        }
        auto pos = std::next(std::begin(container), start);
        container.insert(pos, std::begin(items), std::end(items));
        //each(items, [&container, &pos](const auto& item) {
        //    helper::add_to_container(container, item, pos)
        //});
        //result = slice<Container>(container, start, end);
        //size_t       _index = 0;
        //for (auto i = container.begin(); i != container.end(); /* ++i */) {
        //    auto index = _index++;
        //    if (index >= end) break;
        //    if (index >= start) helper::add_to_container(result, *i);
        //}
        return result;
    }

    template <typename Container, typename Value>
    Container spliceItem(Container& container, long long start, long long deleteCount, const Value& item)
    {
        const size_t len = container.size();
        if (deleteCount < 1) deleteCount = 0;
        if (start < 0) start = len + start;

        start = helper::clamp<size_t>(start, 0, len);
        size_t end = start + deleteCount;
        end   = helper::clamp<size_t>(end, start, len);
        deleteCount = end - start;

        Container result;
        if (deleteCount > 0)
        for (auto it = std::next(std::begin(container), start); deleteCount; deleteCount--) {
            helper::add_to_container(result, *it), it = container.erase(it);
        }
        auto pos = std::next(std::begin(container), start);
        container.emplace(pos, item);
        //each(items, [&container, &pos](const auto& item) {
        //    helper::add_to_container(container, item, pos)
        //});
        //result = slice<Container>(container, start, end);
        //size_t       _index = 0;
        //for (auto i = container.begin(); i != container.end(); /* ++i */) {
        //    auto index = _index++;
        //    if (index >= end) break;
        //    if (index >= start) helper::add_to_container(result, *i);
        //}
        return result;
    }

    /// <summary>`reduce` for sequence containers with 4 argument callback</summary>
    /// <param name="container">The container.</param>
    /// <param name="function">callback(<paramref name="initialValue" />, currentValue, currentIndex, <paramref name="container"
    /// />)</param>
    /// <param name="initialValue">Value to use as the first argument to the first call of the callback.</param>
    /// <returns>The value that results from the reduction.</returns>
    /// <example><code>
    /// std::vector<int> v{ 1, 2, 3 };
    /// count << lodash::reduceArray(v, [](auto accumulator, auto currentValue, auto currentIndex, auto container) {
    ///     return accumulator + "Index: "s + std::to_string(currentIndex) + " = "s + std::to_string(currentValue) + '\n';
    /// }, std::string{})
    /// </code></example>
    /// TODO Implement initialValue as optional: "[Optional] Value to use as the first argument to the first call of the callback.
    /// If no initial value is supplied, the first element in the array will be used. Calling reduce on an empty array without an
    /// initial value is an error."
    template <typename Container, typename Function, typename Memo>
    Memo reduceArray(const Container& container, Function function, Memo initialValue)
    {
        each_with_distance(container, [&](const typename Container::value_type& value, const size_t index) {
            initialValue = function(initialValue, value, index, container);
        });
        return initialValue;
    }

    /// <summary>`reduce` for associative containers with 4 argument callback</summary>
    /// <see cref="reduce" />
    /// <seealso cref="reduceArray" />
    /// <param name="container">The container.</param>
    /// <param name="function">callback(<paramref name="initialValue" />, currentValue, currentKey, <paramref name="container"
    /// />)</param>
    /// <param name="initialValue">Value to use as the first argument to the first call of the callback.</param>
    /// <returns>The value that results from the reduction.</returns>
    /// TODO Implement initialValue as optional: "[Optional] Value to use as the first argument to the first call of the callback.
    /// If no initial value is supplied, the first element in the array will be used. Calling reduce on an empty array without an
    /// initial value is an error."
    template <typename Container, typename Function, typename Memo>
    Memo reduceObject(const Container& container, Function function, Memo initialValue)
    {
        // ResultContainer result;
        auto keys = _::keys2(container);
        for (const auto& key : keys) {
            // const auto& value = container.at(key);
            auto value   = container.at(key);
            initialValue = function(initialValue, value, key, container);
        }
        return initialValue;
    }

    template <typename Container, typename Function, typename Memo>
    Memo inject(const Container& container, Function function, Memo initialValue)
    {
        return reduce(container, function, initialValue);
    }

    template <typename Container, typename Function, typename Memo>
    Memo foldl(const Container& container, Function function, Memo initialValue)
    {
        return reduce(container, function, initialValue);
    }

    // reduce_right/foldr
    template <typename Container, typename Function, typename Memo>
    Memo reduce_right(const Container& container, Function function, Memo initialValue)
    {
        for (typename Container::const_reverse_iterator i = container.rbegin(); i != container.rend(); ++i) {
            initialValue = function(initialValue, *i);
        }
        return initialValue;
    }

    /// <summary>A copy of std::find_if</summary>
    /// <param name="first">Iterator first.</param>
    /// <param name="last">Iterator last.</param>
    /// <param name="predicate">predicate</param>
    /// <returns></returns>
    template <class InputIterator, class UnaryPredicate>
    InputIterator find_if(InputIterator first, InputIterator last, UnaryPredicate predicate)
    {
        while (first != last) {
            if (predicate(*first)) return first;
            ++first;
        }
        return last;
    }

    template <typename Container, typename Function, typename Memo>
    Memo foldr(const Container& container, Function function, Memo initialValue)
    {
        return reduce_right(container, function, initialValue);
    }

    // find/detect
    /// <summary>Iterates over elements of collection, returning the first element predicate returns truthy for. The predicate is
    /// invoked with one argument: (value).</summary>
    /// <param name="container">The container.</param>
    /// <param name="predicate">The predicate.</param>
    /// <returns></returns>
    /// <remarks>This doesn't translate well into C++, as it should (by JavaScript underscore standards) return an actual element,
    /// or <c>undefined</c>.  While we could simulate <c>undefined</c> with C++17 <c>std::optional</c> usage would not be more
    /// convenient that returning an iterator.
    template <typename Container, typename Predicate>
    typename Container::iterator find(Container& container, Predicate predicate)
    {
        return _::find_if(container.begin(), container.end(), predicate);
    }

    template <typename Container, typename Predicate, typename Default>
    typename Container::value_type find(Container& container, Predicate predicate, Default function)
    {
        auto it = _::find_if(container.begin(), container.end(), predicate);
        if (it != container.end())
            return *it;

        if constexpr (std::is_convertible<Default, typename Container::value_type>::value)
        {
            return function;
        }

        else if constexpr (std::is_invocable_r<typename Container::value_type, Default()>::value)
        // else if constexpr (std::is_invocable<Default>::value) 
        {
            return function();
        }
        else 
        {
            // static_assert(!"Suck my dongle");
            throw std::runtime_error("couldn't find appropiate method for _::find");
        }
    }

    /// <summary>Iterates over elements of an associate collection, returning the first key the predicate returns truthy for. The
    /// predicate is invoked with three arguments: (value, index|key, collection).</summary>
    /// <param name="container">The container.</param>
    /// <param name="predicate">The predicate (value, key, collection)</param>
    /// <returns>The key of the first object found, or {}</returns>
    //template <typename Container, typename Function, typename Memo>
    //Memo findObject(const Container& container, Function predicate)
    template <typename Container, typename Function>
    typename Container::key_type findObject(const Container& container, Function predicate)
    {
        // ResultContainer result;
        auto keys = _::keys2(container);
        for (const auto& key : keys) {
            // const auto& value = container.at(key);
            auto value = container.at(key);
            auto found = predicate(value, key, container);
            if (found) return key;
        }
        return {};
    }

    template <typename ResultContainer, typename Container, typename Predicate>
    ResultContainer select(const Container& container, Predicate predicate)
    {
        return filter<ResultContainer>(container, predicate);
    }

    // reject
    template <typename ResultContainer, typename Container, typename Predicate>
    ResultContainer reject(const Container& container, Predicate predicate)
    {
        ResultContainer result;
        for (auto i = container.begin(); i != container.end(); ++i) {
            if (!predicate(*i)) {
                helper::add_to_container(result, *i);
            }
        }
        return result;
    }

    // all/every
    template <typename Container, typename Predicate>
    bool all(Container& container, Predicate&& predicate)
    {
        return std::all_of(std::begin(container), std::end(container), std::forward<Predicate>(predicate));
    }

    template <typename Container, typename Predicate>
    bool every(const Container& container, Predicate&& predicate)
    {
        return all(container, std::forward<Predicate>(predicate));
    }

    /// <summary>
    /// The signature of the predicate function should be equivalent to the following :
    /// <code>
    /// bool pred(const Type &amp;a);
    /// </code>
    /// The signature does not need to have const &amp;, but the function must not modify the objects passed to it.
    /// The type Type must be such that an object of type InputIt can be dereferenced and then implicitly converted to Type.​
    /// </summary>
    template <typename Container, typename Predicate>
    bool any(const Container& container, Predicate&& predicate)
    {
        return std::any_of(std::begin(container), std::end(container), std::forward<Predicate>(predicate));
    }

    template <typename Container, typename Predicate>
    bool some(const Container& container, Predicate&& predicate)
    {
        return any(container, std::forward<Predicate>(predicate));
    }

    // contains (alias includes)
    template <typename Container>
    bool contains(const Container& container, const typename Container::value_type& value)
    {
        return std::find(std::begin(container), std::end(container), value) != container.end();
    }

    template <typename Container>
    bool includes(const Container& container, const typename Container::value_type& value)
    {
        return contains(container, value);
    }

    // invoke
    template <typename ResultContainer, typename Container, typename Function>
    typename std::enable_if<!std::is_void<ResultContainer>::value, ResultContainer>::type invoke(
        const Container& container, Function&& function)
    {
        return map(container, std::mem_fn(std::forward<Function>(function)));
    }

    template <typename ResultContainer, typename Container, typename Function>
    typename std::enable_if<std::is_void<ResultContainer>::value, void>::type invoke(const Container& container, Function function)
    {
        each(container, std::mem_fn(std::forward<Function>(function)));
    }

    // pluck
    // Called like `lodash::pluck<vector<int>>(container, &value_type::member)`
    template <typename ResultContainer, typename Container, typename Member>
    ResultContainer pluck(Container const& container, Member member)
    {
        return invoke(container, member);
    }

    // copied from MSVC
    template <class _InIt, class _Ty, class _Fn>
    inline _Ty _Accumulate_unchecked(_InIt _First, _InIt _Last, _Ty _Val, _Fn& _Func)
    {  // return sum of _Val and all in [_First, _Last), using _Func
        for (; _First != _Last; ++_First) _Val = _Func(_Val, *_First);
        return (_Val);
    }

    // reduce/inject/foldl
    /// <summary>Applies a function against an accumulator and each element in the container (from left to right) to reduce it to a single value.</summary>
    /// <param name="container">The container.</param>
    /// <param name="function">The callback, callback(<paramref name="initialValue" />, currentValue, currentIndex, container)</param>
    /// <param name="initialValue">Value to use as the first argument to the first call of the callback.</param>
    /// <returns>The value that results from the reduction.</returns>
    /// <example><code><![CDATA[
    /// using fspath = std::experimental::filesystem::path;
    /// std::string pathCombine(const std::string& path, const std::vector<std::string>& more) {
    ///     fspath full_path = lodash::reduce(more, [](const fspath _path, const std::string& segment) {
    ///         return _path / filepath(segment);
    ///     }, filepath(path));
    /// 
    ///     full_path.string();
    /// }
    /// ]]></code></example>
    /// TODO Implement initialValue as optional: "[Optional] Value to use as the first argument to the first call of the callback. If no initial value is supplied, the first element in the array will be used. Calling reduce on an empty array without an initial value is an error."
    /// TODO Implement full range of functionality as described in https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/Reduce?v=b
    template <typename Container, typename Memo, typename BinaryOperation>
    Memo reduce(const Container& container, BinaryOperation function, Memo initialValue)
    {
        return std::accumulate(std::begin(container), std::end(container), std::move(initialValue), std::move(function));
        //each(container, [&](const auto& i) {
        //    initialValue = function(initialValue, i);
        //});
    }

    template <typename Container, typename T = typename Container::value_type>
    T sum(const Container& container, T init = T())
    {
        return reduce(container, std::plus<T>(), std::move(init));
    }

    /// <summary>This method is like _.sum except that it accepts iteratee which is invoked for each element in array to generate the value to be summed. The iteratee is invoked with one argument: (value).</summary>
    /// <param name="container">The container.</param>
    /// <param name="iteratee">The iteratee invoked per element.</param>
    /// <returns>The sum.</returns>
    template <typename R, typename Container, typename Function, typename T = typename Container::value_type>
    R sumBy(const Container& container, Function iteratee, R init = R())
    {
        return reduce(container, [](R total, T element) { return total + iteratee(element); }, std::move(init));
    }

    // max

    template <typename Container, typename T = typename Container::value_type>
    T max(const Container& container, T init = T())
    {
        return reduce(container, std::max<T>, init);
    }

    template <typename Compared, typename Container, typename Function>
    typename Container::iterator max(const Container& container, Function function)
    {
        if (container.begin() == container.end()) {
            return container.end();
        }

        struct {
            typename Container::iterator position;
            Compared                     computed;
        } max = {container.begin(), function(*container.begin())};

        for (typename Container::iterator i = ++container.begin(); i != container.end(); ++i) {
            Compared computed = function(*i);
            if (max.computed < computed) {
                max.position = i;
                max.computed = computed;
            }
        }
        return max.position;
    }

    // min
    template <typename Container>
    typename Container::iterator min(const Container& container)
    {
        if (container.begin() == container.end()) {
            return container.end();
        }

        typename Container::iterator min = container.begin();
        for (typename Container::iterator i = ++container.begin(); i != container.end(); ++i) {
            if (*i < *min) {
                min = i;
            }
        }
        return min;
    }

    /// <summary>Obtains the minimum element</summary>
    /// <param name="container">The container.</param>
    /// <param name="function">Function to compute value of each element</param>
    /// <returns>iterator to minimum element or end()</returns>
    /// <example>
    /// auto it = lodash::min<float>(players, [player](const auto& _) {
    ///     return player.distanceToSquared(_); // function returns float
    /// });
    /// if (it != players.end())
    ///     closestPlayer = *it;
    /// </example>

    template <typename Compared, typename Container, typename Function>
    typename Container::const_iterator min(const Container& container, Function function)
    {
        if (container.begin() == container.end()) {
            return container.end();
        }

        struct {
            typename Container::const_iterator position;
            Compared                           computed;
        } min = {container.begin(), function(*container.begin())};

        for (typename Container::const_iterator i = ++container.begin(); i != container.end(); ++i) {
            Compared computed = function(*i);
            if (computed < min.computed) {
                min.position = i;
                min.computed = computed;
            }
        }
        return min.position;
    }

    /// <summary>Returns an array of the elements in container1 and container2 that match, terminating at the first
    /// mismatch</summary>
    /// <param name="container1">container1.</param>
    /// <param name="container2">container2.</param>
    /// <returns></returns>
    /// <remarks>not an underscore or lodash function</remarks>
    template <typename ResultContainer, typename Container1, typename Container2>
    ResultContainer match_consecutive(Container1 const& container1, Container2 const& container2)
    {
        ResultContainer result;

        typename Container1::const_iterator left  = container1.begin();
        typename Container2::const_iterator right = container2.begin();
        while (left != container1.end() && right != container2.end()) {
            if (*left != *right) break;
            helper::add_to_container(result, *left);

            *left++, *right++;
        }

        return result;
    }

    // compare
    /// <summary>Compares the contents of two arrays</summary>
    /// <param name="container1">container1.</param>
    /// <param name="container2">container2.</param>
    /// <returns>-1, 0 or 1 as per <c>strcmp</c></returns>
    /// <remarks>not an underscore or lodash function</remarks>
    template <typename Container1, typename Container2>
    int compare(const Container1& container1, const Container2& container2)
    {
        typename Container1::const_iterator left  = container1.begin();
        typename Container2::const_iterator right = container2.begin();
        while (left != container1.end() && right != container2.end()) {
            if (*left != *right) return *left < *right ? -1 : 1;

            *left++, *right++;
        }

        // shorter container "win" (is less than)
        return
            // right is longer, ergo left is less
            (right != container2.end()) ? -1
                                        :
                                        // left is longer, ergo right is less
                (left != container1.end()) ? +1 :
                                           // both of equal length, ergo equal
                    0;
    }

    // reverse
    template <typename ResultContainer, typename Container>
    ResultContainer reverse(const Container& container)
    {
        ResultContainer result;
        for (typename Container::const_reverse_iterator i = container.rbegin(); i != container.rend(); ++i) {
            helper::add_to_container(result, *i);
        }
        return result;
    }

    // sort_by
    template <typename Container, typename Function>
    Container sortBy(const Container& container, Function function)
    {
        return sort_by(container, [=](const auto& left, const auto& right) { 
            return function(left) < function(right); 
        });
    }
    // sort_by
    template <typename Container, typename Function>
    Container sort_by(const Container& container, Function function)
    {
        std::vector<typename Container::value_type> to_sort(container.begin(), container.end());
        std::sort(to_sort.begin(), to_sort.end(), function);
        return Container(to_sort.begin(), to_sort.end());
    }

    // group_by
    template <typename Key, typename Container, typename Function>
    std::multimap<Key, typename Container::value_type> group_by(const Container& container, Function function)
    {
        std::multimap<Key, typename Container::value_type> result;
        for (auto i = container.begin(); i != container.end(); ++i) {
            result.insert(std::pair<Key, typename Container::value_type>(function(*i), *i));
        }
        return result;
    }

    // An internal function used for aggregate “group by” operations.
    //group = function(behavior) {
    //    return function(obj, iteratee, context) {
    //        var result = {};
    //        iteratee = cb(iteratee, context);
    //        _.each(obj, function(value, index) {
    //            var key = iteratee(value, index, obj);
    //            behavior(result, value, key);
    //        });
    //        return result;
    //    };
    //};

    // count_by
    template <typename Key, typename Container, typename Function>
    std::unordered_map<Key, size_t> count_by(const Container& container, Function function)
    {
        std::unordered_map<Key, size_t> result;
        for (auto i = container.begin(); i != container.end(); ++i) {
            auto key = function(*i);
            if (result.count(key))
                result[key] ++;
            else
                result[key] = 1;
        }
        return result;
    }

    // sorted_index
    template <typename Container>
    typename Container::iterator sorted_index(const Container& container, typename Container::value_type const& value)
    {
        return std::upper_bound(container.begin(), container.end(), value);
    }

    namespace helper {
        template <typename Argument, typename Function>
        class TransformCompare
#if _HAS_CXX17 == 0
            : std::binary_function<Argument, Argument, bool>
#endif
        {
           public:
            TransformCompare(Function const& function) : function_(function) {}

            bool operator()(Argument const& left, Argument const& right) const { return function_(left) < function_(right); }

           private:
            Function function_;
        };
    }  // namespace helper

    template <typename Container, typename Function>
    typename Container::iterator sorted_index(
        const Container& container, typename Container::value_type const& value, Function function)
    {
        return std::upper_bound(
            container.begin(), container.end(), value,
            helper::TransformCompare<typename Container::value_type, Function>(function));
    }

    template <typename T>
    T baseRandom(T min, T max)
    {
        static std::random_device rd;
        static std::mt19937       gen(rd());
        if (min == max) return max;
        std::uniform_int_distribution<> dis(min, max);
        return dis(gen);
    }

    template <typename Container>
    typename std::vector<typename Container::value_type> vectorize(const Container& container)
    {
        return values _VECTOR (typename Container::value_type)(container);
    }


    /**
    * A specialized version of `_.sample` for arrays.
    *
    * @private
    * @param {Array} array The array to sample.
    * @returns {*} Returns the random element.
    */
    template <typename Container>
    typename Container::value_type arraySample(const Container& container)
    {
        auto vector                 = values _VECTOR(typename Container::value_type)(container);
        auto                 length = std::size(vector);
        return vector[baseRandom<size_t>(0, length - 1)];
    }

    /**
    * The base implementation of `_.sample`.
    *
    * @private
    * @param {Array|Object} collection The collection to sample.
    * @returns {*} Returns the random element.
    */
    template <typename Container>
    typename Container::value_type baseSample(const Container& container)
    {
        auto values                       = tuple_values _VECTOR(typename Container::value_type)(container);
        auto                       length = std::size(values);
        return values[baseRandom(0, length - 1)];
    }

    // Gets a random element from `collection`.
    template <typename Container>
    typename Container::value_type sample(const Container& container)
    {
        if
            constexpr(traits::has_mapped_type<Container>::value) { return baseSample(container); }
        else {
            return arraySample(container);
        }
    }

	template<typename Container>
	std::string join_array(const Container& container, const char* const separator) {
		std::ostringstream os;
		// http://stackoverflow.com/a/5289170/912236
		// see also: https://stackoverflow.com/questions/191757/how-to-concatenate-a-stdstring-and-an-int

		const char* _separator = "";
		for (auto& item : container) {
			os << _separator << item;
			_separator = separator;
		}
		return os.str();
	}

	template<typename Container>
    std::string join_map(const Container& container, const char* const separator) {
        auto values = tuple_values _VECTOR(typename Container::value_type)(container);
        return join_array(values, separator);
    }

	template<typename Container>
	std::string join(const Container& container, const char* const separator) {
        if constexpr(traits::has_mapped_type<Container>::value) { 
            return join_map(container, separator);
        }
        else {
            return join_array(container, separator);
        }
	}

    // shuffle
    // This assumes srand has already been called.
    template <typename ResultContainer, typename Container>
    ResultContainer shuffle(const Container& container)
    {
        std::vector<typename Container::value_type> deck(container.begin(), container.end());
        for (int i = deck.size() - 1; i > 1; --i) {
            int j = std::rand() % (i + 1);
            std::swap(deck[i], deck[j]);
        }
        return ResultContainer(deck.begin(), deck.end());
    }

    // to_array
    template <typename Container>
    typename Container::value_type* to_array(const Container& container)
    {
        typename Container::value_type* array = new typename Container::value_type[container.size()];
        struct {
            int                          numeric;
            typename Container::iterator iterator;
        } i;
        for (i.numeric = 0, i.iterator = container.begin(); i.iterator != container.end(); ++i.numeric, ++i.iterator) {
            array[i.numeric] = *i.iterator;
        }

        return array;
    }

    // size
    template <typename Container>
    int size(const Container& container)
    {
        return container.size();
    }

    // Arrays

    // first/head
    template <typename Container>
    typename Container::iterator first(Container& container)
    {
        return container.begin();
    }

    template <typename ResultContainer, typename Container>
    ResultContainer first(Container& container, int count)
    {
        typename Container::iterator end = container.begin();
        std::advance(end, count);
        return ResultContainer(container.begin(), end);
    }

    template <typename Container>
    typename Container::iterator head(Container& container)
    {
        return first(container);
    }

    template <typename ResultContainer, typename Container>
    ResultContainer head(Container& container, int count)
    {
        return first<ResultContainer>(container, count);
    }

    /// <summary>Similar to <paramref="first" /> but returns an array of between 0 and 1 elements</summary>
    /// <param name="container">The container.</param>
    /// <returns></returns>
    template <typename ResultContainer, typename Container>
    ResultContainer first_jquery(Container& container)
    {

        ResultContainer result;
        for (auto i = container.begin(); i != container.end(); ++i) {
            helper::add_to_container(result, *i);
            break;
        }
        return result;
    }

    // initial
    template <typename ResultContainer, typename Container>
    ResultContainer initial(Container& container)
    {
        typename Container::iterator end = container.begin();
        std::advance(end, container.size() - 1);
        return ResultContainer(container.begin(), end);
    }

    template <typename ResultContainer, typename Container>
    ResultContainer initial(Container& container, int n)
    {
        typename Container::iterator end = container.begin();
        std::advance(end, container.size() - n);
        return ResultContainer(container.begin(), end);
    }

    // last
    template <typename Container>
    typename Container::iterator last(Container& container)
    {
        typename Container::iterator last = container.begin();
        std::advance(last, container.size() - 1);
        return last;
    }

    template <typename ResultContainer, typename Container>
    ResultContainer last(Container& container, int n)
    {
        typename Container::iterator begin = container.begin();
        std::advance(begin, container.size() - n);
        return ResultContainer(begin, container.end());
    }

    // rest/tail
    template <typename ResultContainer, typename Container>
    ResultContainer rest(Container& container)
    {
        return ResultContainer(++container.begin(), container.end());
    }

    template <typename ResultContainer, typename Container>
    ResultContainer rest(Container& container, int index)
    {
        typename Container::iterator begin = container.begin();
        std::advance(begin, index);
        return ResultContainer(begin, container.end());
    }

    template <typename ResultContainer, typename Container>
    ResultContainer tail(Container& container)
    {
        return rest<ResultContainer>(container);
    }

    template <typename ResultContainer, typename Container>
    ResultContainer tail(Container& container, int index)
    {
        return rest<ResultContainer>(container, index);
    }

    // concat
    /// <summary>Creates a new array concatenating array <paramref="container1" /> with <paramref="container2" /></summary>
    /// <param name="container1">container1</param>
    /// <param name="container2">container2</param>
    /// <returns></returns>
    template <typename ResultContainer, typename Container1, typename Container2>
    ResultContainer concat(const Container1& container1, const Container2& container2)
    {
        ResultContainer result;

        // This may be a terrible idea, if reserve or size is not defined.
        // result.reserve(container1.size() + container2.size());
        each(container1, [&result](auto value) { helper::add_to_container(result, value); });
        each(container2, [&result](auto value) { helper::add_to_container(result, value); });

        // vector1.insert(vector1.end(), vector2.begin(), vector2.end());
        // for (auto i = container.begin(); i != container.end(); ++i)
        //{
        //    if (static_cast<bool>(*i))
        //    {
        //        helper::add_to_container(result, *i);
        //    }
        //}
        return result;
    }

    // append
    /// <summary>Add a new element <paramref="element" /> to copy of an existing array <paramref="container" /> with </summary>
    /// <param name="container">container</param>
    /// <param name="element">element</param>
    /// <returns></returns>
    template <typename Container>
    Container append(const Container& container, const typename Container::value_type& element)
    {
        Container result;

        // This may be a terrible idea, if reserve or size is not defined.
        // result.reserve(container1.size() + container2.size());
        each(container, [&result](auto value) { helper::add_to_container(result, value); });
        helper::add_to_container(result, element);

        // vector1.insert(vector1.end(), vector2.begin(), vector2.end());
        // for (auto i = container.begin(); i != container.end(); ++i)
        //{
        //    if (static_cast<bool>(*i))
        //    {
        //        helper::add_to_container(result, *i);
        //    }
        //}
        return result;
    }

    /// <summary>Append the contents of <paramref="source" /> to <paramref="destination" /></summary>
    /// <param name="destination">The destination array</param>
    /// <param name="source">The source array</param>
    /// <returns>void</returns>
    template <typename Container1, typename Container2>
    void concat_inplace(Container1& destination, Container2 const& source)
    {
        // This may be a terrible idea, if reserve or size is not defined.
        // result.reserve(container1.size() + source.size());
        // each(container1, [&result](auto value) { helper::add_to_container(result, value);  });
        each(source, [&destination](const auto& value) { helper::add_to_container(destination, value); });

        // vector1.insert(vector1.end(), vector2.begin(), vector2.end());
        // for (auto i = container.begin(); i != container.end(); ++i)
        //{
        //    if (static_cast<bool>(*i))
        //    {
        //        helper::add_to_container(result, *i);
        //    }
        //}
        // return result;
    }

    // compact
    template <typename ResultContainer, typename Container>
    ResultContainer compact(Container const& container)
    {
        ResultContainer result;
        for (auto i = container.begin(); i != container.end(); ++i) {
            if (static_cast<bool>(*i)) {
                helper::add_to_container(result, *i);
            }
        }
        return result;
    }

    // flatten
    namespace helper {
        template <typename ResultContainer, typename Container>
        ResultContainer flatten_one_layer(Container const& container)
        {
            ResultContainer result;
            for (auto i = container.begin(); i != container.end(); ++i) {
                for (typename Container::value_type::const_iterator j = i->begin(); j != i->end(); ++j) {
                    add_to_container(result, *j);
                }
            }
            return result;
        }

        template <typename T>
        class HasConstIterator {
           private:
            typedef char yes[1];
            typedef char no[2];
            template <typename C>
            static yes& test(typename C::const_iterator*);
            template <typename C>
            static no& test(...);

           public:
            static bool const value = sizeof(test<T>(0)) == sizeof(yes);
        };

        template <typename ResultContainer, typename Container>
        typename std::enable_if<!HasConstIterator<typename Container::value_type>::value, void>::type flatten_loop(
            ResultContainer& result, Container const& container)
        {
            for (auto i = container.begin(); i != container.end(); ++i) {
                add_to_container(result, *i);
            }
        }

        template <typename ResultContainer, typename Container>
        typename std::enable_if<HasConstIterator<typename Container::value_type>::value, void>::type flatten_loop(
            ResultContainer& result, Container const& container)
        {
            for (auto i = container.begin(); i != container.end(); ++i) {
                flatten_loop(result, *i);
            }
        }

    }  // namespace helper

    template <typename ResultContainer, typename Container>
    ResultContainer flatten(Container const& container)
    {
        ResultContainer result;
        helper::flatten_loop(result, container);
        return result;
    }

    template <typename ResultContainer, bool                        shallow, typename Container>
    typename std::enable_if<shallow == true, ResultContainer>::type flatten(Container const& container)
    {
        return helper::flatten_one_layer<ResultContainer>(container);
    }

    template <typename ResultContainer, bool                         shallow, typename Container>
    typename std::enable_if<shallow == false, ResultContainer>::type flatten(Container const& container)
    {
        return flatten<ResultContainer>(container);
    }

    // uniq/unique
    template <typename ResultContainer, typename Key, typename Container, typename Function>
    ResultContainer uniq(Container const& container, bool is_sorted, Function function)
    {
        ResultContainer  result;
        std::vector<Key> keys = map<std::vector<Key>>(container, function);
        if (container.size() < 3) {
            is_sorted = true;
        }

        std::vector<Key> memo;

        for (std::pair<typename std::vector<Key>::const_iterator, typename Container::const_iterator>
                 i = std::make_pair(keys.begin(), container.begin());
             i.first != keys.end(); ++i.first, ++i.second) {
            if (is_sorted ? !memo.size() || *last(memo) != *i.first : !includes(memo, *i.first)) {
                memo.push_back(*i.first);
                helper::add_to_container(result, *i.second);
            }
        }
        return result;
    }

    /// <summary>This method is like _.uniq except that it accepts iteratee which is invoked for each element in array to generate
    /// the criterion by which uniqueness is computed. The order of result values is determined by the order they occur in the
    /// array. The iteratee is invoked with one argument</summary>
    /// <param name="container">The container.</param>
    /// <param name="function">iteratee(<paramref name="Key" /> identity)</param>
    /// <returns></returns>
    template <typename ResultContainer, typename Key, typename Container, typename Function>
    ResultContainer uniqBy(Container const& container, Function function)
    {
        return uniq<ResultContainer, Key>(container, false, function);
    }

    template <typename ResultContainer, typename Key, typename Container, typename Function>
    ResultContainer uniq(Container const& container, Function function)
    {
        return uniq<ResultContainer, Key>(container, false, function);
    }

    template <typename ResultContainer, typename Container>
    ResultContainer uniq(Container const& container, bool is_sorted)
    {
        ResultContainer result;
        if (container.size() < 3) {
            is_sorted = true;
        }

        std::vector<typename Container::value_type> memo;
        for (auto i = container.begin(); i != container.end(); ++i) {
            if (is_sorted ? !memo.size() || *last(memo) != *i : !includes(memo, *i)) {
                memo.push_back(*i);
                helper::add_to_container(result, *i);
            }
        }
        return result;
    }

    template <typename ResultContainer, typename Container>
    ResultContainer uniq(Container const& container)
    {
        return uniq<ResultContainer>(container, false);
    }

    template <typename ResultContainer, typename Key, typename Container, typename Function>
    ResultContainer unique(Container const& container, bool is_sorted, Function function)
    {
        return uniq<ResultContainer, Key>(container, is_sorted, function);
    }

    template <typename ResultContainer, typename Key, typename Container, typename Function>
    ResultContainer unique(Container const& container, Function function)
    {
        return uniq<ResultContainer, Key>(container, false, function);
    }

    template <typename ResultContainer, typename Container>
    ResultContainer unique(Container const& container, bool is_sorted)
    {
        return uniq<ResultContainer>(container, is_sorted);
    }

    template <typename ResultContainer, typename Container>
    ResultContainer unique(Container const& container)
    {
        return uniq<ResultContainer>(container, false);
    }

    template <typename ResultContainer, typename Container, typename Function>
    ResultContainer unique(Container const& container, Function function)
    {
        return uniq<ResultContainer>(container, false);
    }

    // union_of
    template <typename ResultContainer, typename Container1, typename Container2>
    ResultContainer union_of(Container1 const& container1, Container2 const& container2)
    {
        std::vector<typename ResultContainer::value_type> left(container1.begin(), container1.end());
        std::vector<typename ResultContainer::value_type> right(container2.begin(), container2.end());
        std::sort(left.begin(), left.end());
        std::sort(right.begin(), right.end());

        std::vector<typename ResultContainer::value_type> union_result;
        std::set_union(left.begin(), left.end(), right.begin(), right.end(), std::back_inserter(union_result));
        return ResultContainer(union_result.begin(), union_result.end());
    }

    // intersection
    template <typename ResultContainer, typename Container1, typename Container2>
    ResultContainer intersection(Container1 const& container1, Container2 const& container2)
    {
        std::vector<typename ResultContainer::value_type> left(container1.begin(), container1.end());
        std::vector<typename ResultContainer::value_type> right(container2.begin(), container2.end());
        std::sort(left.begin(), left.end());
        std::sort(right.begin(), right.end());

        std::vector<typename ResultContainer::value_type> union_result;
        std::set_intersection(left.begin(), left.end(), right.begin(), right.end(), std::back_inserter(union_result));
        return ResultContainer(union_result.begin(), union_result.end());
    }

    // difference
    template <typename ResultContainer, typename Container1, typename Container2>
    ResultContainer difference(Container1 const& container1, Container2 const& container2)
    {
        std::vector<typename ResultContainer::value_type> left(container1.begin(), container1.end());
        std::vector<typename ResultContainer::value_type> right(container2.begin(), container2.end());
        std::sort(left.begin(), left.end());
        std::sort(right.begin(), right.end());

        std::vector<typename ResultContainer::value_type> union_result;
        std::set_difference(left.begin(), left.end(), right.begin(), right.end(), std::back_inserter(union_result));
        return ResultContainer(union_result.begin(), union_result.end());
    }

    // difference2 - because `difference` doesn't work for all types
    template <typename ResultContainer, typename Container1, typename Container2>
    ResultContainer difference2(Container1 const& container1, Container2 const& container2)
    {
        return filter<ResultContainer>(container1, [&](const auto& value) { return !contains(container2, value); });
    }

    // zip
    /// <summary>Merges together the values of each of the arrays with the values at the corresponding position. Useful when you
    /// have separate data sources that are coordinated through matching array indexes.
    /// </summary>
    /// <param name="container1">The container1.</param>
    /// <param name="container2">The container2.</param>
    /// <returns>A sequential container of size <c>min(keys.size(), values.size())</c></returns>
    /// <example><code><![CDATA[// JavaScript example from underscore.org
    /// _.zip(['moe', 'larry', 'curly'], [30, 40, 50], [true, false, false]);
    /// // => [["moe", 30, true], ["larry", 40, false], ["curly", 50, false]]    /// }]]></code></example>
    /// <remarks>Limited to 2 arrays</remarks>
    /// <remarks>lodash version should actually take an array of arrays as a single argument</remarks>
    template <typename ResultContainer, typename Container1, typename Container2>
    ResultContainer zip(const Container1& container1, const Container2& container2)
    {
        ResultContainer                     result;
        typename Container1::const_iterator left  = container1.begin();
        typename Container2::const_iterator right = container2.begin();
        while (left != container1.end() && right != container2.end()) {
            helper::add_to_container(result, typename ResultContainer::value_type(*left++, *right++));
        }
        return result;
    }

    /// <summary>Converts arrays into objects. Pass a list of keys, and a list of values. If duplicate keys exist, the last value
    /// wins.</summary>
    /// <param name="keys">The keys.</param>
    /// <param name="values">The values.</param>
    /// <returns>An associative container of size <c>min(keys.size(), values.size())</c></returns>
    /// <example><code><![CDATA[// JavaScript example from https://lodash.com/docs/4.17.4#zipObject
    /// _.zipObject(['a', 'b'], [1, 2]);
    /// // => { 'a': 1, 'b': 2 }    /// <remarks>Limited to 2 arrays</remarks>
    /// <remarks>a.k.a. underscore's <c>object()</c> function, when passing a list of keys, and a list of values</remarks>
    /// <remarks></remarks>
    template <typename ResultContainer, typename Container1, typename Container2>
    ResultContainer zipObject(const Container1& keys, const Container2& values)
    {
        ResultContainer                     result;
        typename Container1::const_iterator left  = keys.begin();
        typename Container2::const_iterator right = values.begin();
        while (left != keys.end() && right != values.end()) {
            helper::add_to_container(result, typename ResultContainer::value_type(*left++, *right++));
        }
        return result;
    }

    // indexOf
    template <typename Container>
    int indexOf(const Container& container, typename Container::value_type value)
    {
        auto value_position = std::find(container.begin(), container.end(), value);
        return value_position == container.end() ? -1 : std::distance(container.begin(), value_position);
    }

    template <typename Container, typename Function>
    int indexOf_if(const Container& container, Function&& predicate)
    {
        auto value_position = std::find_if(container.begin(), container.end(), std::forward<Function>(function));
        return value_position == container.end() ? -1 : std::distance(container.begin(), value_position);
    }

    // `indexOf` that accepts `Container::value_type = std::pair<K, V>`
    template <typename Container, typename Value>
    int indexOfMap(const Container& container, Value value)
    {
        // https://stackoverflow.com/questions/12742472/how-to-get-matching-key-using-the-value-in-a-map-c

        auto value_position =
            std::find_if(std::begin(container), std::end(container), [&](const auto& pair) { return pair.second == value; });

        return value_position == container.end() ? -1 : std::distance(container.begin(), value_position);
    }

    // `contains` that accepts `Container::value_type = std::pair<K, V>`
    template <typename Container, typename Value>
    bool containsMap(const Container& container, Value value)
    {
        return indexOfMap(container, value) != -1;
    }

    template <typename Container>
    int indexOf(const Container& container, typename Container::value_type value, bool is_sorted)
    {
        if (!is_sorted) {
            return indexOf(container, value);
        }
        typename Container::iterator value_lower_bound = std::lower_bound(container.begin(), container.end(), value);
        return value_lower_bound == container.end() || *value_lower_bound != value ? -1 : std::distance(
                                                                                              container.begin(), value_lower_bound);
    }

    // last_index_of
    template <typename Container>
    int last_index_of(Container const& container, typename Container::value_type value)
    {
        typename Container::const_iterator result = std::find(container.begin(), container.end(), value);
        typename Container::const_iterator i      = result;
        while (i != container.end()) {
            i = std::find(++i, container.end(), value);
            if (i != container.end()) {
                result = i;
            }
        }
        return result == container.end() ? -1 : std::distance(container.begin(), result);
    }

    // range
    template <typename ResultContainer>
    ResultContainer range(int start, int stop, int step)
    {
        int             length = std::max((stop - start) / step, 0);
        int             index  = 0;
        ResultContainer result;

        while (index < length) {
            helper::add_to_container(result, start);
            start += step;
            ++index;
        }

        return result;
    }

    template <typename ResultContainer>
    ResultContainer range(int start, int stop)
    {
        return range<ResultContainer>(start, stop, 1);
    }

    template <typename ResultContainer>
    ResultContainer range(int stop)
    {
        return range<ResultContainer>(0, stop, 1);
    }

    template <typename T>
    auto dereference(T _)
    {
        return *_;
    }

    /// <summary>Invokes the iteratee n times, returning an array of the results of each invocation. The iteratee is invoked with
    /// one argument; (index).</summary>
    /// <param name="n">The number of times to invoke <paramref="iteratee" /></param>
    /// <param name="iteratee">The iteratee, iteratee(size_t n)</param>
    /// <returns>Array of the returned values</returns>
    template <typename ResultContainer, typename Function>
    ResultContainer times(size_t n, Function iteratee)
    {
        auto accum = range<std::vector<size_t>>(n);
        return map<ResultContainer>(accum, iteratee);
    };

    /// <summary>The same as `times` but doesn't collate return values or pass iteration</summary>
    /// <param name="n">The number of times to invoke <paramref="iteratee" /></param>
    /// <param name="iteratee">The iteratee, void iteratee()</param>
    /// <returns>Array of the returned values</returns>
    template <typename Function>
    void timesSimple(size_t n, Function iteratee)
    {
        while (n-- > 0)
            iteratee();
    };

#ifdef UNDERSCORE_BONUS
    // at
    template <typename Container>
    auto& at(Container& container, typename Container::key_type key)
    {
        return container.at(key);
    }

    template <typename Container>
    auto tryAndGet(const Container& container, const typename Container::key_type key, typename Container::value_type& value)
    {
        if (contains(container, key)) {
            value = at(container, key);
            return true;
        }
        return false;
    }

    template <typename Container>
    auto& getOrCall(
        const Container& container, const typename Container::key_type key,
        typename Container::value_type (*function)(typename Container::key_type))
    {
        if (!contains(container, key)) helper::add_to_container(container, key, function(key));
        return at(container, key);
    }

    template <typename Container>
    auto& getOrDefault(
        const Container& container, const typename Container::key_type key, const typename Container::value_type& value)
    {
        if (!contains(container, key)) helper::add_to_container(container, key, value);
        return at(container, key);
    }

    template <typename Container>
    bool tryAndPop(Container& the_queue, typename Container::value_type& popped_value)
    {
        if (the_queue.empty()) {
            return false;
        }

        popped_value = the_queue.front();
        the_queue.pop_front();
        return true;
    }

    /// <summary>Perumtate the specified containers.</summary>
    /// <param name="container1">container 1.</param>
    /// <param name="container2">container 2.</param>
    /// <param name="iteratee">The iteratee, iteratee(a, b)</param>
    /// <returns>Unique permutations of the two containers passed through iteratee</returns>
    template <typename ResultContainer, typename Container1, typename Container2, typename Function>
    ResultContainer permutate(const Container1& container1, const Container2& container2, Function iteratee)
    {
        ResultContainer result;
        for (auto i = 0; i < container1.size(); i++) {
            for (auto j = 0; j < container2.size(); j++) {
                helper::add_to_container(result, iteratee(container1[i], container2[j]));
            }
        }
        return result;
    }
#endif

    // Functions

    // bind
    // bindAll
    // memoize
    // delay
    // defer
    // throttle
    // debounce
    // once
    // after
    // wrap
    // compose

    // Objects

    // keys
    // values
    // functions
    // extend
    // defaults
    // clone
    // tap
    // has
    // isEqual
    // isEmpty
    // isElement
    // isArray
    // isArguments
    // isFunction
    // isString
    // isNumber
    // isBoolean
    // isDate
    // isRegExp
    // isNaN
    // isNull
    // isUndefined

    // Utility

    // noConflict
    // identity
    // times
    // mixin
    // uniqueId
    // escape
    // template

    // Chaining

    template <typename Container>
    class Wrapper;

    // chain
    template <typename Container>
    Wrapper<Container> chain(Container container)
    {
        return Wrapper<Container>(std::move(container));
    }

    // value
    template <typename Container>
    typename Container::value_type value(Wrapper<Container>& wrapper)
    {
        return wrapper.value();
    }

    template <typename Container>
    class Wrapper {
       public:
        // what's this for?
        typedef Container value_type;
        Wrapper(Container container) : container_(std::move(container)) {}

        Container value() { return container_; }

        template <typename Function>
        Wrapper& each(Function function)
        {
            _::each(container_, function);
            return *this;
        }

        template <typename Function, typename ResultContainer = std::vector<typename Container::value_type>>
        Wrapper<ResultContainer> filter(Function function)
        {
            return chain(_::filter<ResultContainer>(container_, function));
        }

        template <typename ResultContainer, typename Function>
        Wrapper<ResultContainer> map(Function function)
        {
            return chain(_::map<ResultContainer>(container_, function));
        }

        template <typename Function, typename Memo>
        Wrapper<Memo> reduce(Function function, Memo memo)
        {
            return chain(_::reduce(container_, function, memo));
        }

        template <typename ResultContainer = std::vector<typename Container::value_type>>
        Wrapper<ResultContainer> tuple_keys()
        {
            return chain(_::tuple_keys<ResultContainer>(container_));
        }


       private:
        Container container_;
    };

}  // namespace _

// namespace _ = _;

// template <typename Key, typename Iterator>
// struct KeyIterator
//{
//    KeyIterator(
//        Iterator i)
//        :_i(i)
//    {
//    }
//
//    KeyIterator operator++()
//    {
//        ++_i;
//        return *this;
//    }
//
//    bool operator==(
//        KeyIterator ki)
//    {
//        return _i = ki._i;
//    }
//
//    typename Iterator::value_type operator*()
//    {
//        return _i->first;
//    }
//};

// see also: https://stackoverflow.com/questions/43992510/enable-if-to-check-if-value-type-of-iterator-is-a-pair
#endif  // UNDERSCORE_UNDERSCORE_H_
