// Output configuration: 
//
// Packets for the network are put on output 0
// Packets for the host are put on output 1

elementclass Client {
	$address, $gateway |

	ip :: Strip(14)
		-> CheckIPHeader()
		-> rt :: StaticIPLookup(
					$address:ip/32 0,
					$address:ipnet 0,
					0.0.0.0/0.0.0.0 $gateway 1)
		-> [1]output;
	
	rt[1]
		-> DropBroadcasts
		-> ipgw :: IPGWOptions($address)
		-> FixIPSrc($address)
		-> ttl :: DecIPTTL
		-> frag :: IPFragmenter(1500)
		-> arpq :: ARPQuerier($address)
		-> output;

	ipgw[1] -> ICMPError($address, parameterproblem) -> output;
	ttl[1]  -> ICMPError($address, timeexceeded) -> output;
	frag[1] -> ICMPError($address, unreachable, needfrag) -> output;

	// IGMP part
	// --------------------------
	igmp :: IGMPClient;
	// This will classify all the igmp stuff on different output ports
	igmp_classifier::IPClassifier(
		ip proto igmp and dst host 224.0.0.1,
		ip proto igmp,
		-);

	// All the packets that are not in using the IGMP protocol
	igmp_classifier[2]
		-> ip;

	// All the packets that are under the IGMP protocol and are not broadcasts
	igmp_classifier[1]
		-> DropBroadcasts
		-> igmp_packet::StripIPHeader // Will take the igmp packet inside the IP packet
		-> ToDump('igmp_packet.pcap')
		-> igmp // The IGMP packet will be routed into the IGMP element of a client
		-> IPEncap(2, $address, DST_ANNO, TTL 2)
		-> arpq :: ARPQuerier($address)
		-> output;

	// The special broadcast should not be dropped, so it skips the drop broadcasts
	igmp_classifier[0]
		-> igmp_packet;

	// Incoming Packets
	input
		-> HostEtherFilter($address)
		-> in_cl :: Classifier(12/0806 20/0001, 12/0806 20/0002, 12/0800)
		-> arp_res :: ARPResponder($address)
		-> output;

	in_cl[1] -> [1]arpq;
	in_cl[2] -> igmp_classifier; // in order to filter out the IGMP packets
}
