ARG BASE_IMAGE=debian
ARG BASE_IMAGE_TAG=latest
ARG R_VERSION=latest

FROM docker.io/koalaman/shellcheck:stable as sci

FROM ${BASE_IMAGE}:${BASE_IMAGE_TAG} as files

RUN mkdir /files

COPY conf/shell /files
COPY scripts /files

  ## Ensure file modes are correct
RUN find /files -type d -exec chmod 755 {} \; \
  && find /files -type f -exec chmod 644 {} \; \
  && find /files/usr/local/bin -type f -exec chmod 755 {} \; \
  && cp -r /files/etc/skel/. /files/root \
  && bash -c 'rm -rf /files/root/{.bashrc,.profile}' \
  && chmod 700 /files/root

# Build R according to https://github.com/b-data/rsi
FROM ${BASE_IMAGE}:${BASE_IMAGE_TAG} as rsi

ARG DEBIAN_FRONTEND=noninteractive

ARG COMPILER
ARG COMPILER_VERSION
ARG CXX_STDLIB

# hadolint ignore=DL3008,SC2086
RUN CXX_STDLIB_VERSION=${CXX_STDLIB:+$COMPILER_VERSION} \
  && apt-get update \
  && apt-get install -y --no-install-recommends \
    dpkg-dev \
    "${CXX_STDLIB:-g++}${CXX_STDLIB_VERSION:+-}${CXX_STDLIB_VERSION}${CXX_STDLIB:+-dev}" \
    libc6-dev \
    make \
    ca-certificates \
    "${COMPILER:-gcc}${COMPILER_VERSION:+-}${COMPILER_VERSION}" \
    gfortran \
    libbz2-* \
    '^libcurl[3|4]$' \
    libicu* \
    '^libjpeg.*-turbo.*' \
    liblzma* \
    "${BLAS:-liblapack-dev}" \
    libpangocairo-* \
    libpaper-utils \
    '^libpcre[2|3]*' \
    libpng-dev \
    libreadline-dev \
    '^libtiff[5|6]$' \
    unzip \
    zip \
    zlib1g \
  && BUILDDEPS="curl \
    default-jdk \
    #libbz2-dev \
    libcairo2-dev \
    libcurl4-openssl-dev \
    libpango1.0-dev \
    libjpeg-dev \
    libicu-dev \
    #libpcre2-dev \
    #libpng-dev \
    #libreadline-dev \
    libtiff-dev \
    #liblzma-dev \
    libx11-dev \
    libxt-dev \
    perl \
    rsync \
    subversion \
    tcl-dev \
    tk-dev \
    texinfo \
    texlive-extra-utils \
    texlive-fonts-recommended \
    texlive-fonts-extra \
    texlive-latex-recommended \
    texlive-latex-extra \
    x11proto-core-dev \
    xauth \
    xfonts-base \
    xvfb \
    wget \
    zlib1g-dev" \
  && apt-get install -y --no-install-recommends $BUILDDEPS

ARG R_VERSION
ARG CONFIG_ARGS="--enable-R-shlib \
  --enable-memory-profiling \
  --with-readline \
  --with-blas \
  --with-lapack \
  --with-tcltk \
  --with-recommended-packages"

ARG PREFIX=/usr/local

# hadolint ignore=DL3003,DL3008,SC2034
RUN if [ "$R_VERSION" = "devel" ]; then \
    R_SRC="base-prerelease/R-devel.tar.gz"; \
  elif [ "$R_VERSION" = "patched" ]; then \
    R_SRC="base-prerelease/R-latest.tar.gz"; \
  elif [ "$R_VERSION" = "latest" ]; then \
    R_SRC="base/R-latest.tar.gz"; \
  else \
    R_SRC="base/R-${R_VERSION%%.*}/R-${R_VERSION}.tar.gz"; \
  fi \
  && cd /tmp \
  && wget --progress=dot:mega https://cran.r-project.org/src/${R_SRC} -O "R.tar.gz" \
  && tar zxf R.tar.gz --no-same-owner \
  && cd R-* \
  && export ${COMPILER:+CC="${COMPILER}${COMPILER_VERSION:+-}${COMPILER_VERSION}"} \
  && export ${COMPILER:+CXX="${COMPILER}++${COMPILER_VERSION:+-}${COMPILER_VERSION}"} \
  && export ${CXX_STDLIB:+CXX="$CXX -stdlib=${CXX_STDLIB}"} \
  && R_PAPERSIZE=letter \
  && R_BROWSER=xdg-open \
  && PAGER=/usr/bin/pager \
  && PERL=/usr/bin/perl \
  && R_UNZIPCMD=/usr/bin/unzip \
  && R_ZIPCMD=/usr/bin/zip \
  && R_PRINTCMD=/usr/bin/lpr \
  && LIBnn=lib \
  && AWK=/usr/bin/awk \
  && CFLAGS="-g -O2 -fstack-protector-strong -Wformat -Werror=format-security -Wdate-time -D_FORTIFY_SOURCE=2 -g" \
  && CXXFLAGS="-g -O2 -fstack-protector-strong -Wformat -Werror=format-security -Wdate-time -D_FORTIFY_SOURCE=2 -g" \
  && ./configure ${CONFIG_ARGS} --prefix=${PREFIX} \
  && make \
  && make install-strip \
  && echo "_R_SHLIB_STRIP_=true" >> ${PREFIX}/lib/R/etc/Renviron.site

