#ifndef VIF_CORE_META_HPP
#define VIF_CORE_META_HPP

#include <type_traits>
#include <array>
#include <utility>
#include <cassert>

namespace vif {
namespace impl {
    namespace meta_impl {
        template<typename L1, typename L2>
        struct reverse_types_;

        template<template<typename ...> class wrapper, typename L2>
        struct reverse_types_<wrapper<>, L2> {
            using type = L2;
        };

        template<template<typename ...> class wrapper, typename ... Args1, typename T, typename L2>
        struct reverse_types_<wrapper<T, Args1...>, L2> :
            reverse_types_<wrapper<Args1...>, typename L2::template prepend<T>> {};

        template<typename TV, typename L1, typename L2>
        struct reverse_values_;

        template<typename TV, template<typename, TV ...> class wrapper, typename L2>
        struct reverse_values_<TV, wrapper<TV>, L2> {
            using type = L2;
        };

        template<typename TV, template<typename, TV ...> class wrapper, TV ... Args1, TV T, typename L2>
        struct reverse_values_<TV, wrapper<TV, T, Args1...>, L2> :
            reverse_values_<TV, wrapper<TV, Args1...>, typename L2::template prepend<T>> {};
    }
}

namespace meta {
    // Simple type list
    template<typename ... Args>
    struct type_list {
        template<typename T>
        using append = type_list<Args..., T>;

        template<typename T>
        using prepend = type_list<T, Args...>;

        using empty = type_list<>;

        using reverse = typename impl::meta_impl::reverse_types_<type_list<Args...>, empty>::type;

        static const std::size_t size = sizeof...(Args);
    };

    // Simple value list
    // C++14: replace with std::integer_sequence<T, V...>
    template<typename T, T ... V>
    struct value_list {
        template<T v>
        using append = value_list<T, V..., v>;

        template<T v>
        using prepend = value_list<T, v, V...>;

        using empty = value_list<T>;

        using type = T;

        using reverse = typename impl::meta_impl::reverse_values_<T, value_list<T, V...>, empty>::type;

        static const std::size_t size = sizeof...(V);
    };

    // Simple boolean list
    template<bool ... B>
    using bool_list = value_list<bool, B...>;

    // Boolean constant
    // C++17: replace with std::bool_constant<B>
    template<bool B>
    using bool_constant = std::integral_constant<bool, B>;

    // Class holding an integer value.
    template<std::size_t I>
    struct cte_t {
        static const std::size_t value = I;
    };
}

namespace impl {
    namespace meta_impl {
        // Return a particular element of a variadic list
        template<std::size_t N, typename T, typename ... Args>
        auto get_(meta::cte_t<N>, meta::cte_t<N>, T&& t, Args&& ... args) -> decltype(std::forward<T>(t)) {
            return std::forward<T>(t);
        }

        template<std::size_t I, std::size_t N, typename T, typename ... Args>
        auto get_(meta::cte_t<I>, meta::cte_t<N>, T&& t, Args&& ... args) ->
            decltype(get_(meta::cte_t<I+1>(), meta::cte_t<N>(), std::forward<Args>(args)...)) {
            return get_(meta::cte_t<I+1>(), meta::cte_t<N>(), std::forward<Args>(args)...);
        }
    }
}

namespace meta {
    // Return the element N of a parameter list
    template<std::size_t N, typename ... Args>
    auto get(Args&& ... args) ->
        decltype(impl::meta_impl::get_(cte_t<0>(), cte_t<N>(), std::forward<Args>(args)...)) {
        return impl::meta_impl::get_(cte_t<0>(), cte_t<N>(), std::forward<Args>(args)...);
    }

    // Shortcut for std::decay
    // C++14: replace with std::decay_t<T>
    template<typename T>
    using decay_t = typename std::decay<T>::type;

    // Class holding a sequence of integer values.
    // C++14: replace with std::index_sequence<...>
    template<std::size_t ...>
    struct seq_t {};

    // Generate a sequence of integer values from N to D (inclusive).
    template<std::size_t N, std::size_t D, std::size_t ... S>
    struct gen_seq : public gen_seq<N+1, D, S..., N> {};

