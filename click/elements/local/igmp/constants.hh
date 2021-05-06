#ifndef CLICK_Constants_HH
#define CLICK_Constants_HH

namespace Constants {
    // RFC3376, 4.2.12
    extern int MODE_IS_INCLUDE;
    extern int MODE_IS_EXCLUDE;
    extern int CHANGE_TO_INCLUDE_MODE;
    extern int CHANGE_TO_EXCLUDE_MODE;

    extern int REPORT_TYPE;
}

namespace Defaults {
    // RFC 3376, section 8
    /**
     * 8.1. Robustness Variable
     *
     * The Robustness Variable allows tuning for the expected packet loss on
     * a network. If a network is expected to be lossy, the Robustness
     * Variable may be increased. IGMP is robust to (Robustness Variable -
     * 1) packet losses. The Robustness Variable MUST NOT be zero, and
     * SHOULD NOT be one. Default: 2
     */
    extern int ROBUSTNESS_VARIABLE;
    /**
     * 8.2. Query Interval
     *
     * The Query Interval is the interval between General Queries sent by
     * the Querier. Default: 125 seconds.
     * By varying the [Query Interval], an administrator may tune the number
     * of IGMP messages on the network; larger values cause IGMP Queries to
     * be sent less often.
     */
    extern int QUERY_INTERVAL;

    /**
     * 8.3. Query Response Interval
     *
     * The Max Response Time used to calculate the Max Resp Code inserted
     * into the periodic General Queries. Default: 100 (10 seconds)
     * By varying the [Query Response Interval], an administrator may tune
     * the burstiness of IGMP messages on the network; larger values make
     * the traffic less bursty, as host responses are spread out over a
     * larger interval. The number of seconds represented by the [Query
     * Response Interval] must be less than the [Query Interval].
     */
    extern int QUERY_RESPONSE_INTERVAL;

    /**
     * 8.4. Group Membership Interval
     *
     * The Group Membership Interval is the amount of time that must pass
     * before a multicast router decides there are no more members of a
     * group or a particular source on a network.
     * This value MUST be ((the Robustness Variable) times (the Query
     * Interval)) plus (one Query Response Interval).
     */
    extern int GROUP_MEMBERSHIP_INTERVAL;

    /**
     * 8.5. Other Querier Present Interval
     *
     * The Other Querier Present Interval is the length of time that must
     * pass before a multicast router decides that there is no longer
     * another multicast router which should be the querier. This value
     * MUST be ((the Robustness Variable) times (the Query Interval)) plus
     * (one half of one Query Response Interval).
     */
    extern int OTHER_QUERIER_PRESENT_INTERVAL;
    /**
     * 8.6. Startup Query Interval
     *
     * The Startup Query Interval is the interval between General Queries
     * sent by a Querier on startup. Default: 1/4 the Query Interval.
     */
    extern int STARTUP_QUERY_INTERVAL;

    /**
     * 8.7. Startup Query Count
     *
     * The Startup Query Count is the number of Queries sent out on startup,
     * separated by the Startup Query Interval. Default: the Robustness
     * Variable.
     */
    extern int STARTUP_QUERY_COUNT;

    /**
     * 8.8. Last Member Query Interval
     *
     * The Last Member Query Interval is the Max Response Time used to
     * calculate the Max Resp Code inserted into Group-Specific Queries sent
     * in response to Leave Group messages. It is also the Max Response
     * Time used in calculating the Max Resp Code for Group-and-Source-
     * Specific Query messages. Default: 10 (1 second)
     * Note that for values of LMQI greater than 12.8 seconds, a limited set
     * of values can be represented, corresponding to sequential values of
     * Max Resp Code. When converting a configured time to a Max Resp Code
     * value, it is recommended to use the exact value if possible, or the
     * next lower value if the requested value is not exactly representable.
     * This value may be tuned to modify the "leave latency" of the network.
     * A reduced value results in reduced time to detect the loss of the
     * last member of a group or source.
     */
    extern int LAST_MEMBER_QUERY_INTERVAL;

    /**
     * 8.9. Last Member Query Count
     *
     * The Last Member Query Count is the number of Group-Specific Queries
     * sent before the router assumes there are no local members. The Last
     * Member Query Count is also the number of Group-and-Source-Specific
     * Queries sent before the router assumes there are no listeners for a
     * particular source. Default: the Robustness Variable.
     */
    extern int LAST_MEMBER_QUERY_COUNT;

    /**
     * 8.10. Last Member Query Time
     *
     * The Last Member Query Time is the time value represented by the Last
     * Member Query Interval, multiplied by the Last Member Query Count. It
     * is not a tunable value, but may be tuned by changing its components.
     */
    extern int LAST_MEMBER_QUERY_TIME;
    /**
     * 8.11. Unsolicited Report Interval
     *
     * The Unsolicited Report Interval is the time between repetitions of a
     * host’s initial report of membership in a group. Default: 1 second.
     */
    extern int UNSOLICITED_REPORT_INTERVAL;
    /**
     * 8.12. Older Version Querier Present Timeout
     *
     * The Older Version Querier Interval is the time-out for transitioning
     * a host back to IGMPv3 mode once an older version query is heard.
     * When an older version query is received, hosts set their Older
     * Version Querier Present Timer to Older Version Querier Interval.
     * This value MUST be ((the Robustness Variable) times (the Query
     * Interval in the last Query received)) plus (one Query Response
     * Interval).
     */
//    extern int OLDER_VERSION_QUERIER_PRESENT_TIMEOUT;
    /**
     * 8.13. Older Host Present Interval
     *
     * The Older Host Present Interval is the time-out for transitioning a
     * group back to IGMPv3 mode once an older version report is sent for
     * that group. When an older version report is received, routers set
     * their Older Host Present Timer to Older Host Present Interval.
     * This value MUST be ((the Robustness Variable) times (the Query
     * Interval)) plus (one Query Response Interval).
     */
//    extern int OLDER_HOST_PRESENT_INTERVAL;
}

#endif