FROM ${BASE_IMAGE}:${BASE_IMAGE_TAG}

ARG DEBIAN_FRONTEND=noninteractive

ARG BASE_IMAGE
ARG BASE_IMAGE_TAG
ARG BLAS=libopenblas-dev
ARG COMPILER
ARG COMPILER_VERSION
ARG CXX_STDLIB
ARG CRAN

ENV BASE_IMAGE=${BASE_IMAGE}:${BASE_IMAGE_TAG} \
    CRAN=${CRAN}

ENV LANG=en_US.UTF-8 \
    TERM=xterm \
    TZ=Etc/UTC

# Setup according to
# https://github.com/b-data/r-docker-stack/blob/main/ver/latest.Dockerfile
COPY --from=rsi /usr/local /usr/local

# hadolint ignore=DL3008
RUN apt-get update \
  ## Copy script checkbashisms from package devscripts
  && apt-get install -y --no-install-recommends devscripts \
  && cp -a /usr/bin/checkbashisms /usr/local/bin/checkbashisms \
  && apt-get remove -y --purge devscripts \
  && apt-get autoremove -y \
  ## Install R runtime dependencies
  && CXX_STDLIB_VERSION=${CXX_STDLIB:+$COMPILER_VERSION} \
  && apt-get install -y --no-install-recommends \
    dpkg-dev \
    "${CXX_STDLIB:-g++}${CXX_STDLIB_VERSION:+-}${CXX_STDLIB_VERSION}${CXX_STDLIB:+-dev}" \
    libc6-dev \
    make \
    ca-certificates \
    "${COMPILER:-gcc}${COMPILER_VERSION:+-}${COMPILER_VERSION}" \
    gfortran \
    "${CXX_STDLIB}${CXX_STDLIB:+abi}${CXX_STDLIB_VERSION:+-}${CXX_STDLIB_VERSION}${CXX_STDLIB:+-dev}" \
    libbz2-dev \
    '^libcurl[3|4]$' \
    libicu-dev \
    '^libjpeg.*-turbo.*-dev$' \
    liblapack-dev \
    liblzma-dev \
    ${BLAS} \
    libpangocairo-1.0-0 \
    libpaper-utils \
    '^libpcre[2|3]-dev$' \
    libpng-dev \
    libreadline-dev \
    '^libtiff[5|6]$' \
    pkg-config \
    unzip \
    zip \
    zlib1g \
  ## Additional packages
  && apt-get install -y --no-install-recommends \
    bash-completion \
    file \
    gsfonts \
    locales \
    tzdata \
  ## Misc packages (Debian unstable)
    libdeflate-dev \
    libtirpc-dev \
  ## Switch BLAS/LAPACK (manual mode)
  && if [ "${BLAS}" = "libopenblas-dev" ]; then \
    update-alternatives --set "libblas.so.3-$(uname -m)-linux-gnu" \
      "/usr/lib/$(uname -m)-linux-gnu/openblas-pthread/libblas.so.3"; \
    update-alternatives --set "liblapack.so.3-$(uname -m)-linux-gnu" \
      "/usr/lib/$(uname -m)-linux-gnu/openblas-pthread/liblapack.so.3"; \
  fi \
  ## Update locale
  && sed -i "s/# $LANG/$LANG/g" /etc/locale.gen \
  && locale-gen \
  && update-locale LANG=$LANG \
  ## Add directory for site-library
  && RLS="$(Rscript -e "cat(Sys.getenv('R_LIBS_SITE'))")" \
  && mkdir -p "${RLS}" \
  ## Set configured CRAN mirror
  && echo "options(repos = c(CRAN='$CRAN'), download.file.method = 'libcurl')" >> "$(R RHOME)/etc/Rprofile.site" \
  ## Use littler installation scripts
  && Rscript -e "install.packages(c('littler', 'docopt'), repos = '$CRAN')" \
  && ln -s "${RLS}/littler/examples/install2.r" /usr/local/bin/install2.r \
  && ln -s "${RLS}/littler/examples/installGithub.r" /usr/local/bin/installGithub.r \
  && ln -s "${RLS}/littler/bin/r" /usr/local/bin/r \
  ## Clean up
  && rm -rf /tmp/* \
  && rm -rf /var/lib/apt/lists/*

# Setup according to
# https://github.com/b-data/r-docker-stack/blob/main/base/latest.Dockerfile

## Installing V8 on Linux, the alternative way
## https://ropensci.org/blog/2020/11/12/installing-v8
ENV DOWNLOAD_STATIC_LIBV8=1

## Disable prompt to install miniconda
ENV RETICULATE_MINICONDA_ENABLED=0

# hadolint ignore=DL3008
RUN apt-get update \
  && apt-get -y install --no-install-recommends \
    bash-completion \
    curl \
    file \
    fontconfig \
    gnupg \
    htop \
    info \
    jq \
    libclang-dev \
    man-db \
    nano \
    ncdu \
    procps \
    psmisc \
    screen \
    sudo \
    swig \
    tmux \
    vim-tiny \
    wget \
    zsh \
    ## Install Git
    git \
    ## Git: Additional runtime recommendations
    less \
    ssh-client \
    ## Install Git LFS
    git-lfs \
    ## Install pandoc
    pandoc \
  ## Python: Additional dev dependencies
  && if [ -z "$PYTHON_VERSION" ]; then \
    apt-get -y install --no-install-recommends \
      python3-dev \
      ## Install Python package installer
      ## (dep: python3-distutils, python3-setuptools and python3-wheel)
      python3-pip \
      ## Install venv module for python3
      python3-venv; \
    ## make some useful symlinks that are expected to exist
    ## ("/usr/bin/python" and friends)
    for src in pydoc3 python3 python3-config; do \
      dst="$(echo "$src" | tr -d 3)"; \
      if [ -s "/usr/bin/$src" ] && [ ! -e "/usr/bin/$dst" ]; then \
        ln -svT "$src" "/usr/bin/$dst"; \
      fi \
    done; \
  else \
    ## Force update pip, setuptools and wheel
    curl -sLO https://bootstrap.pypa.io/get-pip.py; \
    python get-pip.py \
      pip \
      setuptools \
      wheel; \
    rm get-pip.py; \
  fi \
  ## Git: Set default branch name to main
  && git config --system init.defaultBranch main \
  ## Git: Store passwords for one hour in memory
  && git config --system credential.helper "cache --timeout=3600" \
  ## Git: Merge the default branch from the default remote when "git pull" is run
  && git config --system pull.rebase false \
  ## Clean up
  && rm -rf /tmp/* \
  && rm -rf /var/lib/apt/lists/*

# hadolint ignore=DL3008,DL3013
RUN apt-get update \
  && apt-get -y install --no-install-recommends \
    ## Current ZeroMQ library for R pbdZMQ
    libzmq3-dev \
    ## Required for R extension
    libcairo2-dev \
    libcurl4-openssl-dev \
    libfontconfig1-dev \
    libssl-dev \
    libtiff-dev \
    libxml2-dev \
  ## Install radian
  && export PIP_BREAK_SYSTEM_PACKAGES=1 \
  && pip install --no-cache-dir radian \
  ## Clean up
  && rm -rf /tmp/* \
  && rm -rf /var/lib/apt/lists/* \
    /root/.cache

# Dev Container
ARG NCPUS

# hadolint ignore=DL3008,DL3015,SC2016
RUN dpkgArch="$(dpkg --print-architecture)" \
  ## Ensure that common CA certificates
  ## and OpenSSL libraries are up to date
  && apt-get update \
  && apt-get -y install --only-upgrade \
    ca-certificates \
    openssl \
  ## Required for devtools
  && apt-get -y install --no-install-recommends \
    libfribidi-dev \
    libgit2-dev \
    libharfbuzz-dev \
  ## Install pak
  && pkgType="$(Rscript -e 'cat(.Platform$pkgType)')" \
  && os="$(Rscript -e 'cat(R.Version()$os)')" \
  && arch="$(Rscript -e 'cat(R.Version()$arch)')" \
  && install2.r -r "https://r-lib.github.io/p/pak/stable/$pkgType/$os/$arch" -e \
    pak \
  ## Install devtools, languageserver and decor
  && install2.r -s -d TRUE -n "${NCPUS:-$(($(nproc)+1))}" -e \
    devtools \
    languageserver \
    decor \
  ## Clean up
  && rm -rf /tmp/* \
    /root/.cache \
  ## Install hadolint
  && case "$dpkgArch" in \
    amd64) tarArch="x86_64" ;; \
    arm64) tarArch="arm64" ;; \
    *) echo "error: Architecture $dpkgArch unsupported"; exit 1 ;; \
  esac \
  && apiResponse="$(curl -sSL \
    https://api.github.com/repos/hadolint/hadolint/releases/latest)" \
  && downloadUrl="$(echo "$apiResponse" | grep -e \
    "browser_download_url.*Linux-$tarArch\"" | cut -d : -f 2,3 | tr -d \")" \
  && echo "$downloadUrl" | xargs curl -sSLo /usr/local/bin/hadolint \
  && chmod 755 /usr/local/bin/hadolint \
  ## Create backup of root directory
  && cp -a /root /var/backups \
  ## Clean up
  && rm -rf /var/lib/apt/lists/*

## Install common utils
## https://github.com/devcontainers/features/blob/main/src/common-utils/main.sh
ARG INSTALL_COMMON_UTILS

# hadolint ignore=DL3008
RUN if [ "$INSTALL_COMMON_UTILS" = "true" ]; then \
    apt-get update; \
    apt-get -y install --no-install-recommends \
      apt-utils \
      openssh-client \
      gnupg2 \
      dirmngr \
      iproute2 \
      procps \
      lsof \
      htop \
      net-tools \
      psmisc \
      curl \
      tree \
      wget \
      rsync \
      ca-certificates \
      unzip \
      bzip2 \
      xz-utils \
      zip \
      nano \
      vim-tiny \
      less \
      jq \
      lsb-release \
      apt-transport-https \
      dialog \
      libc6 \
      libgcc1 \
      libkrb5-3 \
      libgssapi-krb5-2 \
      libicu[0-9][0-9] \
      liblttng-ust[0-9] \
      libstdc++6 \
      zlib1g \
      locales \
      sudo \
      ncdu \
      man-db \
      strace \
      manpages \
      manpages-dev \
      init-system-helpers; \
    fi \
  ## Clean up
  && rm -rf /var/lib/apt/lists/*

ENV PACKAGES_ALREADY_INSTALLED=${INSTALL_COMMON_UTILS}

# Update environment
ARG USE_ZSH_FOR_ROOT
ARG SET_LANG
ARG SET_TZ

ENV LANG=${SET_LANG:-$LANG} \
    TZ=${SET_TZ:-$TZ}

  ## Change root's shell to ZSH
RUN if [ -n "$USE_ZSH_FOR_ROOT" ]; then \
    chsh -s /bin/zsh; \
  fi \
  ## Update timezone if needed
  && if [ "$TZ" != "Etc/UTC" ]; then \
    echo "Setting TZ to $TZ"; \
    ln -snf "/usr/share/zoneinfo/$TZ" /etc/localtime \
      && echo "$TZ" > /etc/timezone; \
  fi \
  ## Add/Update locale if needed
  && if [ "$LANG" != "en_US.UTF-8" ]; then \
    sed -i "s/# $LANG/$LANG/g" /etc/locale.gen; \
    locale-gen; \
    echo "Setting LANG to $LANG"; \
    update-locale --reset LANG="$LANG"; \
  fi

## Copy files as late as possible to avoid cache busting
COPY --from=files /files /

## Copy shellcheck as late as possible to avoid cache busting
COPY --from=sci --chown=root:root /bin/shellcheck /usr/local/bin
