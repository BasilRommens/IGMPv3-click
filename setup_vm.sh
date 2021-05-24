#!/bin/sh

# Exit on error
set -e

# Change the directory to /home/student
cd

if [-d TCSP-IGMPv3]; then
  git clone https://github.com/BasilRommens/TCSP-IGMPv3
else
  cd TCSP-IGMPv3
  git pull
endif

# Copy our implementation files to click in the home folder
rm -rf click/elements/local/igmp/
cp -r TCSP-IGMPv3/click/elements/local/igmp click/elements/local/
rm -rf click/scripts/
cp -r TCSP-IGMPv3/opgaven/igmp/scripts click/scripts/

# Configure & Compile
cd click
./configure --disable-linuxmodule --enable-local --enable-etherswitch
make elemlist
make

sudo scripts/setup.sh

# Start
cd scripts
sudo ./start_click.sh