# TSCP

Project for Telecommunications system, Arno Deceuninck and Basil Rommens will implement IGMP

## Files

All click elements for our implementation of a subset of IGMPv3 can be found in `click/elements/local/igmp`.
Our `.click` files can be found in `opgaven/igmp/scipts`. These must be copied to `click/scripts` in order to work. The
setup also does this.

## Easy Compile & Run

We made your life easy by providing a `setup_vm.sh` script entirely free for any owner of our TCSP-IGMPv3 source code.
Simply copy paste the `setup_vm.sh` script to your virtual machine and run it. This even clones the repo from GitHub.
Disclaimer: You must have access to our GitHub repo. If not you can copy paste the TCSP-IGMPv3 source code folder to the
home folder of the vm and run the script (although the git pull might give a permission error)

You can also use `compile.sh` to compile and run on your own machine.

Below you can see the seperate steps those scripts do.

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

## More router interfaces
It is possible to add more interfaces to the router, just by specifying the ports on which they
would be delivered on. For the default topology this will be port 1 and 2. Port 3 is always reserved
for UDP, so it can never be used for using other packets. For this part there is no checking of
duplicates in the port list, so it will break the code. This is not written out in click scripts,
so if desired click scripts can be written for it, like the current click script in the router.

## Handlers
These handlers can be called when connecting with telnet.

| Type | Name | Description | Example |
|:-----|------|-------------|--------:|
| write | join | Joins the given multicast group | `write client22/igmp.join 225.1.1.1` |
| write | leave | Leaves the given multicast group | `write client22/igmp.leave 225.1.1.1` |
| write | crash | Crashes the given client | `write client22/igmp.crash` |
| read | tables | Print the socket and interface multicast table of given client | `read client22/igmp.tables` | 