## Test environments
GitHub rlib/actions:
* Windows Server 2019 10.0.17763, R 4.0.3
* Mac OS X 10.15.7, R 4.0.3
* Ubuntu 20.04.1, R 4.0.3
* Ubuntu 20.04.1, R devel

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

## Manual inspection comments
Thank you very much for your comments.

> Please add \value to .Rd files regarding exported methods and explain
> the functions results in the documentation. Please write about the
> structure of the output (class) and also what the output means. (If a
> function does not return a value, please document that too, e.g.
> \value{No return value, called for side effects} or similar)
> Missing Rd-tags:
>       hgd_close.Rd: \value
>       hgd.Rd: \value

I have now added \value fields to all function documentations.

> Some code lines in examples are commented out in hgd_close.Rd.
> Please never do that. Ideally find toy examples that can be regularly
> executed and checked. Lengthy examples (> 5 sec), can be wrapped in
> \donttest.

I have corrected hgd_close.Rd.
Many examples are wrapped in \dontrun as they describe interactive use,
they open network ports and browser windows which has to be tested manually
and should should not be run automatically.

> Please always make sure to reset to user's options(), working directory
> or par() after you changed it in examples and vignettes and demos.
> e.g.:
> oldpar <- par(mfrow = c(1,2))
> ...
> par(oldpar)

I adjusted the tests that use par() in 'test-svglite-devSVG.R' and
'test-svglite-text.R' accordingly.

> Please ensure that your functions do not write by default or in your
> examples/vignettes/tests in the user's home filespace (including the
> package directory and getwd()). This is not allowed by CRAN policies.

I adjusted the example of 'hgd_svg()' to use a temporary file.

> Please always add all authors, contributors and copyright holders in the
> Authors@R field with the appropriate roles.
> e.g.: Vinnie Falco
> Please explain in the submission comments what you did about this issue.

Vinnie Falco is the main developer of Boost/Beast which is not vendored by 
this package but supplied from the upstream 'BH' package. But when looking at
this, I noticed that Authors@R was missing some contributors of the svglite 
package. I have added these now. Thank you for pointing this out!