    template<std::size_t D, std::size_t ... S>
    struct gen_seq<D, D, S...> {
      using type = seq_t<S..., D>;
    };

    // Generate a sequence of integer from 0 to D (exclusive).
    // C++14: replace with std::make_index_sequence<N>
    template<std::size_t D>
    using gen_seq_t = typename gen_seq<0, D-1>::type;

    // Obtain the return type of a function/functor
    template<typename T>
    struct return_type {
        using type = typename return_type<decltype(&T::operator())>::type;
    };

    template<typename R, typename ... Args>
    struct return_type<R (*)(Args...)> {
        using type = R;
    };

    template<typename R, typename T, typename ... Args>
    struct return_type<R (T::*)(Args...)> {
        using type = R;
    };

    template<typename R, typename T, typename ... Args>
    struct return_type<R (T::*)(Args...) const> {
        using type = R;
    };

    // Create a D-dimensional nested initializer_list<T> type
    template<std::size_t D, typename T>
    struct make_nested_initializer_list {
        using type = typename make_nested_initializer_list<D-1,std::initializer_list<T>>::type;
    };

    template<typename T>
    struct make_nested_initializer_list<0,T> {
        using type = T;
    };

    template<std::size_t D, typename T>
    using nested_initializer_list = typename make_nested_initializer_list<D,T>::type;

    // Transfer constness from C to T
    template<typename T, typename C>
    using constify = typename std::conditional<std::is_const<C>::value, const T, T>::type;

    // Shortcut for const_cast that doesn't require spelling the type
    template<typename T>
    T& remove_const(const T& t) {
        return const_cast<T&>(t);
    }

    template<typename T>
    T* remove_const(const T* t) {
        return const_cast<T*>(t);
    }

    // Shortcut for const_cast that doesn't require spelling the type
    template<typename T>
    const T& add_const(T& t) {
        return const_cast<const T&>(t);
    }

    template<typename T>
    const T* add_const(T* t) {
        return const_cast<const T*>(t);
    }

    // Get the size of an std::array
    template<typename T>
    struct array_size;

    template<std::size_t N, typename T>
    struct array_size<std::array<T,N>> {
        static const std::size_t size = N;
    };
}

    //////////////////////////////////////
    //
    // first_type<List>
    //
    // Return the first type in 'List=type_list<...>'. Undefined if the list is empty.
    //
    // Examples:
    // first_type<type_list<int, float, int, char, int, double>>
    // -> int
    //
    //////////////////////////////////////

namespace impl {
    namespace meta_impl {
        template<typename List>
        struct first_type_t;

        template<typename T, typename ... Args>
        struct first_type_t<meta::type_list<T, Args...>> {
            using type = T;
        };
    }
}

namespace meta {
    template<typename List>
    using first_type = typename impl::meta_impl::first_type_t<List>::type;
}

    //////////////////////////////////////
    //
    // filter_type_list<Filter, List>
    //
    // Filter a type list 'List=type_list<...>' according to a boolean pattern 'Filter=bool_list<...>'.
    // The pattern can be of any length, except zero. If it is shorter than the length of the type list,
    // it will repeat itself from the beginning. The type list can be empty.
    //
    // The returned type is a new type list containing only the types that were filtered.
    //
    // Examples:
    // filter_type_list<type_list<int, float, int, char, int, double>, bool_list<true, false>>
    // -> type_list<int, int, int>
    //
    // filter_type_list<type_list<int, float, int, char, int, double>, bool_list<false, true>>
    // -> type_list<float, char, double>
    //
    //////////////////////////////////////

namespace impl {
    namespace meta_impl {
        template<typename Filter, typename RunningFilter, typename List, typename Append = meta::type_list<>>
        struct filter_type_list_t;

        template<bool ... F, bool ... RF, typename ... Filtered>
        struct filter_type_list_t<
            meta::bool_list<F...>,
            meta::bool_list<RF...>,
            meta::type_list<>,
            meta::type_list<Filtered...>
            > {
            using type = meta::type_list<Filtered...>;
        };

