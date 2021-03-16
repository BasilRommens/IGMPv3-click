// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// ! DO NOT CHANGE THIS FILE: Any changes will be removed prior to the project defence. !
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

require (library library/definitions.click)
require (library library/ha.click)

home_agent :: Agent(home_agent_private_address, home_agent_public_address, foreign_agent_public_address);

FromHost(tap1)
	-> home_agent
	-> ToHost(tap1)

FromHost(tap2)
	-> [1]home_agent[1]
	-> ToHost(tap2)

home_agent[2]
	-> Discard

