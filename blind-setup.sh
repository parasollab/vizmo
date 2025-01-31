# run this file to blindly setup vizmo without understanding what's actually going on :(

git clone git@github.com:parasollab/pmpl_utils.git
cd vizmo
git clone git@github.com:parasollab/ppl.git --depth 1 --branch ubuntu-20-04-vizmo
cd ppl
ln -s ../../pmpl_utils
cd src
