# TSCP

Project for Telecommunications system, Arno Deceuninck and Basil Rommens will implement IGMP

## Compiling & Running click script

In order to compile the project you need to execute the following commands in your favourite shell.

```shell
cd click
./configure --disable-linuxmodule --enable-local --enable-etherswitch
make elemlist
make
```

## Running on vm

Since the implementation must run on the vm, the commands described below are used to run the implementation on the
virtual machine.
**Setup**
These commands are used to set up the correct scripts in the correct locations in order to execute them properly.

```shell
rm -rf click/scripts/
cp -r opgaven/igmp/scripts click/scripts
rm -rf click-reference
cp -r referentie-oplossingen/igmp click-reference
sudo click/scripts/setup.sh
```

You can comment out the parts in the start_click.sh scripts you do not want to run, but be sure to use all the elements.
This will only work after you've compiled the code.
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

The following table is a list of all the ports used for each click element. This can be used to access each
corresponding element. 

| Name | Port | 
|:-----|--------:| 
| router | 10001| 
| server | 10002 | 
| client21 | 10003 | 
| client22 | 10004 | 
| client31 | 10005 | 
| client32 | 10006 |

Here is an example for client22 with port 10004.

```shell
./telnet localhost 10004
write client22/igmp.join 225.1.1.1
read client22/igmp.tables
write client22/igmp.leave 225.1.1.1
read client22/igmp.tables
```

You can also send commands without first having to execute the telnet command.

```shell
echo "write client22/igmp.join 225.1.1.1" | telnet localhost 10004
echo "write client22/igmp.leave 225.1.1.1" | telnet localhost 10004
```