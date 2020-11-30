library(svglite)
library(httpgd)

set.seed(1234)

x <- runif(1e3)
y <- runif(1e3)
tmp1 <- tempfile()
tmp2 <- tempfile()
tmp3 <- tempfile()

svglite_test <- function() {
  svglite(tmp1)
  plot(x, y)
  dev.off()
}
svg_test <- function() {
  svg(tmp2, onefile = TRUE)
  plot(x, y)
  dev.off()
}
httpgd_test <- function() {
  hgd(webserver = FALSE)
  plot(x, y)
  hgd_svg(file = tmp3)
  dev.off()
}

ben <-
  bench::mark(httpgd_test(), svglite_test(), svg_test(), iterations = 250)

packages <- c("httpgd", "svglite", "grDevices")

descrp <-
  function(package_name) {
    sprintf("%s (%s)",
            package_name,
            sapply(package_name, function(p)
              paste(packageVersion(p))))
  }
pvers <- paste(descrp(packages), collapse = ", ")

library(ggplot2)

ggplot2::autoplot(ben) +
  labs(title = "Plot speed (smaller is better)",
       subtitle = pvers) +
  scale_x_discrete(labels = packages, limits = c("httpgd_test()", "svglite_test()", "svg_test()")) +
  ylim(0, NA) +
  theme_bw()

ggsave("docs/bench_speed1.png")

df <- data.frame(name = c("svglite", "grDevices", "httpgd"),
                 kb = file.size(c(tmp1, tmp2, tmp3)) / 1024)

ggplot(data = df, aes(x = reorder(name, kb), y = kb)) +
  geom_bar(stat = "identity") +
  coord_flip() +
  labs(
    x = "Package",
    y = "Size (kB)",
    title = "SVG file size (smaller is better)",
    subtitle = pvers
  )
ggsave("docs/bench_size1.png")

cat(paste0(paste0(knitr::kable(ben[, c(
  "expression",
  "min",
  "median",
  "itr/sec",
  "mem_alloc",
  "gc/sec",
  "n_itr",
  "n_gc",
  "total_time"
)]), collapse = "\n"),
"\n\n", pvers),
file = "docs/bench_speed1.md",
sep = "\n")
