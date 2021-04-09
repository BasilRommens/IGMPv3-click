/**
 * In addition to the per-socket multicast reception state, a system
 * must also maintain or compute multicast reception state for each of
 * its interfaces. That state conceptually consists of a set of
 * records of the form:
 *
 * (multicast-address, filter-mode, source-list)
 *
 * At most one record per multicast-address exists for a given
 * interface. This per-interface state is derived from the per-socket
 * state, but may differ from the per-socket state when different
 * sockets have differing filter modes and/or source lists for the
 * same multicast address and interface. (rfc3376, 3.2)
*/