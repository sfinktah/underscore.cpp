#ifndef UNDERSCORE_UNDERSCORE_H_
#define UNDERSCORE_UNDERSCORE_H_
#define UNDERSCORE_BONUS

#include <cstdlib>
#include <algorithm>
#include <functional>
#include <iterator>
#include <map>
#include <vector>
#include <utility>

namespace _
{
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

    namespace helper
    {

        // For a number of Underscore functions, the elements of a container are
        // transformed in some way, and the results are placed in another container.
        // To be able to support different kinds of containers, a way of choosing the
        // proper method for addition to the result container must be called, but these
        // methods are not uniform across the standard library.

        // To get around this, the correct function to call must be determined at
        // compile time using metafunctions and SFINAE.

        // Because the body to determine whether or not a given member function is
        // relatively large, the HAS_MEMBER_FUNCTION macro is temporarily defined to
        // help reduce code size.
        // This is from http://stackoverflow.com/a/264088/1256
#define HAS_MEMBER_FUNCTION(func, name)                                                            \
    template <typename T, typename Sign>                                                           \
    struct name                                                                                    \
    {                                                                                              \
        typedef char yes[1];                                                                       \
        typedef char no[2];                                                                        \
        template <typename U, U>                                                                   \
        struct type_check;                                                                         \
        template <typename _1>                                                                     \
        static yes& chk(type_check<Sign, &_1::func>*);                                             \
        template <typename>                                                                        \
        static no& chk(...);                                                                       \
        static bool const value = sizeof(chk<T>(0)) == sizeof(yes);                                \
    }

        // Use the macro to define metafunctions for the various insertion methods that
        // Underscore supports. Primarily, these will be single parameter member
        // functions that are used across multiple types in the standard library.
        HAS_MEMBER_FUNCTION(push_back, HasPushBack);
        HAS_MEMBER_FUNCTION(insert, HasInsert);

        // Remove the macro so that it doesn't pollute the global scope.
#undef HAS_MEMBER_FUNCTION

        // To simplify function declarations later, the insertion capabilities for a
        // given type are simply listed in a struct.
        template <typename Container>
        struct MemberAdditionCapabilities
        {
            static bool const has_push_back = HasPushBack<Container,
                void (Container::*)(const typename Container::value_type&)>::value;
            static bool const has_insert = HasInsert<Container, std::pair<typename Container::iterator, bool>(Container::*)(
                    const typename Container::value_type&)>::value;
        };

        template <typename Container>
        struct HasSupportedAdditionMethod
        {
            static bool const value = MemberAdditionCapabilities<Container>::has_push_back
                || MemberAdditionCapabilities<Container>::has_insert;
        };

        // A simple implementation of enable_if allows alternative functions to be
        // selected at compile time.
        // This is from http://stackoverflow.com/a/264088/1256
        template <bool C, typename T = void>
        struct enable_if
        {
            typedef T type;
        };

        template <typename T>
        struct enable_if<false, T>
        {
        };

        template <typename Container>
        typename enable_if<MemberAdditionCapabilities<Container>::has_insert, void>::type insert(
            Container& container, typename Container::value_type const& value)
        {
            container.insert(value);
        }

        template <typename Container>
        typename enable_if<MemberAdditionCapabilities<Container>::has_push_back, void>::type push_back(
            Container& container, typename Container::value_type const& value)
        {
            container.push_back(value);
        }

        template <typename Container>
        typename enable_if<!MemberAdditionCapabilities<Container>::has_push_back, void>::type push_back(
            Container& container, typename Container::value_type const& value)
        {
            insert(container, value);
        }

        template <typename Container>
        typename enable_if<HasSupportedAdditionMethod<Container>::value, void>::type add_to_container(
            Container& container, typename Container::value_type const& value)
        {
            push_back(container, value);
        }

        template <typename Container>
        typename enable_if<HasSupportedAdditionMethod<Container>::value, void>::type add_to_container(
            Container& container, typename Container::key_type const& key, typename Container::value_type const& value)
        {
            container.insert(container, value);
        }

        template <typename T>
        struct is_void
        {
            static bool const value = false;
        };

        template <>
        struct is_void<void>
        {
            static bool const value = true;
        };

