#ifndef _KT_SHA204_H_
	#define _KT_SHA204_H_
	void sha204c_calculate_crc(uint16_t length, uint8_t *data, uint8_t *crc);
	void sha204h_calculate_sha256(int32_t len, uint8_t *message, uint8_t *digest);
#endif
