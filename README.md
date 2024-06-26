# Vizmo

Visualization software for PMPL problems and results.

## Setup

This code requires a copy of PMPL's [vizmo branch](https://gitlab.engr.illinois.edu/parasol-group/parasol/pmpl/tree/vizmo) in the top-level directory to function. The PMPL copy must also have a soft link for [pmpl\_utils](https://gitlab.engr.illinois.edu/parasol-group/parasol/pmpl_utils) just like current PMPL (this can be the same copy shared with everything else).

After cloning this repo:
- Switch to the repo's root directory `cd vizmo`.
- Make the smallest possible clone of PMPL's vizmo branch: `git clone git@gitlab.engr.illinois.edu:parasol-group/parasol/pmpl.git --depth 1 --branch vizmo`.
- Switch to the PMPL root directory `cd pmpl`.
- Soft-link to your pmpl\_utils copy `ln -s ../../pmpl_utils`.
- Go to `vizmo/src` and build everything with `make`.