		template<typename T>
		constexpr T clamp(T value, T min, T max) {
			return (
				value > max ? max :
				value < min ? min :
				value
			);
		}
    } // namespace helper

      // Collections

      // each/for_each

	// http://en.cppreference.com/w/cpp/algorithm/for_each
    template <typename Container, typename Function>
    void each(Container container, Function function)
    {
		//function - function object, to be applied to the result of dereferencing every iterator in the range[first, last)
		//The signature of the function should be equivalent to the following :

		//	void fun(const Type &a);

		//The signature does not need to have `const &`.

		//The type Type must be such that an object of type InputIt can be dereferenced and then implicitly converted to Type.

        std::for_each(container.begin(), container.end(), function);
    }

    template <typename Container, typename Function>
	void each_with_distance(Container container, Function function)
	{
		//The signature of the function should be equivalent to the following :

		//	void fun(const Type &a, const size_t d);
		for (auto i = container.begin(); i != container.end(); ++i) {
			function(*i, std::distance(container.begin(), i));
		}
	}

    template <typename Container, typename Function>
	void each_iter(Container container, Function function)
	{
		//The signature of the function should be equivalent to the following :

		//	void fun(const Type &a, const size_t d);
		for (auto i = container.begin(); i != container.end(); ++i) {
			function(i);
		}
	}

	//  The full power of `each`.  Each invocation of iteratee is called 
	//  with three arguments: (element, index, list). If list is an object, 
	//  iteratee's arguments will be (value, key, list).  (MDN)
    template <typename Container, typename Function>
	void each_key_value(Container container, Function function)
	{
		for (auto i = container.begin(); i != container.end(); ++i) {
			auto key = i->first;
			auto value = i->second;
			function(value, key, container);
		}
	}

	//  each - for nlohmann::json containers. iteratee has two arguments: (value, key). 
    template <typename Container, typename Function>
	void each_json(Container container, Function function)
	{
		for (auto i = container.begin(); i != container.end(); ++i) {
			auto key = i.key();
			auto value = i.value();
			function(value, key);
		}
	}

    template <typename Container, typename Function>
    void for_each(Container container, Function function)
    {
        each(container, function);
    }

    // map/collect
    template <typename ResultContainer, typename Container, typename Function>
    ResultContainer map(Container container, Function function)
    {
        ResultContainer result;

        for (auto& item : container) helper::add_to_container(result, function(item));
        //for (auto i = container.begin(); i != container.end(); ++i)
        //{
        //	helper::add_to_container(result, function(*i));
        //}
        return result;
    }

    // map/collect with raw iterators -- sfink
    template <typename ResultContainer, typename Container, typename Function>
    ResultContainer map_iter(Container container, Function function)
    {
        ResultContainer result;
        for (auto i = container.begin(); i != container.end(); ++i)
        {
            helper::add_to_container(result, function(i));
        }
        return result;
    }

	// remove (lodash) - Removes all elements from array that predicate returns truthy for and returns an array of the removed elements.
	// Note: Unlike _.filter, this method mutates array. Use _.pull to pull elements from an array by value.
	// Note: for ease of use, this function does not return removed elements, use `removeAndReturn` instead
    template <typename Container, typename Function>
	void remove(Container& container, Function function) {
		for ( auto i = container.begin(); i != container.end() ; )
			function(*i) ? i = container.erase(i) : ++i;
	}

	// remove (lodash) - Removes all elements from array that predicate returns truthy for and returns an array of the removed elements.
	// Note: Unlike _.filter, this method mutates array. Use _.pull to pull elements from an array by value.
    template <typename ResultContainer, typename Container, typename Function>
	ResultContainer removeAndReturn(Container& container, Function function) {
        ResultContainer result;
		for (auto i = container.begin(); i != container.end(); ) {
			if (function(*i))
				helper::add_to_container(result, *i), 
				i = container.erase(i);
			else
				++i;
		}
	}

	// pull (lodash) - Removes all given values from array using SameValueZero for equality comparisons.
	// Note: Unlike `without`, this method mutates array. Use _.remove to remove elements from an array by predicate.
    template <typename Container>
    void pull(Container& container, typename Container::value_type const& value)
    {
		for (auto i = container.begin(); i != container.end(); )
			(*i == value) ? i = container.erase(i) : ++i;
    }

