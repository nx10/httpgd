test_that("State status OK", {
  hgd(token = FALSE, silent = TRUE)
  res <- fetch_get(hgd_url("state"))
  dev.off()
  expect_equal(httr::status_code(res), 200)
})

test_that("State token check", {
  tok <- "abc123"
  hgd(token = tok, silent = TRUE)
  res_no_token <- fetch_get(
    hgd_url("state", omit_token = TRUE))
  res_wrong_token <- fetch_get(
    hgd_url("state", omit_token = TRUE, token = "xyz321"))
  res_correct_token <- fetch_get(
    hgd_url("state", omit_token = TRUE, token = tok))
  dev.off()
  expect_equal(httr::status_code(res_no_token), 401)
  expect_equal(httr::status_code(res_wrong_token), 401)
  expect_equal(httr::status_code(res_correct_token), 200)
})

test_that("live status OK", {
  hgd(token = FALSE, silent = TRUE)
  plot.new()
  res <- fetch_get(hgd_url("live"))
  dev.off()
  expect_equal(httr::status_code(res), 200)
})

test_that("Render status OK", {
  hgd(token = FALSE, silent = TRUE)
  plot.new()
  res <- fetch_get(hgd_url("plot"))
  dev.off()
  expect_equal(httr::status_code(res), 200)
})

test_that("All renderers OK", {
  hgd(token = FALSE, silent = TRUE)
  plot.new()
  vres <- vapply(unigd::ugd_renderers()$id, function(renderer_id) {
    httr::status_code(fetch_get(hgd_url("plot", renderer=renderer_id))) == 200
  }, logical(1))
  dev.off()
  expect_true(all(vres))
})

test_that("Plot identical", {
  hgd(token = FALSE, silent = TRUE)
  plot.new()
  res <- fetch_get(hgd_url("plot", renderer = "json"))
  uplt <- unigd::ugd_render(as = "json")
  dev.off()
  expect_equal(httr::content(res, as = "text"), uplt)
})

test_that("Renderer info identical", {
  hgd(token = FALSE, silent = TRUE)
  res <- fetch_get(hgd_url("renderers"))
  dev.off()
  df_res <- jsonlite::fromJSON(httr::content(res, as = "text"))$renderers
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

test_that("Clear plots", {
  fetch_json_hgd <- function(...) {
    jsonlite::fromJSON(
      httr::content(
        fetch_get(
          hgd_url(...)), as = "text"), simplifyVector = FALSE)
  }

  hgd(token = FALSE, silent = TRUE)
  for (i in seq_len(10)) {
    plot(0, main = sprintf("plot_%i", i))
  }

  json_plots <- fetch_json_hgd("plots")

  expect_equal(length(json_plots$plots), 10)
  expect_equal(json_plots$state$hsize, 10)

  fetch_get(hgd_url("clear"))

  json_plots <- fetch_json_hgd("plots")

  expect_equal(length(json_plots$plots), 0)
  expect_equal(json_plots$state$hsize, 0)

  dev.off()
})

test_that("Delete plot", {
  fetch_txt_hgd <- function(...) {
      httr::content(
        fetch_get(
          hgd_url(...)), as = "text")
  }

  hgd(token = FALSE, silent = TRUE)
  for (i in seq_len(10)) {
    plot(0, main = sprintf("plot_%i", i))
  }

  json_p4 <- fetch_txt_hgd("plot", index = 4 - 1, renderer = "json")
  json_p5 <- fetch_txt_hgd("plot", index = 5 - 1, renderer = "json")

  expect_true(grepl("\"str\": \"plot_4\"", json_p4))
  expect_true(grepl("\"str\": \"plot_5\"", json_p5))

  fetch_get(paste0(hgd_url("remove", index = 5 - 1)))

  json_p4 <- fetch_txt_hgd("plot", index = 4 - 1, renderer = "json")
  json_p5 <- fetch_txt_hgd("plot", index = 5 - 1, renderer = "json")

  expect_true(grepl("\"str\": \"plot_4\"", json_p4))
  expect_true(grepl("\"str\": \"plot_6\"", json_p5))

  dev.off()
})

test_that("Delete plot status", {
  hgd(token = FALSE, silent = TRUE)
  for (i in seq_len(10)) {
    plot(0, main = sprintf("plot_%i", i))
  }

  expect_equal(httr::status_code(fetch_get(hgd_url("remove", index = 4))), 200)
  expect_equal(httr::status_code(fetch_get(hgd_url("remove", index = 99))), 404)

  dev.off()
})
