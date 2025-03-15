## Test environments
- GitHub rlib/actions
- R-hub builder

## CRAN Check Results

> Version: 2.0.3
> Check: whether package can be installed
> Result: WARN 
>   Found the following significant warnings:
>     lib/crow/common.h:351:39: warning: identifier '_method' preceded by whitespace in a literal operator declaration is deprecated [-Wdeprecated-literal-operator]
>   See ‘/home/hornik/tmp/R.check/r-devel-clang/Work/PKGS/httpgd.Rcheck/00install.out’ for details.
>   * used C++ compiler: ‘Debian clang version 19.1.7 (1+b1)’
> Flavor: r-devel-linux-x86_64-debian-clang

The deprecated syntax has been updated.

> rchk Warning

This is caused by upstream package `cpp11`.

> R CMD CHECK allocator<void> deprecation warning

This is caused by upstream package `AsioHeaders`.

## Downstream dependencies
There are no downstream dependencies.
