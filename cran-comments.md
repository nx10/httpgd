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

> Prof Brian Ripley <ripley@stats.ox.ac.uk>
> Wed, Jan 10, 1:51 PM (12 days ago)
> to me, CRAN
> 
> Dear maintainer,
> 
> Please see the problems shown on
> <https://cran.r-project.org/web/checks/check_results_httpgd.html>.
> 
> Please correct before 2024-01-24 to safely retain your package on CRAN.
> 
> Do remember to look at the 'Additional issues'.
> 
> The CRAN Team

The clang 18 warnings have been addressed in the `unigd` package.