        template<bool ... F, typename T, typename ... Args, typename ... Filtered>
        struct filter_type_list_t<
            meta::bool_list<F...>,
            meta::bool_list<>,
            meta::type_list<T, Args...>,
            meta::type_list<Filtered...>
            > {
            using type = typename filter_type_list_t<
                meta::bool_list<F...>,
                meta::bool_list<F...>,
                meta::type_list<T, Args...>,
                meta::type_list<Filtered...>
            >::type;
        };

        template<bool ... F, bool ... RF, typename A1, typename ... Args, typename ... Filtered>
        struct filter_type_list_t<
            meta::bool_list<F...>,
            meta::bool_list<true, RF...>,
            meta::type_list<A1, Args...>,
            meta::type_list<Filtered...>
            > {
            using type = typename filter_type_list_t<
                meta::bool_list<F...>,
                meta::bool_list<RF...>,
                meta::type_list<Args...>,
                meta::type_list<Filtered..., A1>
            >::type;
        };

        template<bool ... F, bool ... RF, typename A1, typename ... Args, typename ... Filtered>
        struct filter_type_list_t<
            meta::bool_list<F...>,
            meta::bool_list<false, RF...>,
            meta::type_list<A1, Args...>,
            meta::type_list<Filtered...>
            > {
            using type = typename filter_type_list_t<
                meta::bool_list<F...>,
                meta::bool_list<RF...>,
                meta::type_list<Args...>,
                meta::type_list<Filtered...>
            >::type;
        };
    }
}

namespace meta {
    template<typename Filter, typename List>
    using filter_type_list =
        typename impl::meta_impl::filter_type_list_t<Filter, Filter, List>::type;

    //////////////////////////////////////
    //
    // are_all_true<List>
    //
    // Return a bool constant that is 'true' if all the booleans in 'List=bool_list<...>' are themselves
    // equal to 'true'. Returns 'true' for an empty list.
    //
    // Examples:
    // are_all_true<bool_list<true, true, true>>
    // -> bool_constant<true>
    //
    // are_all_true<bool_list<true, false, true>>
    // -> bool_constant<false>
    //
    //////////////////////////////////////

    template<typename T>
    struct are_all_true;

    template<>
    struct are_all_true<bool_list<>> : std::true_type {};

    template<bool B1, bool ... B>
    struct are_all_true<bool_list<B1, B...>> :
        bool_constant<B1 && are_all_true<bool_list<B...>>::value> {};

    //////////////////////////////////////
    //
    // are_all_false<List>
    //
    // Return a bool constant that is 'true' if all the booleans in 'List=bool_list<...>' are themselves
    // equal to 'false'. Returns 'true' for an empty list.
    //
    // Examples:
    // are_all_false<bool_list<false, false, false>>
    // -> bool_constant<true>
    //
    // are_all_false<bool_list<true, false, true>>
    // -> bool_constant<false>
    //
    //////////////////////////////////////

    template<typename T>
    struct are_all_false;

    template<>
    struct are_all_false<bool_list<>> : std::true_type {};

    template<bool B1, bool ... B>
    struct are_all_false<bool_list<B1, B...>> :
        bool_constant<!B1 && are_all_false<bool_list<B...>>::value> {};

    //////////////////////////////////////
    //
    // are_any_false<List>
    //
    // Return a bool constant that is 'true' if at least one boolean in 'List=bool_list<...>' is itself
    // equal to 'false'. Returns 'false' for an empty list. This is the strict negation of
    // are_all_true<List>.
    //
    // Examples:
    // are_any_false<bool_list<true, true, true>>
    // -> bool_constant<false>
    //
    // are_any_false<bool_list<true, false, true>>
    // -> bool_constant<true>
    //
    //////////////////////////////////////

    template<typename List>
    struct are_any_false : bool_constant<!are_all_true<List>::value> {};

    //////////////////////////////////////
    //
    // are_any_true<List>
    //
    // Return a bool constant that is 'true' if at least one boolean in 'List=bool_list<...>' is itself
    // equal to 'true'. Returns 'false' for an empty list. This is the strict negation of
    // are_all_false<List>.
    //
    // Examples:
    // are_any_true<bool_list<false, false, false>>
    // -> bool_constant<false>
    //
    // are_any_true<bool_list<true, false, true>>
    // -> bool_constant<true>
    //
    //////////////////////////////////////

