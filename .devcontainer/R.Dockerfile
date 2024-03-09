ARG BUILD_ON_IMAGE=glcr.b-data.ch/r/tidyverse
ARG R_VERSION=latest

FROM docker.io/koalaman/shellcheck:stable as sci

FROM ${BUILD_ON_IMAGE}:${R_VERSION} as files

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

FROM ${BUILD_ON_IMAGE}:${R_VERSION}

ARG DEBIAN_FRONTEND=noninteractive

ENV PARENT_IMAGE_CRAN=${CRAN}

ARG BUILD_ON_IMAGE
ARG CRAN

ARG CRAN_OVERRIDE=${CRAN}

ENV PARENT_IMAGE=${BUILD_ON_IMAGE}:${R_VERSION} \
    CRAN=${CRAN_OVERRIDE:-$CRAN} \
    PARENT_IMAGE_BUILD_DATE=${BUILD_DATE}

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
  ## Install pak
  && pkgType="$(Rscript -e 'cat(.Platform$pkgType)')" \
  && os="$(Rscript -e 'cat(R.Version()$os)')" \
  && arch="$(Rscript -e 'cat(R.Version()$arch)')" \
  && install2.r -r "https://r-lib.github.io/p/pak/stable/$pkgType/$os/$arch" -e \
    pak \
  ## Install languageserver and decor
  && install2.r -s -d TRUE -n "${NCPUS:-$(($(nproc)+1))}" -e \
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
  fi \
  ## Update CRAN
  && sed -i "s|$PARENT_IMAGE_CRAN|$CRAN|g" "$(R RHOME)/etc/Rprofile.site"

## Unset environment variable BUILD_DATE
ENV BUILD_DATE=

## Copy files as late as possible to avoid cache busting
COPY --from=files /files /

## Copy shellcheck as late as possible to avoid cache busting
COPY --from=sci --chown=root:root /bin/shellcheck /usr/local/bin