	// pullAll (lodash) - This method is like _.pull except that it accepts an array of values to remove.
    // Note: Unlike _.difference, this method mutates array.
    template <typename Container1, typename Container2>
    void pullAll(Container1& container, Container2 const& values)
    {

		// Hmmm.... if it's similar to difference, maybe we could leverage the existing `difference` function...
		// However, that function looks complicated. Lets leverage `contains` instead. It's possibly less 
		// efficient that using `difference` but simplicity wins today.
		for (auto i = container.begin(); i != container.end(); )
			contains(values, *i) ? i = container.erase(i) : ++i;
    }

    // filter/select
    template <typename ResultContainer, typename Container, typename Predicate>
    ResultContainer filter(Container container, Predicate predicate)
    {
        ResultContainer result;
        for (auto i = container.begin(); i != container.end(); ++i)
        {
            if (predicate(*i))
            {
                helper::add_to_container(result, *i);
            }
        }
        return result;
    }


	// without - Creates an array excluding all given values using SameValueZero for equality comparisons.
	// Note: Unlike `pull`, this method returns a new array.
    template <typename ResultContainer, typename Container>
    ResultContainer without(Container const& container, typename Container::value_type const& value)
    {
        return filter<ResultContainer>(
            container, std::bind2nd(std::not_equal_to<typename Container::value_type>(), value));
    }


    template <typename ResultContainer, typename Container, typename Function>
    ResultContainer collect(Container container, Function function)
    {
        return map<ResultContainer>(container, function);
    }

    // sfink - values
    template <typename ResultContainer, typename Container>
    ResultContainer values(Container container)
    {
        ResultContainer result;
        // zorg c++11 optimisation
        for (auto& item : container) helper::add_to_container(result, item);
        //for (auto i = container.begin(); i != container.end(); ++i)
        //{
        //	helper::add_to_container(result, *i);
        //}
        return result;
    }

    // sfink - keys
    template <typename ResultContainer, typename Container>
    ResultContainer keys(Container container)
    {
        ResultContainer result;
		for (auto i = container.begin(); i != container.end(); ++i)
		{
			auto k = i->first;
			helper::add_to_container(result, k);
		}
        //for (auto i = container.begin(); i != container.end(); ++i)
        //{
        //    helper::add_to_container(result, i->key());
        //}
        return result;
    }

	// MDN - The slice() method returns a shallow copy of a portion of an array into a 
	// new array object selected from begin to end (end not included). 
	// The original array will not be modified.
    template <typename ResultContainer, typename Container>
    ResultContainer slice(Container container, long long begin = 0, long long end = 0)
    {
		//begin Optional
		//	Zero - based index at which to begin extraction.
		//	A negative index can be used, indicating an offset from the end of the sequence.slice(-2) extracts the last two elements in the sequence.
		//	If begin is undefined, slice begins from index 0.

		//end Optional
		//	Zero - based index before which to end extraction.slice extracts up to but not including end.
		//	For example, slice(1, 4) extracts the second element through the fourth element(elements indexed 1, 2, and 3).
		//	A negative index can be used, indicating an offset from the end of the sequence.slice(2, -1) extracts the third element through the second - to - last element in the sequence.
		//	If end is omitted, slice extracts through the end of the sequence(arr.length).
		//	If end is greater than the length of the sequence, slice extracts through the end of the sequence(arr.length).

		const size_t len = container.size();
		if (end < 1)
			end = len - end;

		if (begin < 0)
			begin = len - begin - 1;

		begin = helper::clamp<size_t>(begin, 0, len - 1);
		end   = helper::clamp<size_t>(end, 0, len);

        ResultContainer result;
		size_t _index = 0;
		for (auto i = container.begin(); i != container.end(); ++i)
		{
			auto index = _index++;
			if (index >= end)
				break;
			if (index >= begin)
				helper::add_to_container(result, *i);
		}
        return result;
    }

    // reduce/inject/foldl
    template <typename Container, typename Function, typename Memo>
    Memo reduce(const Container container, Function function, Memo memo)
    {
        for (auto i = container.begin(); i != container.end(); ++i)
        {
            memo = function(memo, *i);
        }
        return memo;
    }

