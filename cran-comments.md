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

## I received the following email from CRAN on 2024-05-06:

> GCC 14 is due to be released tomorrow (in some time zone) and we have
> been testing CRAN against 14.1RC (which reports as 14.0.1). Packages
>
>    Numero SheetReader batchmix epanet2toolkit httpgd
> have new compilation warnings shown in the fedora-gcc results on CRAN.
>
> Please correct before 2024-06-06 to safely retain your package on CRAN.

I have fixed the issue and am resubmitting the package.
