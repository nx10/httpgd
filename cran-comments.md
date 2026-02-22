## Test environments
- GitHub Actions: macOS (release, oldrel-1), Windows (release), Ubuntu (devel, release, oldrel-1, oldrel-2)
- R-hub: linux, windows, macos-arm64, clang-asan, clang-ubsan, gcc-asan, clang16-clang20, etc.
- Docker: Debian testing with Clang 21.1.8

## R CMD check results

0 errors | 0 warnings | 0 notes

## Resubmission

This package was archived on 2025-04-23.

v2.1.1 failed on Debian/Clang 21. Fixed vendored CrowCpp `std::tuple` member initialization.

## Downstream dependencies
There are no downstream dependencies.
