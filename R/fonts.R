# fonts.r
# from: https://github.com/r-lib/svglite/blob/master/R/fonts.R

r_font_families <- c("sans", "serif", "mono", "symbol")
r_font_faces <- c("plain", "bold", "italic", "bolditalic", "symbol")

alias_lookup <- function() {
  if (.Platform$OS.type == "windows") {
    serif_font <- "Times New Roman"
    symbol_font <- "Standard Symbols L"
  } else {
    serif_font <- "Times"
    symbol_font <- "Symbol"
  }
  c(
    sans = "Arial",
    serif = serif_font,
    mono = "Courier",
    symbol = symbol_font
  )
}

#' @importFrom systemfonts font_info
match_family <- function(font, bold = FALSE, italic = FALSE) {
  font_info(font, bold = bold, italic = italic)$family[1]
}

validate_aliases <- function(system_fonts, user_fonts) {
  system_fonts <- compact(lapply(system_fonts, compact))
  user_fonts <- compact(lapply(user_fonts, compact))

  system_fonts <- lapply(system_fonts, validate_system_alias)
  user_fonts <- ilapply(user_fonts, validate_user_alias)

  aliases <- c(names(system_fonts), names(user_fonts))
  if (any(duplicated(aliases))) {
    stop("Cannot supply both system and font alias", call. = FALSE)
  }

  # Add missing system fonts for base families
  missing_aliases <- setdiff(r_font_families, aliases)
  system_fonts[missing_aliases] <-
    lapply(alias_lookup()[missing_aliases], match_family)

  list(
    system = system_fonts,
    user = user_fonts
  )
}

validate_system_alias <- function(alias) {
  if (!is_scalar_character(alias)) {
    stop("System fonts must be scalar character vector", call. = FALSE)
  }

  matched <- match_family(alias)
  if (alias != matched) {
    warning(
      call. = FALSE,
      "System font `",
      alias,
      "` not found. ",
      "Closest match: `",
      matched,
      "`"
    )
  }
  matched
}

is_user_alias <- function(x) {
  is.list(x) &&
    (is_scalar_character(x$file) || is_scalar_character(x$ttf)) &&
    (is_scalar_character(x$alias) || is_scalar_character(x$name))
}

validate_user_alias <- function(default_name, family) {
  if (!all(names(family) %in% r_font_faces)) {
    stop(
      "Faces must contain only: ",
      paste(sprintf("`%s`", r_font_faces), collapse = ", "),
      call. = FALSE
    )
  }

  is_alias_object <- vapply_lgl(family, is_user_alias)
  is_alias_plain <- vapply_lgl(family, is_scalar_character)

  is_valid_alias <- is_alias_object | is_alias_plain
  if (any(!is_valid_alias)) {
    stop(
      call. = FALSE,
      "The following faces are invalid for `",
      default_name,
      "`: ",
      paste0(names(family)[!is_valid_alias], collapse = ", ")
    )
  }

  names <- ifelse(is_alias_plain, default_name, family)
  names <- lapply_if(names, is_alias_object, function(obj) {
    obj$alias %||% obj$name
  })
  files <- lapply_if(family, is_alias_object, function(obj) {
    obj$file %||% obj$ttf
  })

  file_exists <- vapply_lgl(files, file.exists)
  if (any(!file_exists)) {
    missing <- unlist(files)[!file_exists]
    stop(
      call. = FALSE,
      "Could not find font file: ",
      paste0(missing, collapse = ", ")
    )
  }

  zip(list(name = names, file = files))
}