    template<typename List>
    struct are_any_true : bool_constant<!are_all_false<List>::value> {};

    //////////////////////////////////////
    //
    // first_true<List>
    //
    // Return an integer constant giving the position of the first 'true' value in
    // the list 'List=bool_list<...>'. If no value is 'true', returns 'N' for a list
    // of length 'N'.
    //
    // Examples:
    // first_true<bool_list<false, false, false>>
    // -> cte_t<3>
    //
    // first_true<bool_list<true, false, true>>
    // -> cte_t<0>
    //
    //////////////////////////////////////

    template<typename List>
    struct first_true;

    template<>
    struct first_true<bool_list<>> : cte_t<1> {};

    template<bool B1, bool ... B>
    struct first_true<bool_list<B1, B...>> :
        cte_t<B1 ? 0 : 1 + first_true<bool_list<B...>>::value> {};

    //////////////////////////////////////
    //
    // last_true<List>
    //
    // Return an integer constant giving the position of the last 'true' value in
    // the list 'List=bool_list<...>'. If no value is 'true', returns 'N' for a list
    // of length 'N'.
    //
    // Examples:
    // last_true<bool_list<false, false, false>>
    // -> cte_t<3>
    //
    // last_true<bool_list<true, false, true>>
    // -> cte_t<2>
    //
    //////////////////////////////////////

    template<typename List>
    struct last_true {
        static const std::size_t tmp = first_true<typename List::reverse>::value;
        static const std::size_t value = tmp == List::size ? tmp : List::size-1 - tmp;
    };

    //////////////////////////////////////
    //
    // first_false<List>
    //
    // Return an integer constant giving the position of the first 'false' value in
    // the list 'List=bool_list<...>'. If no value is 'false', returns 'N' for a list
    // of length 'N'.
    //
    // Examples:
    // first_false<bool_list<true, true, true>>
    // -> cte_t<3>
    //
    // first_false<bool_list<false, true, false>>
    // -> cte_t<0>
    //
    //////////////////////////////////////

    template<typename List>
    struct first_false;

    template<>
    struct first_false<bool_list<>> : cte_t<1> {};

    template<bool B1, bool ... B>
    struct first_false<bool_list<B1, B...>> :
        cte_t<B1 ? 1 + first_false<bool_list<B...>>::value : 0> {};

    //////////////////////////////////////
    //
    // last_false<List>
    //
    // Return an integer constant giving the position of the last 'false' value in
    // the list 'List=bool_list<...>'. If no value is 'false', returns 'N' for a list
    // of length 'N'.
    //
    // Examples:
    // last_false<bool_list<true, true, true>>
    // -> cte_t<3>
    //
    // last_false<bool_list<false, true, false>>
    // -> cte_t<2>
    //
    //////////////////////////////////////

    template<typename List>
    struct last_false {
        static const std::size_t tmp = first_false<typename List::reverse>::value;
        static const std::size_t value = tmp == List::size ? tmp : List::size-1 - tmp;
    };

    //////////////////////////////////////
    //
    // count<List>
    //
    // Return an integer constant holding the number of 'true' values in a boolean list
    //
    // Examples:
    // count<bool_list<true, true, true>>
    // -> cte_t<3>
    //
    // count<bool_list<true, false, true>>
    // -> cte_t<2>
    //
    //////////////////////////////////////

    template<typename T>
    struct count;

    template<>
    struct count<bool_list<>> : cte_t<0> {};

    template<bool B1, bool ... B>
    struct count<bool_list<B1, B...>> :
        cte_t<(B1 ? 1 : 0) + count<bool_list<B...>>::value> {};
}

    //////////////////////////////////////
    //
    // unary_apply_type_to_type_list<List, UnaryFunction>
    //
    // Applies the meta function 'UnaryFunction' to each type in 'List=type_list<T...>'.
    // The result is 'type_list<UnaryFunction<T>::type...>'.
    //
    // Examples:
    // unary_apply_type_to_type_list<type_list<int, double>, std::add_const>
    // -> type_list<const int, const double>
    //
    //////////////////////////////////////

namespace impl {
    namespace meta_impl {
        template<typename List, template<typename> class Function>
        struct unary_apply_type_to_type_list_t;

