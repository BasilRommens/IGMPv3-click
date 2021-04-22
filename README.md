# TSCP
Project for Telecommunications system, Arno Deceuninck and Basil Rommens will implement IGMP

## Compiling & Running click script
```shell
cd click
./configure --disable-linuxmodule --enable-local --enable-etherswitch
make elemlist
make
userlevel/click test_project/temp.click
```

## Running on vm
```shell
rm -rf click/scripts/
cp -r opgaven/igmp/scripts click/scripts
sudo click/scripts/setup.sh
cd click/scripts
sudo ./start_click.sh
```

## Connecting with telnet
Ports can be found in `start_click.sh`, example for client22.
```shell
./telnet localhost 10004
write client22/igmp.join ADDRESS 225.1.1.1
read client22/igmp.tables
write client22/igmp.leave ADDRESS 225.1.1.1
read client22/igmp.tables
```