/*
 * bootstrap - uCryptTargetID is 128bit.
 * 	if KADVersion >= KADEMLIA_V6_49aBeta
 * 		send(<empty>, KADEMLIA2_BOOTSTRAP_REQ, ip, udp, 0, ucrypttargetid)
 * 	else
 * 		send(<empty>, KADEMLIA2_BOOTSTRAP_REQ, ip, udp, 0, NULL)
 */

/*
 * send my details - 
 * 	if kadVersion > 1
 * 		OP_KADEMLIA_HEADER (byte)
 * 		byOpcode	(byte)
 * 		KadID		(uint128)
 * 		port		(uint16)
 * 		KADEMLIA_VERSION	(uint8)
 *
 * 		if (!CKademlia::GetPrefs()->GetUseExternKadPort())
 * 			tag uint16
 * 				TAG_SOURCEUPORT
 * 				internal kad port
 * 		
 * 		if byKadVersion >= KADEMLIA_VERSION8_49b
 * 			byMiscOptions = req_ack << 2 | tcp_firwalled << 1 | udp_firewalled;
 *			tag uint8
 *				TAG_KADMISCOPTIONS
 *				byMiscOptions
 *
 *		if byKadVersion >= KADEMLIA_VERSION6_49aBETA
 *			if isnulmd4(uCryptTargetID->GetDataPtr())
 *				warn("crypt enabled node with empty NodeID)
 *				send(byPacket, uLen, uIP, uUDPPort, targetUDPKey, NULL)
 *			else
 *				send(byPacket, uLen, uIP, uUDPPort, targetUDPKey, uCryptTargetID)
 *
 *		else
 *			send(byPacket, uLen, uIP, uUDPPort, targetUDPKey, 0, NULL)
 *
 */

