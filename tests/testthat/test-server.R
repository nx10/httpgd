test_that("Render status OK", {
  hgd(token = FALSE, silent = TRUE)
  plot.new()
  res <- httr::GET(hgd_url("plot"))
  dev.off()
  expect_equal(httr::status_code(res), 200)
})

test_that("Plot identical", {
  hgd(token = FALSE, silent = TRUE)
  plot.new()
  res <- httr::GET(hgd_url("plot", renderer = "svg"))
  uplt <- unigd::ugd_render(as = "svg")
  dev.off()
  expect_equal(httr::text_content(res), uplt)
})

test_that("Renderer info identical", {
  hgd(token = FALSE, silent = TRUE)
  res <- httr::GET(hgd_url("renderers"))
  dev.off()
  df_res <- jsonlite::fromJSON(httr::text_content(res))$renderers
  df_ugd <- unigd::ugd_renderers()

  # this API should change at some point
  names(df_res)[names(df_res) == "bin"] <- "text"
  df_res$text <- !df_res$text

  sort_df <- function(df, id_col = "id") {
    df[order(df[[id_col]]), order(colnames(df))]
  }
  df_res <- sort_df(df_res)
  df_ugd <- sort_df(df_ugd)

  expect_equal(colnames(df_res), colnames(df_ugd))
  for (cname in colnames(df_res)) {
    expect_equal(df_res[[cname]], df_ugd[[cname]])
  }
})