        template<typename ... Args, template<typename> class Function>
        struct unary_apply_type_to_type_list_t<meta::type_list<Args...>, Function> {
            using type = meta::type_list<typename Function<Args>::type...>;
        };
    }
}

namespace meta {
    template<typename List, template<typename> class Function>
    using unary_apply_type_to_type_list =
        typename impl::meta_impl::unary_apply_type_to_type_list_t<List, Function>::type;
}

    //////////////////////////////////////
    //
    // unary_apply_type_to_value_list<List, ValueType, UnaryFunction>
    //
    // Applies the meta function 'UnaryFunction' to each type in 'List=type_list<T...>'.
    // The result is 'value_list<ValueType, UnaryFunction<T>::value...>'.
    //
    // Examples:
    // unary_apply_type_to_value_list<type_list<int, const double>, bool, std::is_const>
    // -> bool_list<false, true>
    //
    //////////////////////////////////////

namespace impl {
    namespace meta_impl {
        template<typename List, typename T, template<typename> class Function>
        struct unary_apply_type_to_value_list_t;

        template<typename ... Args, typename T, template<typename> class Function>
        struct unary_apply_type_to_value_list_t<meta::type_list<Args...>, T, Function> {
            using type = meta::value_list<T, Function<Args>::value...>;
        };
    }
}

namespace meta {
    template<typename List, typename T, template<typename> class Function>
    using unary_apply_type_to_value_list =
        typename impl::meta_impl::unary_apply_type_to_value_list_t<List, T, Function>::type;

    template<typename List, template<typename> class Function>
    using unary_apply_type_to_bool_list =
        typename impl::meta_impl::unary_apply_type_to_value_list_t<List, bool, Function>::type;
}

    //////////////////////////////////////
    //
    // binary_second_apply_type_to_type_list<List, BinaryFunction, Arg1>
    //
    // Applies the meta function 'BinaryFunction<Arg1,.>' to each type in 'List=type_list<T...>'.
    // The result is 'type_list<BinaryFunction<Arg1,T>::type...>'.
    //
    // Examples:
    // template<typename T1, typename T2>
    // using largest_type = std::conditional<sizeof(T1) >= sizeof(T2), T1, T2>;
    // binary_second_apply_type_to_type_list<type_list<char, short, int, long>, largest_type, int>
    // -> type_list<int, int, int, long>
    //
    //////////////////////////////////////

namespace impl {
    namespace meta_impl {
        template<typename List, template<typename,typename> class Function, typename A1>
        struct binary_second_apply_type_to_type_list_t;

        template<typename ... Args, template<typename,typename> class Function, typename A1>
        struct binary_second_apply_type_to_type_list_t<meta::type_list<Args...>, Function, A1> {
            using type = meta::type_list<typename Function<A1,Args>::type...>;
        };
    }
}

namespace meta {
    template<typename List, template<typename,typename> class Function, typename A1>
    using binary_second_apply_type_to_type_list =
        typename impl::meta_impl::binary_second_apply_type_to_type_list_t<List, Function, A1>::type;
}

    //////////////////////////////////////
    //
    // binary_second_apply_type_to_value_list<List, ValueType, BinaryFunction, Arg1>
    //
    // Applies the meta function 'BinaryFunction<Arg1,.>' to each type in 'List=type_list<T...>'.
    // The result is 'value_list<ValueType, BinaryFunction<Arg1,T>::value...>'.
    //
    // Examples:
    // binary_second_apply_type_to_value_list<type_list<int, const double>, bool, std::is_same, int>
    // -> bool_list<true, false>
    //
    //////////////////////////////////////

namespace impl {
    namespace meta_impl {
        template<typename List, typename T, template<typename,typename> class Function, typename A1>
        struct binary_second_apply_type_to_value_list_t;

        template<typename ... Args, typename T, template<typename,typename> class Function, typename A1>
        struct binary_second_apply_type_to_value_list_t<meta::type_list<Args...>, T, Function, A1> {
            using type = meta::value_list<T, Function<A1,Args>::value...>;
        };
    }
}

namespace meta {
    template<typename List, typename T, template<typename,typename> class Function, typename A1>
    using binary_second_apply_type_to_value_list =
        typename impl::meta_impl::binary_second_apply_type_to_value_list_t<List, T, Function, A1>::type;

