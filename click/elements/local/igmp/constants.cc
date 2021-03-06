int Constants::MODE_IS_INCLUDE = 1;
int Constants::MODE_IS_EXCLUDE = 2;
int Constants::CHANGE_TO_INCLUDE_MODE = 3;
int Constants::CHANGE_TO_EXCLUDE_MODE = 4;

// RFC 3376, sectie 4
int Constants::REPORT_TYPE = 0x22;
int Constants::QUERY_TYPE = 0x11;

int Defaults::ROBUSTNESS_VARIABLE = 2;
int Defaults::QUERY_INTERVAL = 125;
int Defaults::QUERY_RESPONSE_INTERVAL = 100;
int Defaults::MAX_RESPONSE_CODE = 0x64;
int Defaults::GROUP_MEMBERSHIP_INTERVAL = Defaults::ROBUSTNESS_VARIABLE *Defaults::QUERY_INTERVAL + Defaults::QUERY_RESPONSE_INTERVAL;
int Defaults::OTHER_QUERIER_PRESENT_INTERVAL = Defaults::ROBUSTNESS_VARIABLE * Defaults::QUERY_INTERVAL + 1/2*Defaults::QUERY_RESPONSE_INTERVAL;
int Defaults::STARTUP_QUERY_INTERVAL = 1/4 * Defaults::QUERY_RESPONSE_INTERVAL;
int Defaults::STARTUP_QUERY_COUNT = Defaults::ROBUSTNESS_VARIABLE;
int Defaults::LAST_MEMBER_QUERY_INTERVAL = 10;
int Defaults::LAST_MEMBER_QUERY_COUNT = Defaults::ROBUSTNESS_VARIABLE;
int Defaults::LAST_MEMBER_QUERY_TIME = Defaults::LAST_MEMBER_QUERY_INTERVAL * Defaults::LAST_MEMBER_QUERY_COUNT;
int Defaults::UNSOLICITED_REPORT_INTERVAL = 1;
//int Defaults::OLDER_VERSION_QUERIER_PRESENT_TIMEOUT;
//int Defaults::OLDER_HOST_PRESENT_INTERVAL;