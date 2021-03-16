// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// ! DO NOT CHANGE THIS FILE: Any changes will be removed prior to the project defence. !
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

require (library library/definitions.click)
require (library library/mn.click)

mobile_node :: MobileNode(mobile_node_address, home_agent_private_address, home_agent_public_address);

FromHost(tap0)
	-> mobile_node
	-> ToHost(tap0)

mobile_node[1]
	-> ICMPPingResponder
	-> EtherEncap(0x0800, mobile_node_address:eth, mobile_node_address:eth)
	-> mobile_node

