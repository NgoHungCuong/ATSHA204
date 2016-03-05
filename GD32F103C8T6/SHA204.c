#include "gd32f10x.h"
#include "sha204.h"

void sha204c_calculate_crc(uint16_t length, uint8_t *data, uint8_t *crc) {
	uint8_t counter;
	uint16_t crc_register = 0;
	uint16_t polynom = 0x8005;
	uint8_t shift_register;
	uint8_t data_bit, crc_bit;
	
	for (counter = 0; counter < length; counter++) {
		for (shift_register = 0x01; shift_register > 0x00; shift_register <<= 1) {
			data_bit = (data[counter] & shift_register) ? 1 : 0;
			crc_bit = crc_register >> 15;
			crc_register <<= 1;
			if (data_bit != crc_bit)
				crc_register ^= polynom;
		}
	}
	crc[0] = (uint8_t) (crc_register & 0x00FF);
	crc[1] = (uint8_t) (crc_register >> 8);
}

#define SHA256_BLOCK_SIZE   (64) 
#define rotate_right(value, places) ((value >> places) | (value << (32 - places)))

void sha204h_calculate_sha256(int32_t len, uint8_t *message, uint8_t *digest)
{
	int32_t j, swap_counter, len_mod = len % sizeof(int32_t);
	uint32_t i, w_index;
	int32_t message_index = 0;
	uint32_t padded_len = len + 8; // 8 bytes for bit length
	uint32_t bit_len = len * 8;
	uint32_t s0, s1;
	uint32_t t1, t2;
	uint32_t maj, ch;
	uint32_t word_value;
	uint32_t rotate_register[8];

	union {
		uint32_t w_word[SHA256_BLOCK_SIZE];
		uint8_t w_byte[SHA256_BLOCK_SIZE * sizeof(int32_t)];
	} w_union;

	uint32_t hash[] = {
		0x6a09e667, 0xbb67ae85, 0x3c6ef372,	0xa54ff53a,
		0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
	};

	const uint32_t k[] = {
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	};

	// Process message.
	while (message_index <= padded_len) {

		// Break message into 64-byte blocks.
		w_index = 0;
		do {
			// Copy message chunk of four bytes (size of integer) into compression array.
			if (message_index < (len - len_mod)) {
				for (swap_counter = sizeof(int32_t) - 1; swap_counter >= 0; swap_counter--)
					// No padding needed. Swap four message bytes to chunk array.
					w_union.w_byte[swap_counter + w_index] = message[message_index++];

				w_index += sizeof(int32_t);
			}
			else {
				// We reached last complete word of message {len - (len mod 4)}.
				// Swap remaining bytes if any, append '1' bit and pad remaining
				// bytes of the last word.
				for (swap_counter = sizeof(int32_t) - 1;
						swap_counter >= sizeof(int32_t) - len_mod; swap_counter--)
					w_union.w_byte[swap_counter + w_index] = message[message_index++];
				w_union.w_byte[swap_counter + w_index] = 0x80;
				for (swap_counter--; swap_counter >= 0; swap_counter--)
					w_union.w_byte[swap_counter + w_index] = 0;

				// Switch to word indexing.
				w_index += sizeof(int32_t);
				w_index /= sizeof(int32_t);

				// Pad last block with zeros to a block length % 56 = 0
				// and pad the four high bytes of "len" since we work only
				// with integers and not with long integers.
				while (w_index < 15)
					 w_union.w_word[w_index++] = 0;
				// Append original message length as 32-bit integer.
				w_union.w_word[w_index] = bit_len;
				// Indicate that the last block is being processed.
				message_index += SHA256_BLOCK_SIZE;
				// We are done with pre-processing last block.
				break;
			}
		} while (message_index % SHA256_BLOCK_SIZE);
		// Created one block.

		w_index = 16;
		while (w_index < SHA256_BLOCK_SIZE) {
			// right rotate for 32-bit variable in C: (value >> places) | (value << 32 - places)
			word_value = w_union.w_word[w_index - 15];
			s0 = rotate_right(word_value, 7) ^ rotate_right(word_value, 18) ^ (word_value >> 3);

			word_value = w_union.w_word[w_index - 2];
			s1 = rotate_right(word_value, 17) ^ rotate_right(word_value, 19) ^ (word_value >> 10);

			w_union.w_word[w_index] = w_union.w_word[w_index - 16] + s0 + w_union.w_word[w_index - 7] + s1;

			w_index++;
		}

		// Initialize hash value for this chunk.
		for (i = 0; i < 8; i++)
			rotate_register[i] = hash[i];

		// hash calculation loop
		for (i = 0; i < SHA256_BLOCK_SIZE; i++) {
			s0 = rotate_right(rotate_register[0], 2)
				^ rotate_right(rotate_register[0], 13)
				^ rotate_right(rotate_register[0], 22);
			maj = (rotate_register[0] & rotate_register[1])
				^ (rotate_register[0] & rotate_register[2])
				^ (rotate_register[1] & rotate_register[2]);
			t2 = s0 + maj;
			s1 = rotate_right(rotate_register[4], 6)
				^ rotate_right(rotate_register[4], 11)
				^ rotate_right(rotate_register[4], 25);
			ch =  (rotate_register[4] & rotate_register[5])
				^ (~rotate_register[4] & rotate_register[6]);
			t1 = rotate_register[7] + s1 + ch + k[i] + w_union.w_word[i];

			rotate_register[7] = rotate_register[6];
			rotate_register[6] = rotate_register[5];
			rotate_register[5] = rotate_register[4];
			rotate_register[4] = rotate_register[3] + t1;
			rotate_register[3] = rotate_register[2];
			rotate_register[2] = rotate_register[1];
			rotate_register[1] = rotate_register[0];
			rotate_register[0] = t1 + t2;
		}

	    // Add the hash of this block to current result.
		for (i = 0; i < 8; i++)
			hash[i] += rotate_register[i];
	}

	// All blocks have been processed.
	// Concatenate the hashes to produce digest, MSB of every hash first.
	for (i = 0; i < 8; i++) {
		for (j = sizeof(int32_t) - 1; j >= 0; j--, hash[i] >>= 8)
			digest[i * sizeof(int32_t) + j] = hash[i] & 0xFF;
	}
}
