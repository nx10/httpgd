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
