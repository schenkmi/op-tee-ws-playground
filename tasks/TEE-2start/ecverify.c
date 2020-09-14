/**
 * ECDSA https://de.wikipedia.org/wiki/Elliptic_Curve_DSA
 *
 * Copyright (c) 2020, Michael Schenk
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ec.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
#include<err.h>
#include<errno.h>
#include<fcntl.h>
#include<inttypes.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

// EC Pubkey TEE_ECC_CURVE_NIST_P256  each 32 bytes (256 bits)
// hexdump -e '32/1 "%02X""\n"' pub.txt
static const char *ecdsa_pubkey_x ="0FCA14C5FFFA234B737AFF8C430DAC95768240370033F58D4A06E7AA522D00A5";
static const char *ecdsa_pubkey_y = "1AEC4CEBC8B6652D9C5368DABAACA7D02F1A8C14037A6E48FD47EF6B6D8E10CF";
// EC signature R & S component each 32 bytes (256 bits)
// hexdump -e '32/1 "%02X""\n"' signature.txt
static const char *ecdsa_signature_r = "71C99AD481E92424D0E0A58A13F71F69E5978D5F8AC603972E6DB5FD9BC546EA";
static const char *ecdsa_signature_s = "B3D2C189CE4DE13C31ACF64F4418F5DE79AAD951DDFDEED6DD7854F5EBAE01C7";

static const char* string_to_sign = "Noser Engineering";

int main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[])
{
    int ret;
    BIO *outbio = NULL;

    OpenSSL_add_all_algorithms();
    OpenSSL_add_all_ciphers();
    OpenSSL_add_all_digests();
    ERR_load_BIO_strings();
    ERR_load_crypto_strings();

    outbio = BIO_new(BIO_s_file());
    outbio = BIO_new_fp(stdout, BIO_NOCLOSE);

    BIO_printf(outbio, "Skeleton\n");

    BIO_free_all(outbio);

    return 0;
}
