#ifndef INVOKE_INTSEQ_H
#define INVOKE_INTSEQ_H

#include <array>
#include <ranges>
#include <functional>



// Beginning of the function returning the number of all results
template <typename... Args>
struct returns_counter{
    static constexpr std::size_t size = 1;
};

// Function returning the count of all results for intiger_sequence
template <typename T, T... values, typename... Args>
struct returns_counter<std::integer_sequence<T, values...>, Args...> {
    static constexpr std::size_t size = 
        sizeof...(values) * returns_counter<std::remove_cvref_t<Args>...>::size;
};

// Function returning the count of all results for a regular argument
template <typename T, typename... Args>
struct returns_counter<T, Args...> {
    static constexpr std::size_t size = returns_counter<std::remove_cvref_t<Args>...>::size;
};

// Basis for recursive case generation
template <bool ret_is_void, bool ret_is_ref, typename RetT, size_t N,
    typename Function, typename... PrevT>
constexpr void _invoke_intseq(std::array<RetT, N>& res, std::size_t& index,
    Function&& f, std::tuple<PrevT ...>&& prev){

    if constexpr(ret_is_void){
        std::apply(std::forward<Function>(f), std::forward<decltype(prev)>(prev));
    }else if constexpr(ret_is_ref){
        res[index].emplace(
            std::reference_wrapper(std::apply(std::forward<Function>(f), 
            std::forward<decltype(prev)>(prev))));
        index++;
    }else{
        res[index].emplace(
            std::apply(std::forward<Function>(f), 
            std::forward<decltype(prev)>(prev)));
        index++;
    }
}

// Recursive case generation for standard parameter
template <bool ret_is_void, bool ret_is_ref, typename RetT, size_t N, 
    typename Function, typename... PrevT, typename CurrT, typename... NextT>
constexpr void _invoke_intseq(std::array<RetT, N>& res, std::size_t& index,
        Function&& f, std::tuple<PrevT...>&& prev, CurrT&& curr, NextT&& ... next){

    _invoke_intseq<ret_is_void, ret_is_ref>(
        res,
        index,
        std::forward<Function>(f),
        std::tuple_cat(
            std::forward<decltype(prev)>(prev), 
            std::forward_as_tuple(std::forward<CurrT>(curr))
        ),
        std::forward<NextT>(next) ...
    );
}

// Recursive case generation for ineger_sequence
template <bool ret_is_void, bool ret_is_ref, typename RetT, size_t N, 
    typename Function, typename... PrevT, typename IS_T, IS_T... t, typename... NextT>
constexpr void _invoke_intseq(std::array<RetT, N>& res, std::size_t& index, Function&& f, 
    std::tuple<PrevT...>&& prev, const std::integer_sequence<IS_T, t...>, NextT&& ... next){

    (_invoke_intseq<ret_is_void, ret_is_ref>(
        res,
        index,
        std::forward<Function>(f),
        std::tuple_cat(
            std::forward<decltype(prev)>(prev), 
            std::forward_as_tuple(
                std::forward<std::integral_constant<IS_T, t>>(std::integral_constant<IS_T, t>{}
            ))
        ),
        std::forward<NextT>(next) ...
    ), ...);
}


// Returns the appropriate result table for the given data
template <typename T, std::size_t Size>
constexpr auto array_creator(){
    constexpr bool ret_is_void = std::is_same_v<T, void>; 
    constexpr bool ret_is_ref = std::is_reference_v<T>;
    if constexpr (ret_is_void){
        return std::array<int, 0>();
    } else if constexpr(ret_is_ref){
        return std::array<std::optional<std::reference_wrapper<std::remove_reference_t<T>>>, Size>();
    }else{
        return std::array<std::optional<T>, Size>();
    }
};

// Convert std::array<std::optional<T>,S> to std::array<T,S>
template <typename T, std::size_t N>
constexpr auto optional_array_to_regular_array(const std::array<T, N>& arr) {
    return std::apply([](const auto&... args) {
        return std::array<typename T::value_type, N>{args.value()...};
    }, arr);
}

// Converts integer_sequence to integer
template <class... Args>
struct remove_integer_sequence;

// T_O - original type
template <typename T_O, typename T, T... values>
struct remove_integer_sequence<T_O, std::integer_sequence<T, values...>> {
    using type = std::integral_constant<T,0>;
};

// T_O - original type
template <typename T_O, typename T>
struct remove_integer_sequence<T_O, T> {
    using type = T_O;
};

// Function that checks whether a given parameter is integer_sequence
template <typename T>
struct is_integer_sequence : std::false_type {};

template <typename T, T... Values>
struct is_integer_sequence<std::integer_sequence<T, Values...>> : std::true_type {};


// Function that checks whether a given parameter is an empty integer_sequence
template <typename T>
struct is_empty_integer_sequence : std::false_type {};

template <typename T>
struct is_empty_integer_sequence<std::integer_sequence<T>> : std::true_type {};


// Main function
template <class Function, class... Args>
constexpr decltype(auto) invoke_intseq(Function&& f, Args&& ... args){
    using ret_type = std::invoke_result_t<
        Function, 
        typename remove_integer_sequence<Args, std::remove_cvref_t<Args>>::type ...
    >;
    constexpr bool ret_is_void = std::is_same_v<ret_type, void>; 
    constexpr bool ret_is_ref = std::is_reference_v<ret_type>;
    constexpr bool param_cont_int_seq =    
        (is_integer_sequence<std::remove_cvref_t<Args>>::value || ...);
    constexpr bool param_cont_empty_int_seq = 
        (is_empty_integer_sequence<std::remove_cvref_t<Args>>::value || ...);
    
    if constexpr (param_cont_empty_int_seq){
        if constexpr(ret_is_ref){
            return std::array<std::reference_wrapper<ret_type>, 0>();
        }else{
            return std::array<ret_type, 0>();
        }
    }else if constexpr(!param_cont_int_seq) {
        return std::invoke(std::forward<Function>(f), std::forward<Args>(args) ...);
    }else{
        constexpr size_t ret_size = 
            ret_is_void ? 0 : returns_counter<std::remove_cvref_t<Args> ...>::size;

        auto arr = array_creator<ret_type, ret_size>();
        std::size_t index = 0;
        
        _invoke_intseq<ret_is_void, ret_is_ref>(
            arr, index,
            std::forward<Function>(f), 
            std::make_tuple(), 
            std::forward<Args>(args) ...
        );

        if constexpr (ret_is_void){
            return;
        }else{
            return optional_array_to_regular_array(arr);
        }
    }
    
}

#endif
