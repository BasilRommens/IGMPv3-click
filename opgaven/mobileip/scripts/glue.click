// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// ! DO NOT CHANGE THIS FILE: Any changes will be removed prior to the project defence. !
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

FromDevice(tap0)
	-> mobility :: Switch
	-> home :: ListenEtherSwitch
	-> home_mobility :: Switch
	-> queue_tap0 :: Queue
	-> ToDevice(tap0)

mobility[1]
	-> foreign :: ListenEtherSwitch
	-> foreign_mobility :: Switch(-1)
	-> queue_tap0

FromDevice(tap1)
	-> [1]home[1]
	-> Queue
	-> ToDevice(tap1)

FromDevice(tap2)
	-> public :: ListenEtherSwitch
	-> Queue
	-> ToDevice(tap2)

FromDevice(tap3)
        -> [1]public[1]
        -> Queue
        -> ToDevice(tap3)

FromDevice(tap4)
        -> [2]public[2]
        -> Queue
        -> ToDevice(tap4)

FromDevice(tap5)
	-> [1]foreign[1]
	-> Queue
	-> ToDevice(tap5)

home[2]
	-> ToDump(home.pcap)
public[3]
	-> ToDump(public.pcap)
foreign[2]
	-> ToDump(foreign.pcap)

