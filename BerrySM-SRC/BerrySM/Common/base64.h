#pragma once

void base64_encode(char* encoded, int encoded_len, unsigned char const* bytes_to_encode, unsigned int len);
void base64_decode(char* decoded, int decoded_len, const char* encoded_string);
