#ifndef EC_H_
#define EC_H_
/*!
 * External Connection Protocol, *Mule's communication protocol.
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
 * @group ec_opcode.h
 * Defines opcodes as well as related types and methods for the External
 * Connections (EC) protocol used by *Mule
 */
#include <stdint.h>
typedef uint8_t ec_opcode_t;

enum ec_opcode_e {
	
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



#endif
