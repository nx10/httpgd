## Test environments
- GitHub rlib/actions
- R-hub builder

## R CMD check results
There were no ERRORs or WARNINGs. 

There was 1 NOTE:

* checking C++ specification ... NOTE
    Specified C++14: please drop specification unless essential

C++14 is essential.

## Downstream dependencies
There are no downstream dependencies.

## httpgd

We are splitting the `httpgd` package up into this package and the `unigd` package.
`unigd` has all the plotting and rendering code, and the `httpgd` package has the web-service code.

`unigd` was published on CRAN on 2024-01-25.

## I recently received the following email from CRAN:

> I just tried the effect of checking the submission on a Debian testing
> system
> with the current LLVM 18 compilers and clang++-18 using the libc++
> stdlib.
> For this, compilation fails with
> 
> * installing *source* package ‘httpgd’ ...
> ** using staged installation
> ** libs
> using C++ compiler: ‘Debian clang version 18.1.0 (rc2-3)’
> using C++14
> rm -f httpgd.so cpp11.o httpgd.o httpgd_rng.o httpgd_webserver.o
> unigd_impl.o
> clang++-18 -stdlib=libc++  -std=gnu++14
> -I"/home/hornik/tmp/R-d-clang-18-libcxx/include" -DNDEBUG -Ilib
> -DFMT_HEADER_ONLY
> -I'/home/hornik/lib/R/Library/4.4/x86_64-linux-gnu/cpp11/include'
> -I'/home/hornik/lib/R/Library/4.4/x86_64-linux-gnu/AsioHeaders/include'
> -I'/home/hornik/tmp/CRAN/Library/unigd/include' -I/usr/local/include
> -DUSE_TYPE_CHECKING_STRICT -D_FORTIFY_SOURCE=3   -fpic  -g -O2 -Wall
> -pedantic   -c cpp11.cpp -o cpp11.o
> clang++-18 -stdlib=libc++  -std=gnu++14
> -I"/home/hornik/tmp/R-d-clang-18-libcxx/include" -DNDEBUG -Ilib
> -DFMT_HEADER_ONLY
> -I'/home/hornik/lib/R/Library/4.4/x86_64-linux-gnu/cpp11/include'
> -I'/home/hornik/lib/R/Library/4.4/x86_64-linux-gnu/AsioHeaders/include'
> -I'/home/hornik/tmp/CRAN/Library/unigd/include' -I/usr/local/include
> -DUSE_TYPE_CHECKING_STRICT -D_FORTIFY_SOURCE=3   -fpic  -g -O2 -Wall
> -pedantic   -c httpgd.cpp -o httpgd.o
> In file included from httpgd.cpp:18:
> In file included from ./httpgd_webserver.h:4:
> In file included from lib/crow.h:2:
> In file included from lib/crow/query_string.h:7:
> /usr/include/c++/v1/unordered_map:955:17: error: no viable overloaded
> '='
>    955 |         __ref() = _VSTD::forward<_ValueTp>(__v);
>        |         ~~~~~~~ ^ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
> /usr/include/c++/v1/__hash_table:1497:47: note: in instantiation of
> function template specialization 'std::__hash_value_type<std::string,
> crow::json::wvalue>::operator=<const std::pair<const std::string,
> crow::json::wvalue> &, void>' requested here
>   1497 |                 __cache->__upcast()->__value_ = *__first;
>        |                                               ^
> /usr/include/c++/v1/unordered_map:1879:14: note: in instantiation of
> function template specialization
> 'std::__hash_table<std::__hash_value_type<std::string,
> crow::json::wvalue>, std::__unordered_map_hasher<std::string,
> std::__hash_value_type<std::string, crow::json::wvalue>,
> std::hash<std::string>, std::equal_to<std::string>>,
> std::__unordered_map_equal<std::string,
> std::__hash_value_type<std::string, crow::json::wvalue>,
> std::equal_to<std::string>, std::hash<std::string>>,
> std::allocator<std::__hash_value_type<std::string,
> crow::json::wvalue>>>::__assign_unique<const std::pair<const
> std::string, crow::json::wvalue> *>' requested here
>   1879 |     __table_.__assign_unique(__il.begin(), __il.end());
>        |              ^
> lib/crow/json.h:1662:26: note: in instantiation of member function
> 'std::unordered_map<std::string, crow::json::wvalue>::operator='
> requested here
>   1662 |                     (*o) = initializer_list;
>        |                          ^
> /usr/include/c++/v1/__utility/pair.h:319:11: note: candidate function
> not viable: no known conversion from 'const std::pair<const std::string,
> crow::json::wvalue>' to 'const
> __conditional_t<is_copy_assignable<first_type>::value &&
> is_copy_assignable<second_type>::value, pair<string &, wvalue &>,
> __nat>' (aka 'const std::__nat') for 1st argument
>    319 |     pair& operator=(__conditional_t<
>        |           ^         ~~~~~~~~~~~~~~~~
>    320 |                         is_copy_assignable<first_type>::value &&
>        |                         ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
>    321 |                         is_copy_assignable<second_type>::value,
>        |                         ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
>    322 |                     pair, __nat> const& __p)
>        |                     ~~~~~~~~~~~~~~~~~~~~~~~
> 
> etc.
> 
> Please fix ...
> 
> Best
> -k

I have fixed the issue and tested the package on rhub2/clang18. The package now compiles with clang-18 and libc++-18 without issues.

Thank you for your patience.
