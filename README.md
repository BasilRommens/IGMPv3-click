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
**Setup**
```shell
rm -rf click/scripts/
cp -r opgaven/igmp/scripts click/scripts
rm -rf click-reference
cp -r referentie-oplossingen/igmp click-reference
sudo click/scripts/setup.sh
```
You can comment out the parts in the start_click.sh scripts you do
not want to run, but be sure to use all the elements
**Run own implementation**
```shell
cd click/scripts
sudo ./start_click.sh
```
**Run reference implementation**
```shell
cd click-reference/solution
sudo ./start_click.sh
```

## Connecting with telnet
Ports can be found in `start_click.sh`, example for client22.
```shell
./telnet localhost 10004
write client22/igmp.join 225.1.1.1
read client22/igmp.tables
write client22/igmp.leave 225.1.1.1
read client22/igmp.tables
```
You can also send commands without first having to telnet.
```shell
echo "write client22/igmp.join 225.1.1.1" | telnet localhost 10004
echo "write client22/igmp.leave 225.1.1.1" | telnet localhost 10004
```