    template <typename Container, typename Function, typename Memo>
    Memo inject(const Container container, Function function, Memo memo)
    {
        return reduce(container, function, memo);
    }

    template <typename Container, typename Function, typename Memo>
    Memo foldl(const Container container, Function function, Memo memo)
    {
        return reduce(container, function, memo);
    }

    // reduce_right/foldr
    template <typename Container, typename Function, typename Memo>
    Memo reduce_right(const Container container, Function function, Memo memo)
    {
        for (typename Container::const_reverse_iterator i = container.rbegin(); i != container.rend();
            ++i)
        {
            memo = function(memo, *i);
        }
        return memo;
    }

    template <typename Container, typename Function, typename Memo>
    Memo foldr(const Container container, Function function, Memo memo)
    {
        return reduce_right(container, function, memo);
    }

    // find/detect
    template <typename Container, typename Predicate>
    typename Container::iterator find(Container& container, Predicate predicate)
    {
        return find_if(container.begin(), container.end(), predicate);
    }

    template <typename Container, typename Predicate>
    typename Container::iterator detect(Container& container, Predicate predicate)
    {
        return find(container, predicate);
    }

    template <typename ResultContainer, typename Container, typename Predicate>
    ResultContainer select(Container container, Predicate predicate)
    {
        return filter<ResultContainer>(container, predicate);
    }

    // reject
    template <typename ResultContainer, typename Container, typename Predicate>
    ResultContainer reject(Container container, Predicate predicate)
    {
        ResultContainer result;
        for (auto i = container.begin(); i != container.end(); ++i)
        {
            if (!predicate(*i))
            {
                helper::add_to_container(result, *i);
            }
        }
        return result;
    }

    // all/every
    template <typename Container, typename Predicate>
    bool all(Container container, Predicate predicate)
    {
        for (auto i = container.begin(); i != container.end(); ++i)
        {
            if (!predicate(*i))
            {
                return false;
            }
        }
        return true;
    }

    template <typename Container, typename Predicate>
    bool every(Container container, Predicate predicate)
    {
        return all(container, predicate);
    }

    // any/some
    template <typename Container, typename Predicate>
    bool any(Container container, Predicate predicate)
    {
        for (auto i = container.begin(); i != container.end(); ++i)
        {
            if (predicate(*i))
            {
                return true;
            }
        }
        return false;
    }

    template <typename Container, typename Predicate>
    bool some(Container container, Predicate predicate)
    {
        return any(container, predicate);
    }

    // contains (alias includes)
    template <typename Container>
    bool contains(Container container, typename Container::value_type value)
    {
        return std::find(container.begin(), container.end(), value) != container.end();
    }

    template <typename Container>
    bool includes(Container container, typename Container::value_type value)
    {
        return contains(container, value);
    }

    // invoke
    template <typename ResultContainer, typename Container, typename Function>
    typename helper::enable_if<!helper::is_void<ResultContainer>::value, ResultContainer>::type invoke(
        Container container, Function function)
    {
        ResultContainer result;
        for (typename Container::iterator i = container.begin(); i != container.end(); ++i)
        {
            helper::add_to_container(result, (*i.*function)());
        }
        return result;
    }

    template <typename ResultContainer, typename Container, typename Function>
    typename helper::enable_if<helper::is_void<ResultContainer>::value, void>::type invoke(
        Container container, Function function)
    {
        for (typename Container::iterator i = container.begin(); i != container.end(); ++i)
        {
            (*i.*function)();
        }
    }

    // pluck
    // Called like `_::pluck<vector<int>>(container, &value_type::member)`
    template <typename ResultContainer, typename Container, typename Member>
    ResultContainer pluck(Container const& container, Member member)
    {
        ResultContainer result;
        for (auto i = container.begin(); i != container.end(); ++i)
        {
            helper::add_to_container(result, *i.*member);
        }
        return result;
    }

    // max
    template <typename Container>
    typename Container::iterator max(Container container)
    {
        if (container.begin() == container.end())
        {
            return container.end();
        }

        typename Container::iterator max = container.begin();
        for (typename Container::iterator i = ++container.begin(); i != container.end(); ++i)
        {
            if (*max < *i)
            {
                max = i;
            }
        }
        return max;
    }

