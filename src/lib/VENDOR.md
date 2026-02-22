# Vendored Libraries

## CrowCpp (src/lib/crow/)

- **Upstream:** https://github.com/CrowCpp/Crow
- **Version:** 1.2.1.2
- **License:** BSD-3-Clause (see inst/licenses/CrowCpp-BSD-3-Clause.txt)

### Patches applied on top of upstream

1. **logging.h:65** - Commented out `std::cerr` in `CerrLogHandler::log()`.
   CRAN policy forbids writing to stderr from compiled code. httpgd sets a
   custom log handler (`HttpgdLogHandler`) so this default is never used.

2. **common.h:236-248** - Commented out `std::cerr` calls in
   `routing_params::debug_print()`. Same CRAN policy reason.

3. **http_parser_merged.h:1932-1934** - Removed read of potentially
   uninitialized `parser->data` in `http_parser_init()`. GCC 14 warns about
   reading `parser->data` before the struct is initialized; replaced with
   `memset` + `parser->data = NULL`.

4. **common.h:350** - Removed space in literal operator declaration
   `operator"" _method` → `operator""_method`. Clang 20 warns about this
   (`-Wdeprecated-literal-operator`); the spaced form is deprecated in C++23.

5. **middlewares/cors.h:101** - Changed `CORSRules() = delete` to
   `CORSRules() : handler_(nullptr) {}`. Clang 21 requires `CORSHandler` to be
   default-constructible for `std::tuple` member initialization in `Crow()`.

6. **app.h:211** - Added explicit `middlewares_()` initialization in the
   `Crow()` default constructor. Clang 21 errors when a member of type
   `std::tuple<Middlewares...>` is not explicitly initialized in the
   constructor.

### Patches that were needed in older versions but are now fixed upstream

- **json.h** `_LIBCPP_VERSION` preprocessor guard - fixed in v1.2.1.

### How to update

1. Download the release from https://github.com/CrowCpp/Crow/releases
2. Replace contents of `src/lib/crow/` with `include/crow/` from the release
3. Update `src/lib/crow.h` if new headers were added
4. Re-apply the patches listed above (check if upstream has fixed any)
5. Build and run `R CMD check` - look for new warnings or the `std::cerr` NOTE
