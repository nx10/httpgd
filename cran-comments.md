## Test environments
GitHub rlib/actions:
* Windows Server 2019 10.0.17763, R 4.1.2
* Mac OS X 11.6.2, R 4.1.2
* Ubuntu 20.04.3, R 4.1.2
* Ubuntu 20.04.3, R devel
R-hub builder:
* Windows Server 2022, R-devel, 64 bit
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

## UCRT

I have applied the provided patches for UCRT.

## Failing test on CRAN Windows Server 2022

> Failed tests
> Failure (test-svglite-text-fonts.R:7:3): font sets weight/style 
> 
>   style_attr(text, "font-weight") not equal to c(NA, "bold", NA, "bold").
>   2/4 mismatches
>   x[2]: NA
>   y[2]: "bold"
>   
>   x[4]: NA
>   y[4]: "bold"
>   
>   [ FAIL 1 | WARN 1 | SKIP 1 | PASS 101 ]
>   Error: Test failures

I can not replicate the issue with RHub.
This seems to be a problem with the `systemfonts` package and the new CRAN windows server 2022 machine. (See: https://github.com/r-lib/svglite/issues/145#issuecomment-1004716572)

I have disabled the font tests on windows and will reenable them as soon as the `systemfonts` issues are resolved.