    template <typename Compared, typename Container, typename Function>
    typename Container::iterator max(Container container, Function function)
    {
        if (container.begin() == container.end())
        {
            return container.end();
        }

        struct
        {
            typename Container::iterator position;
            Compared computed;
        } max = { container.begin(), function(*container.begin()) };

        for (typename Container::iterator i = ++container.begin(); i != container.end(); ++i)
        {
            Compared computed = function(*i);
            if (max.computed < computed)
            {
                max.position = i;
                max.computed = computed;
            }
        }
        return max.position;
    }

    // min
    template <typename Container>
    typename Container::iterator min(Container container)
    {
        if (container.begin() == container.end())
        {
            return container.end();
        }

        typename Container::iterator min = container.begin();
        for (typename Container::iterator i = ++container.begin(); i != container.end(); ++i)
        {
            if (*i < *min)
            {
                min = i;
            }
        }
        return min;
    }

    template <typename Compared, typename Container, typename Function>
    typename Container::iterator min(Container container, Function function)
    {
        if (container.begin() == container.end())
        {
            return container.end();
        }

        struct
        {
            typename Container::iterator position;
            Compared computed;
        } min = { container.begin(), function(*container.begin()) };

        for (typename Container::iterator i = ++container.begin(); i != container.end(); ++i)
        {
            Compared computed = function(*i);
            if (computed < min.computed)
            {
                min.position = i;
                min.computed = computed;
            }
        }
        return min.position;
    }

    // sort_by
    template <typename Container, typename Function>
    Container sort_by(Container container, Function function)
    {
        std::vector<typename Container::value_type> to_sort(container.begin(), container.end());
        std::sort(to_sort.begin(), to_sort.end(), function);
        return Container(to_sort.begin(), to_sort.end());
    }

    // group_by
    template <typename Key, typename Container, typename Function>
    std::multimap<Key, typename Container::value_type> group_by(Container container, Function function)
    {
        std::multimap<Key, typename Container::value_type> result;
        for (typename Container::iterator i = container.begin(); i != container.end(); ++i)
        {
            result.insert(std::pair<Key, typename Container::value_type>(function(*i), *i));
        }
        return result;
    }

    // sorted_index
    template <typename Container>
    typename Container::iterator sorted_index(
        Container container, typename Container::value_type const& value)
    {
        return std::upper_bound(container.begin(), container.end(), value);
    }

    namespace helper
    {
        template <typename Argument, typename Function>
        class TransformCompare
#if _HAS_CXX17 == 0
			: std::binary_function<Argument, Argument, bool>
#endif
        {
        public:
            TransformCompare(Function const& function)
                : function_(function)
            {
            }

            bool operator()(Argument const& left, Argument const& right) const
            {
                return function_(left) < function_(right);
            }

        private:
            Function function_;
        };
    } // namespace helper

    template <typename Container, typename Function>
    typename Container::iterator sorted_index(
        Container container, typename Container::value_type const& value, Function function)
    {
        return std::upper_bound(container.begin(), container.end(), value,
            helper::TransformCompare<typename Container::value_type, Function>(function));
    }

    // shuffle
    // This assumes srand has already been called.
    template <typename ResultContainer, typename Container>
    ResultContainer shuffle(Container container)
    {
        std::vector<typename Container::value_type> deck(container.begin(), container.end());
        for (int i = deck.size() - 1; i > 1; --i)
        {
            int j = std::rand() % (i + 1);
            std::swap(deck[i], deck[j]);
        }
        return ResultContainer(deck.begin(), deck.end());
    }

    // to_array
    template <typename Container>
    typename Container::value_type* to_array(Container container)
    {
        typename Container::value_type* array = new typename Container::value_type[container.size()];
        struct
        {
            int numeric;
            typename Container::iterator iterator;
        } i;
        for (i.numeric = 0, i.iterator = container.begin(); i.iterator != container.end();
            ++i.numeric, ++i.iterator)
        {
            array[i.numeric] = *i.iterator;
        }

        return array;
    }

    // size
    template <typename Container>
    int size(Container container)
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

