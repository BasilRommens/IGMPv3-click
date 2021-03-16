// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// ! DO NOT CHANGE THIS FILE: Any changes will be removed prior to the project defence. !
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

require (library library/definitions.click)
require (library library/fa.click)

foreign_agent :: Agent(foreign_agent_private_address, foreign_agent_public_address, home_agent_public_address);

FromHost(tap5)
	-> foreign_agent
	-> ToHost(tap5)

FromHost(tap4)
	-> [1]foreign_agent[1]
	-> ToHost(tap4)

foreign_agent[2]
	-> Discard

