/**
 * For each socket on which IPMulticastListen has been invoked, the
 * system records the desired multicast reception state for that socket.
 * That state conceptually consists of a set of records of the form:
 * (interface, multicast-address, filter-mode, source-list) (rfc3376, 3.1)
 */

/**
 * The socket state evolves in response to each invocation of
 * IPMulticastListen on the socket, as follows:
 * - If the requested filter mode is INCLUDE *and* the requested source
 *   list is empty, then the entry corresponding to the requested
 *   interface and multicast address is deleted if present. If no such
 *   entry is present, the request is ignored.
 * - If the requested filter mode is EXCLUDE *or* the requested source
 *   list is non-empty, then the entry corresponding to the requested
 *   interface and multicast address, if present, is changed to contain
 *   the requested filter mode and source list. If no such entry is
 *   present, a new entry is created, using the parameters specified in
 *   the request. (rfc3376, 3.1)
*/