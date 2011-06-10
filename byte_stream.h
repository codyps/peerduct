#ifndef BYTE_STREAM_H_
#define BYTE_STREAM_H_

struct bytes {

};

void bytes_encode_u8(struct bytes *b, uint8_t x);
void bytes_encode_u16(struct bytes *b, uint16_t x);
void bytes_encode_u32(struct bytes *b, uint32_t x);
void bytes_encode_u64(struct bytes *b, uint64_t x);
void bytes_encode_bytes(struct bytes *b, void *buf, size_t len);

uint8_t bytes_decode_u8(struct bytes *b);

#endif
