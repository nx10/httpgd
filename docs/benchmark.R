library(svglite)
library(httpgd)

# Benchmark: Time to plot

results <- bench::press(
  pts = 2^(0:10),
  {
    set.seed(1234)
    x <- runif(pts)
    y <- runif(pts)
    
    svglite_test <- function() {
      stringSVG({
        plot(x, y)
      })
    }
    httpgd_test <- function() {
      hgd_inline({
        plot(x, y)
      })
    }
    
    bench::mark(httpgd_test(), svglite_test(), iterations = 128, check = FALSE)
  }
)

# Benchmark: SVG size

df <- data.frame(pts = 2^(0:12))

df$pts
df["svglite_test()"] <- vapply(df$pts, function(i) {
  set.seed(1234)
  x <- runif(i)
  y <- runif(i)
  nchar(stringSVG({
    plot(x, y)
  }))},
  numeric(1)
)
df["httpgd_test()"] <- vapply(df$pts, function(i) {
  set.seed(1234)
  x <- runif(i)
  y <- runif(i)
  nchar(hgd_inline({
    plot(x, y)
  }))},
  numeric(1)
)

# Merge data

df <- tidyr::pivot_longer(df,c("svglite_test()", "httpgd_test()"), names_to = "expression", values_to = "chars")
results$expression <- as.character(results$expression)
df <- dplyr::inner_join(df, results)

df$mem_alloc <- as.numeric(df$mem_alloc)
dfmem <- tidyr::pivot_longer(df,c("mem_alloc", "chars"), names_to = "mem_type", values_to = "mem_val")

# Plot results

library(ggplot2)

g1 <- ggplot(df, aes(x=pts, y=as.numeric(median), colour=expression)) +
  xlab('number of plot points') +
  ylab('time to plot (sec)') +
  scale_colour_discrete(name = '', labels=list(`svglite_test()`="svglite", `httpgd_test()`="httpgd")) +
  geom_point() +
  geom_line() +
  theme_bw() + 
  theme(legend.position="bottom")


g2 <- ggplot(dfmem, aes(x=pts, y=mem_val/1024, colour=expression, shape=mem_type)) +
  xlab('number of plot points') +
  ylab('size (KB)') +
  scale_shape_discrete(name = '', labels=list(chars="SVG size", mem_alloc="allocated memory")) +
  scale_colour_discrete(name = '', labels=list(`svglite_test()`="svglite", `httpgd_test()`="httpgd")) +
  geom_point() +
  geom_line() +
  theme_bw() + 
  theme(legend.position="bottom", legend.box="vertical", legend.margin=margin())



gridExtra::grid.arrange(g1, g2, ncol = 2)

