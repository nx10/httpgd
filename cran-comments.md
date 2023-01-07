## Test environments
GitHub rlib/actions:
* Windows Server 2019 10.0.20348, R 4.2.2
* Mac OS X 12.6.2, R 4.2.2
* Ubuntu 22.04.1, R 4.2.2
* Ubuntu 22.04.1, R 4.1.3
* Ubuntu 22.04.1, R devel (2023-01-06 r83576)

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


## CRAN packages missing inclusion of ´<cstdint>´ 

> POSetR RcppAlgos duckdb fixest flexpolyline fstcore gdalcubes ggiraph
> gkmSVM groupedSurv httpgd libgeos naryn rayrender readxlsb rgeoda rvg s2
> svglite spiderbar tgstat vdiffr wkutils
>
> See the logs and README.txt at https://www.stats.ox.ac.uk/pub/bdr/gcc13/ .
> 
> Although you may not have access to gcc 13 to test this, the missingness
> of the header should be easy to identify from the logs: it defines types
> such as uint32_t and places them in the std namespace.
> 
> Please correct before 2023-01-24 to safely retain your package on CRAN.
> (CRAN submissions are shut until 2023-01-06.)

I have included the missing header.


## Compilation issues with upcoming version of dependency ´BH´ 1.81.0

> Notified by Dirk Eddelbuettel on Thu, Dec 15, 2022.

These have been fixed.


## New ´sprintf´ warning using R devel (2023-01-06 r83576)

>checking compiled code ... WARNING
>  File ‘httpgd/libs/httpgd.so’:
>    Found ‘__sprintf_chk’, possibly from ‘sprintf’ (C)
>      Object: ‘HttpgdWebServer.o’
>  
>  Compiled code should not call entry points which might terminate R nor
>  write to stdout/stderr instead of to the console, nor use Fortran I/O
>  nor system RNGs nor [v]sprintf.
>  
>  See ‘Writing portable packages’ in the ‘Writing R Extensions’ manual.

The included code does not have any occurrence of sprintf so I assume
this is pulled in by a dependency.
