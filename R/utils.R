# utils.R
# from: https://github.com/r-lib/svglite/blob/master/R/utils.R

mini_plot <- function(...) graphics::plot(..., axes = FALSE, xlab = "", ylab = "")

plot_dim <- function(dim = c(NA, NA)) {
  if (any(is.na(dim))) {
    if (length(grDevices::dev.list()) == 0) {
      default_dim <- c(10, 8)
    } else {
      default_dim <- grDevices::dev.size()
    }

    dim[is.na(dim)] <- default_dim[is.na(dim)]
    dim_f <- prettyNum(dim, digits = 3)

    message("Saving ", dim_f[1], "\" x ", dim_f[2], "\" image")
  }

  dim
}

vapply_chr <- function(.x, .f, ...) {
  vapply(.x, .f, character(1), ...)
}
vapply_lgl <- function(.x, .f, ...) {
  vapply(.x, .f, logical(1), ...)
}
lapply_if <- function(.x, .p, .f, ...) {
  if (!is.logical(.p)) {
    .p <- vapply_lgl(.x, .p)
  }
  .x[.p] <- lapply(.x[.p], .f, ...)
  .x
}
keep <- function(.x, .p, ...) {
  .x[vapply_lgl(.x, .p, ...)]
}
compact <- function(x) {
  Filter(length, x)
}
`%||%` <- function(x, y) {
  if (is.null(x)) y else x
}
is_scalar_character <- function(x) {
  is.character(x) && length(x) == 1
}
names2 <- function(x) {
  names(x) %||% rep("", length(x))
}
ilapply <- function(.x, .f, ...) {
  idx <- names(.x) %||% seq_along(.x)
  out <- Map(.f, names(.x), .x, ...)
  names(out) <- names(.x)
  out
}
ilapply_if <- function(.x, .p, .f, ...) {
  if (!is.logical(.p)) {
    .p <- vapply_lgl(.x, .p)
  }
  .x[.p] <- ilapply(.x[.p], .f, ...)
  .x
}
set_names <- function(x, nm = x) {
  stats::setNames(x, nm)
}
zip <- function(.l) {
  fields <- set_names(names(.l[[1]]))
  lapply(fields, function(i) {
    lapply(.l, .subset2, i)
  })
}

invalid_filename <- function(filename) {
  if (!is.character(filename) || length(filename) != 1) {
    return(TRUE)
  }

  # strip double occurences of %
  stripped_file <- gsub("%{2}", "", filename)
  # filename is fine if there are no % left
  if (!grepl("%", stripped_file)) {
    return(FALSE)
  }
  # remove first allowed pattern, % followed by digits followed by [diouxX]
  stripped_file <- sub("%[#0 ,+-]*[0-9.]*[diouxX]", "", stripped_file)
  # matching leftover % indicates multiple patterns or a single incorrect pattern (e.g., %s)
  return(grepl("%", stripped_file))
}
