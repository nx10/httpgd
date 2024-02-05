#!/usr/bin/env bash
# Copyright (c) 2023 b-data GmbH.
# Distributed under the terms of the MIT License.

set -e

# Create R user library
mkdir -p "$(Rscript -e "cat(Sys.getenv('R_LIBS_USER'))")"

# Install packages stated in environment variable R_PACKAGES
if [ -n "$R_PACKAGES" ]; then
  R -q -e "pak::pak(trimws(unlist(strsplit('$R_PACKAGES', ',')))); \
    pak::cache_clean()"
fi

# Install required (hard) dependencies for the current package
R -q -e "pak::pak(dependencies = NA); pak::cache_clean()"