    // compact
    template <typename ResultContainer, typename Container>
    ResultContainer compact(Container const& container)
    {
        ResultContainer result;
        for (auto i = container.begin(); i != container.end(); ++i)
        {
            if (static_cast<bool>(*i))
            {
                helper::add_to_container(result, *i);
            }
        }
        return result;
    }

    // flatten
    namespace helper
    {
        template <typename ResultContainer, typename Container>
        ResultContainer flatten_one_layer(Container const& container)
        {
            ResultContainer result;
            for (auto i = container.begin(); i != container.end(); ++i)
            {
                for (typename Container::value_type::const_iterator j = i->begin(); j != i->end(); ++j)
                {
                    add_to_container(result, *j);
                }
            }
            return result;
        }

        template <typename T>
        class HasConstIterator
        {
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
        typename enable_if<!HasConstIterator<typename Container::value_type>::value, void>::type
            flatten_loop(ResultContainer& result, Container const& container)
        {
            for (auto i = container.begin(); i != container.end(); ++i)
            {
                add_to_container(result, *i);
            }
        }

        template <typename ResultContainer, typename Container>
        typename enable_if<HasConstIterator<typename Container::value_type>::value, void>::type
            flatten_loop(ResultContainer& result, Container const& container)
        {
            for (auto i = container.begin(); i != container.end(); ++i)
            {
                flatten_loop(result, *i);
            }
        }

    } // namespace helper

    template <typename ResultContainer, typename Container>
    ResultContainer flatten(Container const& container)
    {
        ResultContainer result;
        helper::flatten_loop(result, container);
        return result;
    }

    template <typename ResultContainer, bool shallow, typename Container>
    typename helper::enable_if<shallow == true, ResultContainer>::type flatten(
        Container const& container)
    {
        return helper::flatten_one_layer<ResultContainer>(container);
    }

    template <typename ResultContainer, bool shallow, typename Container>
    typename helper::enable_if<shallow == false, ResultContainer>::type flatten(
        Container const& container)
    {
        return flatten<ResultContainer>(container);
    }

