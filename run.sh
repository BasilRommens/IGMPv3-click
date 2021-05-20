cd click
./configure --disable-linuxmodule --enable-local --enable-etherswitch
make elemlist
make

cd ..
rm -rf click/scripts/
cp -r opgaven/igmp/scripts click/scripts
rm -rf click-reference
cp -r referentie-oplossingen/igmp click-reference
sudo click/scripts/setup.sh

cd click/scripts
sudo ./start_click.sh