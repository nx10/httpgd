FROM rocker/r-devel-san:latest

WORKDIR /app
COPY . .

RUN R -e "install.packages('pak', repos = 'http://cran.us.r-project.org')"
RUN apt-get install -y --no-install-recommends libxml2-dev libssl-dev
RUN R -e "pak::pkg_install('.', dependencies=TRUE)"

ENTRYPOINT ["/bin/sh"]

# Use this: cd tests && R -d "valgrind --leak-check=full" -f testthat.R