/*
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

#include<err.h>
#include<errno.h>
#include<fcntl.h>
#include<inttypes.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include <termios.h>

#include<tee_client_api.h>
/* To the the UUID (found the the TA's h-file(s)) */
#include <signer-tee_ta.h>

/**
 *
 * @param c
 * @return
 */
int get_one_character(char* c)
{
    struct termios tmbuf,tmsave;

    if (tcgetattr(0, &tmbuf)) {
        return -1;
    }

    memmove(&tmsave, &tmbuf, sizeof(tmbuf));

    tmbuf.c_lflag &= ~ICANON;
    tmbuf.c_cc[VMIN] = 1;
    tmbuf.c_cc[VTIME] = 0;

    if (tcsetattr(0, TCSANOW, &tmbuf)) {
        return -1;
    }

    if (read(0, c, 1) != 1) {
        return -1;
    }

    if (tcsetattr(0, TCSANOW, &tmsave)) {
        return -1;
    }

    return 0;
}

/**
 *
 * @param buf
 * @param len
 */
void print_buffer(void *buf, size_t len){
    uint8_t *pbyte = buf;

    for(int i = 0; i < len; i ++)
        printf("%02"PRIX8"", pbyte[i]);
    printf("\n");
}

/**
 *
 * @param res
 * @param eo
 * @param str
 */
static void teec_err(TEEC_Result res, uint32_t eo, const char *str)
{
    errx(1, "%s: %#" PRIx32 " (error origin %#" PRIx32 ")", str, res, eo);
}

/**
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[])
{
    TEEC_Result res;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_SIGNER_TEE_UUID;
    int exit = 0, fd;
    char ch;
    uint32_t err_origin;
    uint8_t ecdsa_pubkey_x[32]; // place to store ECDSA public key X part
    uint8_t ecdsa_pubkey_y[32]; // place to store ECDSA public key Y part

    if ((res = TEEC_InitializeContext(NULL, &ctx)) != TEEC_SUCCESS) {
        teec_err(res, 0, "TEEC_InitializeContext(NULL, x)");
    }

    if ((res = TEEC_OpenSession(&ctx, &sess, &uuid,
                                TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin)) != TEEC_SUCCESS) {
        teec_err(res, err_origin, "TEEC_OpenSession(TEEC_LOGIN_PUBLIC)");
    }

    while (!exit) {
        printf("1 - Get ECDSA key\n");
        printf("2 - Sign\n");
        printf("3 - Verify\n");
        printf("0 - Exit\n");
        fflush(stdout);
        if (get_one_character(&ch)) {
            continue;
        }
        printf("\n");
        switch (ch) {
            case '1':
                //Save public key
                memset(&op, 0, sizeof(op));
                op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_OUTPUT, // ECDSA pubkey X part
                                                 TEEC_MEMREF_TEMP_OUTPUT, // ECDSA pubkey Y part
                                                 TEEC_NONE,
                                                 TEEC_NONE);
                op.params[0].tmpref.buffer = ecdsa_pubkey_x;
                op.params[0].tmpref.size = sizeof(ecdsa_pubkey_x);
                op.params[1].tmpref.buffer = ecdsa_pubkey_y;
                op.params[1].tmpref.size = sizeof(ecdsa_pubkey_y);

                if ((res = TEEC_InvokeCommand(&sess, TA_SIGNER_TEE_CMD_GET_KEY, &op, &err_origin)) != TEEC_SUCCESS) {
                    teec_err(res, err_origin, "TEEC_InvokeCommand(TA_SIGNER_TEE_CMD_GET_KEY)");
                }

                printf("ECDSA Pubkey X: ");
                print_buffer(op.params[0].tmpref.buffer, op.params[0].memref.size);
                printf("ECDSA Pubkey Y: ");
                print_buffer(op.params[1].tmpref.buffer, op.params[1].memref.size);

                if ((fd = open("pub.txt", O_CREAT | O_WRONLY, 0644)) > 0){
                    if (write(fd, op.params[0].tmpref.buffer /* pubkey X*/, op.params[0].memref.size) > 0) {
                        if (write(fd, op.params[1].tmpref.buffer /* pubkey Y */, op.params[1].memref.size) > 0) {
                            printf("ECDSA public key saved in pub.txt\n");
                        }
                    }
                    close(fd);
                }
                else {
                    fprintf(stderr, "%s while opening pub.txt\n", strerror(errno));
                }
                break;
            case '2':
                break;
            case '3':
                break;
            case '0':
                exit = 1;
                break;
            default:
                break;
        }
    }

    TEEC_CloseSession(&sess);
    TEEC_FinalizeContext(&ctx);

    return 0;
}