    template<typename List, template<typename,typename> class Function, typename A1>
    using binary_second_apply_type_to_bool_list =
        binary_second_apply_type_to_value_list<List, bool, Function, A1>;
}

    //////////////////////////////////////
    //
    // binary_first_apply_type_to_type_list<List, BinaryFunction, Arg2>
    //
    // Applies the meta function 'BinaryFunction<.,Arg2>' to each type in 'List=type_list<T...>'.
    // The result is 'type_list<BinaryFunction<T,Arg2>::type...>'.
    //
    // Examples:
    // template<typename T1, typename T2>
    // using largest_type = std::conditional<sizeof(T1) >= sizeof(T2), T1, T2>;
    // binary_first_apply_type_to_type_list<type_list<char, short, int, long>, largest_type, int>
    // -> type_list<int, int, int, long>
    //
    //////////////////////////////////////

namespace impl {
    namespace meta_impl {
        template<typename List, template<typename,typename> class Function, typename A2>
        struct binary_first_apply_type_to_type_list_t;

        template<typename ... Args, template<typename,typename> class Function, typename A2>
        struct binary_first_apply_type_to_type_list_t<meta::type_list<Args...>, Function, A2> {
            using type = meta::type_list<typename Function<Args,A2>::type...>;
        };
    }
}

namespace meta {
    template<typename List, template<typename,typename> class Function, typename A2>
    using binary_first_apply_type_to_type_list =
        typename impl::meta_impl::binary_first_apply_type_to_type_list_t<List, Function, A2>::type;
}

    //////////////////////////////////////
    //
    // binary_first_apply_type_to_value_list<List, ValueType, BinaryFunction, Arg1>
    //
    // Applies the meta function 'BinaryFunction<.,Arg2>' to each type in 'List=type_list<T...>'.
    // The result is 'value_list<ValueType, BinaryFunction<T,Arg2>::value...>'.
    //
    // Examples:
    // binary_first_apply_type_to_value_list<type_list<int, const double>, bool, std::is_same, int>
    // -> bool_list<true, false>
    //
    //////////////////////////////////////

namespace impl {
    namespace meta_impl {
        template<typename List, typename T, template<typename,typename> class Function, typename A2>
        struct binary_first_apply_type_to_value_list_t;

        template<typename ... Args, typename T, template<typename,typename> class Function, typename A2>
        struct binary_first_apply_type_to_value_list_t<meta::type_list<Args...>, T, Function, A2> {
            using type = meta::value_list<T, Function<Args,A2>::value...>;
        };
    }
}

namespace meta {
    template<typename List, typename T, template<typename,typename> class Function, typename A2>
    using binary_first_apply_type_to_value_list =
        typename impl::meta_impl::binary_first_apply_type_to_value_list_t<List, T, Function, A2>::type;

    template<typename List, template<typename,typename> class Function, typename A2>
    using binary_first_apply_type_to_bool_list =
        binary_first_apply_type_to_value_list<List, bool, Function, A2>;

    //////////////////////////////////////
    //
    // is_any_type_of<T,List>
    //
    // Return a bool constant that is 'true' if the type 'T' is found in 'List=type_list<...>', and
    // 'false' otherwise. Returns 'false' for an empty list.
    //
    // Examples:
    // is_any_type_of<int, type_list<float, int, double>>
    // -> bool_constant<true>
    //
    // is_any_type_of<int, type_list<float, char, double>>
    // -> bool_constant<false>
    //
    //////////////////////////////////////

    template<typename T, typename List>
    using is_any_type_of = are_any_true<binary_second_apply_type_to_bool_list<List, std::is_same, T>>;

    template <typename T, T ... Args>
    struct max;

    template <typename T>
    struct max<T> {
        static_assert(!std::is_same<T,T>::value, "cannot find the maximum of an empty list");
    };

    template <typename T, T V>
    struct max<T,V> : std::integral_constant<T,V> {};

    template <typename T, T V, T... Args>
    struct max<T,V,Args...> : std::integral_constant<T, (V > meta::max<T,Args...>::value ? V : meta::max<T,Args...>::value)> {};
}
}

#endif