    // uniq/unique
    template <typename ResultContainer, typename Key, typename Container, typename Function>
    ResultContainer uniq(Container const& container, bool is_sorted, Function function)
    {
        ResultContainer result;
        std::vector<Key> keys = map<std::vector<Key>>(container, function);
        if (container.size() < 3)
        {
            is_sorted = true;
        }

        std::vector<Key> memo;
        for (std::pair<typename std::vector<Key>::const_iterator, typename Container::const_iterator> i = std::make_pair(keys.begin(), container.begin());
            i.first != keys.end(); ++i.first, ++i.second)
        {
            if (is_sorted ? !memo.size() || *last(memo) != *i.first : !include(memo, *i.first))
            {
                memo.push_back(*i.first);
                helper::add_to_container(result, *i.second);
            }
        }
        return result;
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
        if (container.size() < 3)
        {
            is_sorted = true;
        }

        std::vector<typename Container::value_type> memo;
        for (auto i = container.begin(); i != container.end(); ++i)
        {
            if (is_sorted ? !memo.size() || *last(memo) != *i : !include(memo, *i))
            {
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

    // union_of
    template <typename ResultContainer, typename Container1, typename Container2>
    ResultContainer union_of(Container1 const& container1, Container2 const& container2)
    {
        std::vector<typename ResultContainer::value_type> left(container1.begin(), container1.end());
        std::vector<typename ResultContainer::value_type> right(container2.begin(), container2.end());
        std::sort(left.begin(), left.end());
        std::sort(right.begin(), right.end());

        std::vector<typename ResultContainer::value_type> union_result;
        std::set_union(
            left.begin(), left.end(), right.begin(), right.end(), std::back_inserter(union_result));
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
        std::set_intersection(
            left.begin(), left.end(), right.begin(), right.end(), std::back_inserter(union_result));
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
        std::set_difference(
            left.begin(), left.end(), right.begin(), right.end(), std::back_inserter(union_result));
        return ResultContainer(union_result.begin(), union_result.end());
    }

    // difference2 - because `difference` doesn't work for all types
    template <typename ResultContainer, typename Container1, typename Container2>
    ResultContainer difference2(Container1 const& container1, Container2 const& container2)
    {
		return filter<ResultContainer>(container1, [&](const auto& value) {
				return !contains(container2, value);
		});
    }

    // zip
    template <typename ResultContainer, typename Container1, typename Container2>
    ResultContainer zip(const Container1& container1, const Container2& container2)
    {
        ResultContainer result;
        typename Container1::const_iterator left = container1.begin();
        typename Container2::const_iterator right = container2.begin();
        while (left != container1.end() && right != container2.end())
        {
            helper::add_to_container(result, typename ResultContainer::value_type(*left++, *right++));
        }
        return result;
    }

    // indexOf
    template <typename Container>
    int indexOf(Container& container, typename Container::value_type value)
    {
        auto value_position = std::find(container.begin(), container.end(), value);
        return value_position == container.end() ? -1
            : std::distance(container.begin(), value_position);
    }

	// `indexOf` that accepts `Container::value_type = std::pair<K, V>`
    template <typename Container, typename Value>
    int indexOfMap(Container& container, Value value)
    {
		// https://stackoverflow.com/questions/12742472/how-to-get-matching-key-using-the-value-in-a-map-c

		auto value_position = std::find_if(std::begin(container), std::end(container), [&](const auto& pair)
		{
			return pair.second == value;
		});

        return value_position == container.end() ? -1
            : std::distance(container.begin(), value_position);
    }


	// `contains` that accepts `Container::value_type = std::pair<K, V>`
    template <typename Container, typename Value>
    bool containsMap(Container container, Value value)
    {
		return indexOfMap(container, value) != -1;
    }

    template <typename Container>
    int indexOf(Container& container, typename Container::value_type value, bool is_sorted)
    {
        if (!is_sorted)
        {
            return indexOf(container, value);
        }
        typename Container::iterator value_lower_bound = std::lower_bound(container.begin(), container.end(), value);
        return value_lower_bound == container.end() || *value_lower_bound != value
            ? -1
            : std::distance(container.begin(), value_lower_bound);
    }

    // last_index_of
    template <typename Container>
    int last_index_of(Container const& container, typename Container::value_type value)
    {
        typename Container::const_iterator result = std::find(container.begin(), container.end(), value);
        typename Container::const_iterator i = result;
        while (i != container.end())
        {
            i = std::find(++i, container.end(), value);
            if (i != container.end())
            {
                result = i;
            }
        }
        return result == container.end() ? -1 : std::distance(container.begin(), result);
    }

    // range
    template <typename ResultContainer>
    ResultContainer range(int start, int stop, int step)
    {
        int length = std::max((stop - start) / step, 0);
        int index = 0;
        ResultContainer result;

        while (index < length)
        {
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

#ifdef UNDERSCORE_BONUS
	// at
	template <typename Container>
	auto& at(Container& container, typename Container::key_type key)
	{
		return container.at(key);
	}

	template <typename Container>
	auto tryAndGet(Container container, const typename Container::key_type key, typename Container::value_type& value) {
		if (contains(container, key)) {
			value = at(container, key);
			return true;
		}
		return false;
	}

	template <typename Container>
	auto& getOrCall(Container container, const typename Container::key_type key, typename Container::value_type(*function)(typename Container::key_type)) {
		if (!contains(container, key))
			helper::add_to_container(container, key, function(key));
		return at(container, key);
	}

	template <typename Container>
	auto& getOrDefault(Container container, const typename Container::key_type key, const typename Container::value_type& value) {
		if (!contains(container, key))
			helper::add_to_container(container, key, value);
		return at(container, key);
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
        return Wrapper<Container>(container);
    }

    // value
    template <typename Container>
    typename Container::value_type value(Wrapper<Container>& wrapper)
    {
        return wrapper.value();
    }

    template <typename Container>
    class Wrapper
    {
    public:
        typedef Container value_type;
        Wrapper(Container container)
            : container_(container)
        {
        }

        Container value()
        {
            return container_;
        }

        template <typename Function>
        Wrapper& each(Function function)
        {
            _::each(container_, function);
            return *this;
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


    private:
        Container container_;
    };

} // namespace _

  // namespace _ = _;

#endif // UNDERSCORE_UNDERSCORE_H_
