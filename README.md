# Vizmo

Visualization software for PMPL problems and results.

## Setup

This code requires a copy of PMPL's vizmo branch in the top-level directory to function. The PMPL copy must also have a soft link for pmpl_utils just like old PMPL.

After cloning this repo:
- Also clone PMPL Utils in your PPL root directory: `git clone git@github.com:parasollab/pmpl_utils.git`.
- Switch to the repo's root directory `cd vizmo`.
- Make the smallest possible clone of PMPL's vizmo branch: `git clone git@github.com:parasollab/ppl.git --depth 1 --branch ubuntu-20-04-vizmo`.
- Switch to the PMPL root directory `cd pmpl`.
- Soft-link to your pmpl\_utils copy `ln -s ../../pmpl_utils`.
- Go to `vizmo/src` and build everything with `make`.
