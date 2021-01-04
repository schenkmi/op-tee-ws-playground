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
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void print_buffer(void *buf, size_t len)
{
    uint8_t *pbyte = buf;

    for(size_t i = 0; i < len; i ++)
        printf("%02"PRIX8"", pbyte[i]);
    printf("\n");
}

/**
 * Calculate hash of type EVP_MD*
 * @param type
 * @param src
 * @param size
 * @param digest
 * @return
 */
int calc_hash(const EVP_MD* type, const void* src, const size_t size, void** digest)
{
    unsigned int len = 0;
    unsigned char calc_hash[EVP_MAX_KEY_LENGTH];

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    //EVP_MD_CTX_set_flags(ctx, EVP_MD_CTX_FLAG_ONESHOT | EVP_MD_CTX_FLAG_FINALISE);

    if (!EVP_DigestInit_ex(ctx,  type, NULL)) {
        printf("EVP_DigestInit_ex() failed! Error: %s\n", ERR_error_string(ERR_get_error(), NULL));
        goto cleanup;
    }

    if (!EVP_DigestUpdate(ctx, src, size)) {
        printf("EVP_DigestUpdate() failed! Error: %s\n", ERR_error_string(ERR_get_error(), NULL));
        goto cleanup;
    }

    if (!EVP_DigestFinal_ex(ctx, calc_hash, &len)) {
        printf("EVP_DigestFinal_ex() failed! Error: %s\n", ERR_error_string(ERR_get_error(), NULL));
        goto cleanup;
    }

    *digest = OPENSSL_zalloc(len);
    memcpy(*digest, calc_hash, len);

cleanup:
    EVP_MD_CTX_free(ctx);
    return len;
}

