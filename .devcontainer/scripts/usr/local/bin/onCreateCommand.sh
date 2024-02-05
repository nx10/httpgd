#!/usr/bin/env bash
# Copyright (c) 2023 b-data GmbH.
# Distributed under the terms of the MIT License.

set -e

mkdir -p "$HOME/.local/bin"

# Copy Bash-related files from root's backup directory
if [ "$(id -un)" == "root" ]; then
  if [ ! -f /root/.bashrc ]; then
    cp /var/backups/root/.bashrc /root;
  fi
  if [ ! -f /root/.profile ]; then
    cp /var/backups/root/.profile /root;
  fi
fi

# Copy Zsh-related files and folders from the untouched home directory
if [ "$(id -un)" == "root" ]; then
  if [ ! -d /root/.oh-my-zsh ]; then
    cp -R /home/*/.oh-my-zsh /root;
  fi
  if [ ! -f /root/.zshrc ]; then
    cp /home/*/.zshrc /root;
  fi
else
  if [ ! -d "$HOME/.oh-my-zsh" ]; then
    sudo cp -R /root/.oh-my-zsh "$HOME";
    sudo chown -R "$(id -u)":"$(id -g)" "$HOME/.oh-my-zsh";
  fi
  if [ ! -f "$HOME/.zshrc" ]; then
    sudo cp /root/.zshrc "$HOME";
    sudo chown "$(id -u)":"$(id -g)" "$HOME/.zshrc";
  fi
fi

# If existent, prepend the user's private bin to PATH
if ! grep -q "user's private bin" "$HOME/.bashrc"; then
  cat "/var/tmp/snippets/rc.sh" >> "$HOME/.bashrc"
fi
if ! grep -q "user's private bin" "$HOME/.zshrc"; then
  cat "/var/tmp/snippets/rc.sh" >> "$HOME/.zshrc"
fi

# Enable Oh My Zsh plugins
sed -i "s/plugins=(git)/plugins=(docker docker-compose git git-lfs pip screen tmux vscode)/g" \
  "$HOME/.zshrc"

# Remove old .zcompdump files
rm -f "$HOME"/.zcompdump*
