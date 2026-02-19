## Test environments
- GitHub Actions: macOS (release, oldrel-1), Windows (release), Ubuntu (devel, release, oldrel-1, oldrel-2)
- R-hub

## R CMD check results

0 errors | 0 warnings | 0 notes

## Resubmission

This package was archived on 2025-04-23.

Relevant changes since archival:

- Updated vendored CrowCpp library to v1.2.1 (fixes deprecated literal operator warning).
- Updated AsioHeaders dependency to >= 1.28.2 (fixes macOS `allocator<void>` deprecation warnings).
- Removed unused function and unused private fields that caused compiler warnings.

## Downstream dependencies
There are no downstream dependencies.
