## Test environments
GitHub rlib/actions:
* Windows Server 2019 10.0.17763, R4.0.4
* Mac OS X 10.15.7, R 4.0.4
* Ubuntu 20.04.2, R 4.0.4
* Ubuntu 20.04.2, R devel

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


## CRAN Package Check Issues

### Unit test error on r-devel-linux-x86_64-fedora-*

I was able to reproduce the error and fixed it in this update.

### Warning -Wdeprecated-declarations

The warning

`warning: 'allocator<void>' is deprecated [-Wdeprecated-declarations]`

on `r-devel-linux-x86_64-fedora-clang` originates from the boost.asio
library:

https://github.com/chriskohlhoff/asio/issues/785

It is possibly a false positive:

https://github.com/chriskohlhoff/asio/issues/290

The warning is also only shown with LLVM/clang version 11, 
GCC does not show the warning.

Would you be able to make an exception in this case and allow the warning?
I will monitor the boost.asio issue tracker closely from now on, 
in case a workaround will be released.
