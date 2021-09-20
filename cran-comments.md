## Test environments
GitHub rlib/actions:
* Windows Server 2019 10.0.17763, R 4.1.1
* Mac OS X 10.15.7, R 4.1.1
* Ubuntu 20.04.3, R 4.1.1
* Ubuntu 20.04.3, R devel
R-hub builder
* Windows Server 2008 R2 SP1, R-devel, 32/64 bit
* Ubuntu Linux 20.04.1 LTS, R-release, GCC
* Fedora Linux, R-devel, clang, gfortran
* Debian Linux, R-devel, GCC ASAN/UBSAN

## R CMD check results
There were no ERRORs or WARNINGs. 

There was 1 NOTE:

* checking installed package size ... NOTE
    installed size is  5.6Mb
    sub-directories of 1Mb or more:
      libs   5.4Mb

  This is the directory where build time dependency png.h is downloaded on windows.

## Downstream dependencies
There are no downstream dependencies.

## CRAN sumbission errors

### Flavors: r-release-macos-x86_64, r-oldrel-macos-x86_64

```
In file included from DrawData.cpp:2:
In file included from ./DrawData.h:4:
./HttpgdGeom.h:36:17: error: call to 'abs' is ambiguous
        return (std::abs(r0.x - r1.x) < eps) &&
                ^~~~~~~~
DrawData.cpp:90:16: note: in instantiation of function template
specialization 'httpgd::rect_equals<double>' requested here
        return rect_equals(t_rect, rect, 0.01);
                ^
/usr/include/stdlib.h:137:6: note: candidate function
int      abs(int) __pure2;
          ^
/Library/Developer/CommandLineTools/usr/include/c++/v1/stdlib.h:111:44:
note: candidate function
inline _LIBCPP_INLINE_VISIBILITY long      abs(     long __x) _NOEXCEPT
{return  labs(__x);}
                                            ^
/Library/Developer/CommandLineTools/usr/include/c++/v1/stdlib.h:113:44:
note: candidate function
inline _LIBCPP_INLINE_VISIBILITY long long abs(long long __x) _NOEXCEPT
{return llabs(__x);}
                                            ^
1 error generated.

You were warned about that in 'Writing R Extensions'.  std::fabs is
required ... four times, and to include <cmath>.
```


We replaced all occurences were `std::abs()` was used for floating point operations
with `std::fabs()` and added missing `cmath` includes where necessary.
We also tested the build on Rhub 'macOS 10.13.6 High Sierra, R-release, CRAN's setup'.
