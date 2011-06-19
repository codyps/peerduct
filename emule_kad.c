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

/* Firewall check -
 *
 * if kadVersion > 6
 * 	pkt {
 * 		thePrefs::GetPort()
 * 		CKademlia::GetPrefs()->GetClientHash()
 * 		CPrefs::GetConnectOptions(true, false)
 * 	}
 * 	send pkt KADEMLIA_FIREWALLED2_REQ ip port senderKey NULL
 * else
 * 	pkt {
 * 		thePrefs::GetPort()
 * 	}
 * 	send pkt KADEMLIA_FIREWALLED_REQ ip port senderKey NULL
 *
 */

/* publish source -
 * 
 * packetdata << targetID
 * if contact.GetVersion >= 4 (47c)
 * 	opcode <- KADEMLIA2_PUBLISH_SOURCE_REQ;
 * 	packetdata << contactID
 * 		   << tags (a tag ptr list)
 * else
 * 	opcode <- KADEMLIA_PUBLISH_REQ
 * 	packetdata << 1 (this 1 used to be other things too, only for sources now.)
 * 		   << contactID
 * 		   << tags
 *
 * if contact.GetVersion() >= 6 (obfuscated ?)
 * 	send packetdata opcode contact.get_ip_addr contact.get_udp_port contact.get_udp_key contact.get_client_id
 * else
 * 	send packetdata opcode contact.get_ip_addr contact.get_udp_port 0 NULL
 *
 */

/* process packet -
 *
 * if port == 53 && sender_key.is_empty()
 * 	( avoid attacks based on DNS proto confusion )
 * 	return;
 *
 * bool cur_on <- CKademlia::GetPrefs()->HasHadContact()
 * CKademlia::GetPrefs()->SetLastContact();
 *
 * if ( curCon != CKademlia::GetPrefs()->HasHadContact() )
 * 	theApp->ShowConnectionState();
 *
 * uint8_t opcode = data[1];
 * const uint8_t *pkt_data = data + 2;
 * uint32_t len_pkt = len_data - 2;
 *
 */

/* legacy-challeng ip port contactID =
 * 
 * if has_active_legacy_challenge ip
 * 	return
 *
 * packetdata <<
 * 	@# 8 KADEMLIA_FIND_VALUE
 * 	@# 128 ( rand_128 ? : 1 )
 *	@# 128 contactID
 *
 * send packetdata KADEMLIA2_REQ ip port 0 NULL
 * track_challenge contactID challenge ip KADEMLIA2_REQ
 */

/* expire_immed :: CKadClientSearcher
 * expire_client_search expire_immed =`
 * 	scan a list of fetchNodeID requests for expire_immed and remove if found.
 * 	additionally, check if any requests in this list have timed out
 */

/* find_node_id_by_ip :: CKadClientSearcher -> uint32_t -> tcpPort -> udpPort
 * 
 * > send_my_details KADEMLIA@_HELLO_REQ ip udpPort 1 0 NULL false // note about unbfuscateness
 * > FetchNodeID_Struct sRequest = { ip, tcpPort, ::GetTickCount() + SEC2MS(60), requester };
 * add sRequest to list of fetchNodeIDRequests
 *
 */

/* send_packet :: CMemFile& -> uint8_t -> uint32_t -> uint16_t -> CKadUDPKey& -> CUInt128*
 * send_packet    data         opcode     dest_host   dest_port   target_key     crypt_target_id =
 * 
 * track_out_packet dest_host opcode
 * packet = CPacket(data, OP_KADEMLIAHEADER, opcode)
 * if (sizeof packet) > 200
 * 	pack packet
 *
 * uint8_t cryptData[16]
 * uint8_t *cryptKey
 *
 *
 */
