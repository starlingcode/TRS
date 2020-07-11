
# TRS

Rack mockups of modules that pass stereo signals over TRS cables.

User documentation can be found here: https://starling.space/trs/modules

## Build Instructions

First, build Rack (https://vcvrack.com/manual/Building.html)

Change the current directory to Rack/plugins and clone the repo:
```
git clone https://github.com/starlingcode/TRS.git
```
Navigate to the Via plugin directory:
```
cd TRS
```
Update the submodules:
```
git submodule update --init --recursive
```
Then, build the plugins:
```
make
```
