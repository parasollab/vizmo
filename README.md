# Vizmo

Visualization software for PMPL problems and results.

## Setup

This code requires a copy of PMPL's vizmo branch in the top-level directory to function. The PMPL copy must also have a soft link for pmpl_utils just like old PMPL.

After cloning this repo AND CHECKING OUT THIS BRANCH:
- Also clone PMPL Utils in your PPL root directory: `git clone git@github.com:parasollab/pmpl_utils.git --branch ubuntu-24-04`.
- Switch to the repo's root directory `cd vizmo`.
- Make the smallest possible clone of PMPL's vizmo branch: `git clone git@github.com:parasollab/ppl.git --depth 1 --branch ubuntu-24-04-vizmo`.
- Switch to the PMPL root directory `cd ppl`.
- Soft-link to your pmpl\_utils copy `ln -s ../../pmpl_utils`.
- Switch to ppl's src directory: `cd src`
- Make ppl and pmpl_utils: `make` (with -j4 for more speed)
- Switch to vizmo now: `cd ../../`
- Switch to vizmo's src directory: `cd src`
- QMake, and force qt6: `QT_SELECT=qt6 qmake`
- Make. `make`
