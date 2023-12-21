
# invoke_intseq README

This C++20 project aims to implement a generalized version of __std::invoke__ that handles arguments of type __std::integer_sequence__ in a special way. The primary goal is to provide a mechanism for invoking a callable object with all possible combinations of values encoded in __std::integer_sequence__.

# Implementation Details
## Header File: invoke_intseq.h
The header file __invoke_intseq.h__ contains the implementation of the __invoke_intseq__ function template. The template takes a callable object __F__ that represent function and arguments __Args__. The behavior of the function is defined as follows:

1. If none of the arguments is of type __std::integer_sequence__, the invocation of __invoke_intseq(f, args...)__ should have the same effect as __std::invoke(f, args...)__, equivalent to calling __f(args...)__.

2. If one or more arguments are of type __std::integer_sequence__, the function should invoke f for all possible combinations of elements encoded in the __std::integer_sequence arguments__. The result of the main invocation is:

    1. __void__ if the return type of __f__ is __void__.

    2. A type satisfying the __std::ranges::range__ concept if __f__ returns a type over which we can iterate.
   
## Recursive Invocation
For each argument of type __std::integer_sequence<T, j_1, j_2, ..., j_m>__, the invocation involves recursively calling __invoke_intseq__ with different values of __j_i__. This leads to a sequence of recursive calls, exploring all possible combinations.

## Additional Features
- __Perfect Forwarding__: The implementation use perfect forwarding to avoid unnecessary copies of arguments passed by reference (lvalues) and take ownership of arguments passed by value (rvalues).

- __Constexpr__: The __invoke_intseq__ function is constexpr, allowing it to be evaluated at compile-time if all arguments (__f__ and __args...__) are constexpr.

## Example Usage
```cpp
#include "invoke_intseq.h"

int main() {
    auto func = [](int a, char b, double c) {
        // Function implementation
    };

    invoke_intseq(func, 1, std::integer_sequence<int, 2, 3>, 'a', std::integer_sequence<double, 4.5, 6.7>);
    
    return 0;
}
```

In this example, __invoke_intseq__ is used to invoke the __func__ lambda with all possible combinations of values specified in the __std::integer_sequence arguments__.

## Build and Test
To build and test the implementation, include the __invoke_intseq.h__ header in your project and use it as described in the example above. Ensure that your project is configured to use C++20 features.

## Notes
This README provides a high-level overview of the __invoke_intseq__ implementation. For a detailed understanding, refer to the comments within the __invoke_intseq.h__ file.
