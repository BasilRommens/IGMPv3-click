// Router with three interfaces
// The input/output configuration is as follows:
//
// Input:
//	[0]: packets received on the 192.168.1.0/24 network
//	[1]: packets received on the 192.168.2.0/24 network
//	[2]: packets received on the 192.168.3.0/24 network
//
// Output:
//	[0]: packets sent to the 192.168.1.0/24 network
//	[1]: packets sent to the 192.168.2.0/24 network
//	[2]: packets sent to the 192.168.3.0/24 network
//  [3]: packets destined for the router itself

elementclass Router {
	$server_address, $client1_address, $client2_address |

	// Shared IP input path and routing table
	ip :: Strip(14)
		-> CheckIPHeader
		-> rt :: StaticIPLookup(
					$server_address:ip/32 0,
					$client1_address:ip/32 0,
					$client2_address:ip/32 0,
					$server_address:ipnet 1,
					$client1_address:ipnet 2,
					$client2_address:ipnet 3,
					224.0.0.0/4 4);

	// IGMP router
	igmp::IGMPRouter; // This router will process and handle the IGMP related packets
	//igmp_send::IGMPRouter; // This router will send the UDP packets
	// Will copy into the 2 igmp routers
	//copy::Tee(2);

	// ARP responses are copied to each ARPQuerier and the host.
	arpt :: Tee (3);

	// Input and output paths for interface 0: server
	input[0]
		-> HostEtherFilter($server_address)
		-> server_class :: Classifier(12/0806 20/0001, 12/0806 20/0002, -)
		-> ARPResponder($server_address)
		-> [0]output;

	server_arpq :: ARPQuerier($server_address) -> output;
	server_class[1] -> arpt[0] -> [1]server_arpq;
	server_class[2]
		-> Paint(1)
		-> server_igmp_classifier::IPClassifier(ip proto igmp, -)
		-> StripIPHeader
		-> [0]igmp;

	server_igmp_classifier[1]
		-> ip;

	// Input and output paths for interface 1: client 1
	input[1]
		-> HostEtherFilter($client1_address)
		-> client1_class :: Classifier(12/0806 20/0001, 12/0806 20/0002, -)
		-> ARPResponder($client1_address)
		-> [1]output;

	client1_arpq :: ARPQuerier($client1_address) -> [1]output;
	client1_class[1] -> arpt[1] -> [1]client1_arpq;
	client1_class[2]
		-> Paint(2)
		-> client1_igmp_classifier::IPClassifier(ip proto igmp, -)
		-> StripIPHeader
		-> [1]igmp;

	client1_igmp_classifier[1]
		-> ip;


	// Input and output paths for interface 2: client 2
	input[2]
		-> HostEtherFilter($client2_address)
		-> client2_class :: Classifier(12/0806 20/0001, 12/0806 20/0002, -)
		-> ARPResponder($client2_address)
		-> [2]output;

	client2_arpq :: ARPQuerier($client2_address) -> [2]output;
	client2_class[1] -> arpt[2] -> [1]client2_arpq;
	client2_class[2]
		-> Paint(3)
		-> client2_igmp_classifier::IPClassifier(ip proto igmp, -)
		-> StripIPHeader
		-> [2]igmp;


	client2_igmp_classifier[1]
		-> ip;

	// Local delivery
	rt[0] -> [3]output

	// Forwarding paths per interface
	rt[1]
		-> DropBroadcasts
		-> server_paint :: PaintTee(1)
		-> server_ipgw :: IPGWOptions($server_address)
		-> FixIPSrc($server_address)
		-> server_ttl :: DecIPTTL
		-> server_frag :: IPFragmenter(1500)
		-> server_arpq;

	server_paint[1] -> ICMPError($server_address, redirect, host) -> rt;
	server_ipgw[1]  -> ICMPError($server_address, parameterproblem) -> rt;
	server_ttl[1]   -> ICMPError($server_address, timeexceeded) -> rt;
	server_frag[1]  -> ICMPError($server_address, unreachable, needfrag) -> rt;

	igmp[0]
		// This will be used to detect real IP headers, thus if it is an UDP packet.
		// It will work because an IGMP packet will always contain a type field.
		// This partly corresponds in the IP header with the length of the IP header.
		// Since this type in a bare IGMP packet will always be 0x11 or 0x22, In binary
		// this becomes 0001 0001 and 0010 0010. We only need to look at the last 4
		// bits. These bits should be a minimum of 5, we clearly see that this is
		// not the case and therefore the IP header should be faulty and filtered out.
		-> check_server::CheckIPHeader;

	check_server[0]
		-> server_ttl;

	check_server[1]
		-> IPEncap(2, $client2_address:ip, DST_ANNO, TTL 1)
		-> server_arpq;

	rt[2]
		-> DropBroadcasts
		-> client1_paint :: PaintTee(2)
		-> client1_ipgw :: IPGWOptions($client1_address)
		-> FixIPSrc($client1_address)
		-> client1_ttl :: DecIPTTL
		-> client1_frag :: IPFragmenter(1500)
		-> client1_arpq;

	client1_paint[1] -> ICMPError($client1_address, redirect, host) -> rt;
	client1_ipgw[1]  -> ICMPError($client1_address, parameterproblem) -> rt;
	client1_ttl[1]   -> ICMPError($client1_address, timeexceeded) -> rt;
	client1_frag[1]  -> ICMPError($client1_address, unreachable, needfrag) -> rt;

	igmp[1]
		-> check_client1::CheckIPHeader;

    check_client1[0]
    	-> client1_ttl;

    check_client1[1]
		-> IPEncap(2, $client2_address:ip, DST_ANNO, TTL 1)
		-> client1_arpq;

	rt[3]
		-> DropBroadcasts
		-> client2_paint :: PaintTee(2)
		-> client2_ipgw :: IPGWOptions($client2_address)
		-> FixIPSrc($client2_address)
		-> client2_ttl :: DecIPTTL
		-> client2_frag :: IPFragmenter(1500)
		-> client2_arpq;

	igmp[2]
		-> check_client2::CheckIPHeader;

	check_client2[0]
		-> client2_ttl;

	check_client2[1]
		-> IPEncap(2, $client2_address:ip, DST_ANNO, TTL 1)
		-> client2_arpq;

	client2_paint[1] -> ICMPError($client2_address, redirect, host) -> rt;
	client2_ipgw[1]  -> ICMPError($client2_address, parameterproblem) -> rt;
	client2_ttl[1]   -> ICMPError($client2_address, timeexceeded) -> rt;
	client2_frag[1]  -> ICMPError($client2_address, unreachable, needfrag) -> rt;

	// TODO Manage the broadcast messages, this needs a lot of fixing
	rt[4]
		-> [3]igmp; // Will decide where the multicast message will go

	igmp[3]
		-> server_ttl;
}