unsigned char* hexstr_to_char(const char* hexstr)
{
    size_t len = strlen(hexstr);
    if (len % 2 != 0)
        return NULL;
    size_t final_len = len / 2;
    unsigned char* chrs = (unsigned char*)malloc((final_len+1) * sizeof(*chrs));
    for (size_t i=0, j=0; j<final_len; i+=2, j++)
        chrs[j] = (hexstr[i] % 32 + 9) % 25 * 16 + (hexstr[i+1] % 32 + 9) % 25;
    chrs[final_len] = '\0';
    return chrs;
}

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
    void *digest = NULL;
    int digestlen = 0;

    OpenSSL_add_all_algorithms();
    OpenSSL_add_all_ciphers();
    OpenSSL_add_all_digests();
    ERR_load_BIO_strings();
    ERR_load_crypto_strings();

    outbio = BIO_new(BIO_s_file());
    outbio = BIO_new_fp(stdout, BIO_NOCLOSE);

    // calculate SHA256 digest from string_to_sign
    digestlen = calc_hash(EVP_sha256(), string_to_sign, strlen(string_to_sign), &digest);
    BIO_printf(outbio, "SHA256 Digest :    ");
    print_buffer(digest, digestlen);

    // convert ECDSA public key X/Y into big number
    BIGNUM *ecdsa_pubkey_bn_x = NULL;
    BIGNUM *ecdsa_pubkey_bn_y = NULL;

    // if big number var is NULL, it will be allocated internally
    BN_hex2bn(&ecdsa_pubkey_bn_x, ecdsa_pubkey_x);
    BN_hex2bn(&ecdsa_pubkey_bn_y, ecdsa_pubkey_y);

    char *value = BN_bn2hex(ecdsa_pubkey_bn_x);
    BIO_printf(outbio, "ECDSA Pubkey X:    %s\n", value);
    OPENSSL_free(value);

    value = BN_bn2hex(ecdsa_pubkey_bn_y);
    BIO_printf(outbio, "ECDSA Pubkey Y:    %s\n", value);
    OPENSSL_free(value);

    // create new EC key
    EC_KEY *ec_key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    EC_KEY_set_asn1_flag(ec_key, OPENSSL_EC_NAMED_CURVE);

    // assigne public key values
    if (!EC_KEY_set_public_key_affine_coordinates(ec_key, ecdsa_pubkey_bn_x, ecdsa_pubkey_bn_y)) {
        BIO_printf(outbio, "EC_KEY_set_public_key_affine_coordinates() failed! Error: %s\n", ERR_error_string(ERR_get_error(), NULL));
    }

    // Converting the EC key into a PKEY structure
    EVP_PKEY *pkey = EVP_PKEY_new();

    // EVP_PKEY_assign_EC_KEY free ec_key
    if (!EVP_PKEY_assign_EC_KEY(pkey, ec_key)) {
        BIO_printf(outbio, "EVP_PKEY_assign_EC_KEY() failed! Error: %s\n", ERR_error_string(ERR_get_error(), NULL));
    }

    // print public key as PEM
    if (!PEM_write_bio_PUBKEY(outbio, pkey)) {
        BIO_printf(outbio, "Error writing public key data in PEM format\n");
    }

    EVP_PKEY_CTX *key_ctx = EVP_PKEY_CTX_new(pkey, NULL);

    ECDSA_SIG *ecdsa_sig = ECDSA_SIG_new();
    // convert ecdsa signature R/S values into big number
    BIGNUM *ecdsa_signature_bn_r = NULL;
    BIGNUM *ecdsa_signature_bn_s = NULL;

    // if big number var is NULL, it will be allocated internally
    BN_hex2bn(&ecdsa_signature_bn_r, ecdsa_signature_r);
    BN_hex2bn(&ecdsa_signature_bn_s, ecdsa_signature_s);

    value = BN_bn2hex(ecdsa_signature_bn_r);
    BIO_printf(outbio, "ECDSA Signature R: %s\n", value);
    OPENSSL_free(value);

    value = BN_bn2hex(ecdsa_signature_bn_s);
    BIO_printf(outbio, "ECDSA Signature S: %s\n\n", value);
    OPENSSL_free(value);

    // set ECDSA signature R and S part
    if (!ECDSA_SIG_set0(ecdsa_sig, ecdsa_signature_bn_r, ecdsa_signature_bn_s)) {
        BIO_printf(outbio, "ECDSA_SIG_set0() failed! Error: %s\n", ERR_error_string(ERR_get_error(), NULL));
    }

    // EC signatures are always in plain signature format for EC curves but EVP_PKEY_verify whants ASN.1
    int ecdsa_sig_asn1_len;
    unsigned char *ecdsa_sig_asn1 = NULL;

    // ASN.1 encode the signature
    ecdsa_sig_asn1_len = i2d_ECDSA_SIG(ecdsa_sig, &ecdsa_sig_asn1);

    if (!EVP_PKEY_verify_init(key_ctx)) {
        BIO_printf(outbio, "EVP_PKEY_verify_init() failed! Error: %s\n", ERR_error_string(ERR_get_error(), NULL));
    }
    if (!EVP_PKEY_CTX_set_signature_md(key_ctx, EVP_sha256())) {
        BIO_printf(outbio, "EVP_PKEY_CTX_set_signature_md() failed! Error: %s\n", ERR_error_string(ERR_get_error(), NULL));
    }

    // verify the signatore
    ret = EVP_PKEY_verify(key_ctx,  ecdsa_sig_asn1, ecdsa_sig_asn1_len, digest, digestlen);
    if (ret == 1) {
        BIO_printf(outbio, "EVP_PKEY_verify(): returned SUCCESS\n\n");
    } else if (ret == 0) {
        BIO_printf(outbio, "EVP_PKEY_verify(): returned FAILURE\n\n");
    } else {
        BIO_printf(outbio, "EVP_PKEY_verify() failed! Error: %s\n\n", ERR_error_string(ERR_get_error(), NULL));
        //ERR_print_errors(outbio);
    }

    EVP_PKEY_CTX_free(key_ctx);
    EVP_PKEY_free(pkey);
    OPENSSL_free(ecdsa_sig_asn1);
    BN_free(ecdsa_pubkey_bn_x);
    BN_free(ecdsa_pubkey_bn_y);
    BN_free(ecdsa_signature_bn_r);
    BN_free(ecdsa_signature_bn_s);
    OPENSSL_free(digest);
    BIO_free_all(outbio);

    return 0;
}