// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// ! DO NOT CHANGE THIS FILE: Any changes will be removed prior to the project defence. !
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

require (library library/definitions.click)
require (library library/cn.click)

corresponding_node :: CorrespondingNode(corresponding_node_address, home_agent_public_address);

FromHost(tap3)
	-> corresponding_node
	-> ToHost(tap3)

icmp :: ICMPPingSource(corresponding_node_address, mobile_node_address)
	-> EtherEncap(0x0800, corresponding_node_address:eth, corresponding_node_address:eth)
	-> corresponding_node

corresponding_node[1]
	-> icmp

