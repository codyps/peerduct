#ifndef MULE_EC_H_
#define MULE_EC_H_
/*!
 * External Connection Protocol, *Mule's communication protocol, application layer.
 *
 * Data Types (from amule/docs/EC_Protocol.txt)
 * 	integer types: u8, be16, be32
 * 	strings: UTF-8 with trailing '\0'
 * 	boolean: when reading, lack = false & exsistence = true
 * 		 when writing, u8 with zero = false & non-zero = true
 * 	md5 hashes: msb first
 *	floting point: converted to strings and sent as such. decimal is always
 * 		       '.'
 */

struct ec_pkt {
	ec_flags_t  flags;
	ec_opcode_t opcode;
	ec_tagct_t  tag_ct;

	struct list_head children /* struct ec_tag */;
};

/*!
 * @group ec_opcode
 * Defines opcodes as well as related types and methods for the External
 * Connections (EC) protocol used by *Mule
 */
#include <stdint.h>
typedef uint8_t ec_opcode_t;

enum ec_opcode_e {
	EC_OP_AUTH_REQ = 0x02

};


/*!
 * @group ec_tag.h
 * Defines tagtypes as well as related types and methods for the External
 * Connections (EC) protocol used by *Mule
 */
struct ec_tag {
	ec_tag_t       tag;
	ec_tagtype_t   type;
	union ec_data  data[]; /* this is a hack so i can append it to the
				  struct without automatically allocating the
				  space */
};

enum ec_tag_e {
	EC_TAG_CLIENT_NAME = 0x06,
	EC_TAG_PASSWD_HASH = 0x04,
	EC_TAG_PROTOCOL_VERSION = 0x0c,
	EC_TAG_CLIENT_VERSION = 0x08,
	EC_TAG_VERSION_ID = 0x0e	
};

enum ec_tagtype_e {
	EC_TAGTYPE_HASH16 = 0x01,
	EC_TAGTYPE_STRING = 0x02,
	EC_TAGTYPE_UINT32,
	EC_TAGTYPE_FLOAT32,
	EC_TAGTYPE_BOOL,
	EC_TAGTYPE_BOOLARRAY,
	EC_TAGTYPE_BLOB,
	EC_TAGTYPE_UINT16,
	EC_TAGTYPE_UINT8,
	EC_TAGTYPE_BSOB,
	EC_TAGTYPE_UINT64,

	EC_TAGTYPE_STR1 = 0x11,

	/* Sequential to STR22, only up to STR16 should be used. */

};

#endif
