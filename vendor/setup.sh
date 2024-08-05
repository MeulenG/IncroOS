#!/bin/bash

# Update the main repository
git pull origin master

# Update all submodules
git submodule update --init --recursive

echo "Repository and submodules updated."