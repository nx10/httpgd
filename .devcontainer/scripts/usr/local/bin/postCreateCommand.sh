#!/usr/bin/env bash
# Copyright (c) 2023 b-data GmbH.
# Distributed under the terms of the MIT License.

set -e

# Change ownership of the workspaces folder
sudo chown "$(id -u)":"$(id -g)" /workspaces
