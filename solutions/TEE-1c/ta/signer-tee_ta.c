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

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <string.h>

#include <signer-tee_ta.h>

struct ecdsa_instance {
    void *key_obj_id;
    uint32_t key_obj_id_size;
};

struct ecdsa_session {
    TEE_ObjectHandle key;
};

static const char* filename_key = "gugus.key";

/**
 * Retrieve EC public key
 * @param sess_ctx
 * @param param_types
 * @param params
 * @return
 */
static TEE_Result get_ecdsa_key(void *sess_ctx, uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS])
{
    DMSG("has been called");

    TEE_Result res = TEE_ERROR_BAD_PARAMETERS;
    struct ecdsa_session *sp = sess_ctx;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_OUTPUT, // ECDSA public key X
                                               TEE_PARAM_TYPE_MEMREF_OUTPUT, // ECDSA public key Y
                                               TEE_PARAM_TYPE_NONE,
                                               TEE_PARAM_TYPE_NONE);

    if (exp_param_types != param_types) {
        EMSG("Expected: 0x%x, got: 0x%x", exp_param_types, param_types);
        goto out;
    }

    if (sp->key == NULL) {
        EMSG("Session key object is NULL");
        goto out;
    }

    // get the X value
    if ((res = TEE_GetObjectBufferAttribute(sp->key,
                                            TEE_ATTR_ECC_PUBLIC_VALUE_X,
                                            params[0].memref.buffer, &(params[0].memref.size))) != TEE_SUCCESS) {
        EMSG("Call to TEE_GetObjectBufferAttribute TEE_ATTR_ECC_PUBLIC_VALUE_X fail, res=0x%08x", res);
        goto out;
    }

    // get the Y value
    if ((res = TEE_GetObjectBufferAttribute(sp->key,
                                            TEE_ATTR_ECC_PUBLIC_VALUE_Y,
                                            params[1].memref.buffer, &(params[1].memref.size))) != TEE_SUCCESS) {
        EMSG("Call to TEE_GetObjectBufferAttribute TEE_ATTR_ECC_PUBLIC_VALUE_Y fail, res=0x%08x", res);
        goto out;
    }

out:
    return res;
}


/**
 * Sign operation
 * @param sess_ctx
 * @param param_types
 * @param params
 * @return
 */
static TEE_Result sign(void *sess_ctx, uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS])
{
    DMSG("has been called");

    TEE_Result res = TEE_ERROR_BAD_PARAMETERS;
    TEE_OperationHandle op = TEE_HANDLE_NULL;
    struct ecdsa_session *sp = sess_ctx;
    void *sha256_dgst = TEE_Malloc(sizeof(uint8_t)*32, TEE_MALLOC_FILL_ZERO);
    uint32_t sha256_dgst_len = sizeof(uint8_t)*32;

    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,     // input which will be signed
                                               TEE_PARAM_TYPE_MEMREF_OUTPUT,    // signature value (r, s)
                                               TEE_PARAM_TYPE_NONE,
                                               TEE_PARAM_TYPE_NONE);

    if (exp_param_types != param_types) {
        EMSG("Expected: 0x%x, got: 0x%x", exp_param_types, param_types);
        goto out;
    }

    if (!sp->key){
        EMSG("No ECDSA key found, exiting...\n");
        goto out;
    }

    // SHA256 digest mode
    if ((res = TEE_AllocateOperation(&op, TEE_ALG_SHA256, TEE_MODE_DIGEST, 0)) != TEE_SUCCESS) {
        EMSG("Call to TEE_AllocateOperation TEE_ALG_SHA256/TEE_MODE_DIGEST fail, res=0x%08x", res);
        goto out;
    }

    // calculate digest
    if ((res = TEE_DigestDoFinal(op, params[0].memref.buffer, params[0].memref.size, sha256_dgst, &sha256_dgst_len)) != TEE_SUCCESS) {
        EMSG("Call to TEE_DigestDoFinal fail, res=0x%08x", res);
        goto out;
    }

    TEE_FreeOperation(op);

    // sign with ECDSA P256 NiST
    if ((res = TEE_AllocateOperation(&op, TEE_ALG_ECDSA_P256, TEE_MODE_SIGN, 256)) != TEE_SUCCESS) {
        EMSG("Call to TEE_AllocateOperation TEE_ALG_ECDSA_P256/TEE_MODE_SIGN fail, res=0x%08x", res);
        goto out;
    }

    // set key for signing
    if ((res = TEE_SetOperationKey(op, sp->key)) != TEE_SUCCESS) {
        EMSG("Call to TEE_SetOperationKey fail, res=0x%08x", res);
        goto out;
    }

    // do asymetric sign
    if ((res = TEE_AsymmetricSignDigest(op, NULL, 0, sha256_dgst, sha256_dgst_len,
                                        params[1].memref.buffer, &(params[1].memref.size))) != TEE_SUCCESS) {
        EMSG("Call to TEE_AsymmetricSignDigest fail, res=0x%08x", res);
        goto out;
    }

out:
    if (op != TEE_HANDLE_NULL) {
        TEE_FreeOperation(op);
    }

    TEE_Free(sha256_dgst);

    return res;
}

/*
 * Create ECDSA key-pair for TA instance (one per device)
 */
TEE_Result TA_CreateEntryPoint(void)
{
    DMSG("has been called");

    TEE_ObjectHandle hobj_storage = TEE_HANDLE_NULL;
    TEE_ObjectHandle hobj_key = TEE_HANDLE_NULL;
    TEE_Result res = TEE_ERROR_OUT_OF_MEMORY;
    struct ecdsa_instance *inst = NULL;

    if ((inst = TEE_Malloc(sizeof(struct ecdsa_instance), TEE_MALLOC_FILL_ZERO)) == NULL) {
        EMSG("TEE_Malloc failed");
        goto out;
    }

    if ((inst->key_obj_id = TEE_Malloc(sizeof(uint8_t) * strlen(filename_key), TEE_MALLOC_FILL_ZERO)) == NULL) {
        EMSG("TEE_Malloc failed");
        goto out;
    }

    inst->key_obj_id_size = strlen(filename_key);

    // try to load ECDSA key set
    if ((res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
                                        inst->key_obj_id, inst->key_obj_id_size,
                                        TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_SHARE_READ,
                                        &hobj_storage)) == TEE_ERROR_ITEM_NOT_FOUND) {
        DMSG("no ECDSA keypair found, creating a new one");

        if ((res = TEE_AllocateTransientObject(TEE_TYPE_ECDSA_KEYPAIR, 521, &hobj_key)) != TEE_SUCCESS) {
            EMSG("Call to TEE_AllocateTransientObject TEE_TYPE_ECDSA_KEYPAIR fail, res=0x%08x", res);
            goto out;
        }

        // use NiST P256 for ECC curve
        TEE_Attribute attrs[1];
        TEE_InitValueAttribute(attrs, TEE_ATTR_ECC_CURVE, TEE_ECC_CURVE_NIST_P256, 0);

        if ((res = TEE_GenerateKey(hobj_key, 256, attrs, sizeof(attrs) / sizeof(TEE_Attribute)))  != TEE_SUCCESS) {
            EMSG("Call to TEE_GenerateKey fail, res=0x%08x", res);
            goto out;
        }

        // store the created key
        if ((res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE,
                                              inst->key_obj_id /* objectid aka file name*/, inst->key_obj_id_size,
                                              TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_SHARE_READ,
                                              hobj_key,
                                              NULL, 0,
                                              &hobj_storage)) != TEE_SUCCESS) {
            EMSG("Call to TEE_CreatePersistentObject fail, res=0x%08x", res);
            goto out;
        }
    }
    else if (res != TEE_SUCCESS) {
        EMSG("Call to TEE_OpenPersistentObject fail, res=0x%08x", res);
        goto out;
    }
    else {
        DMSG("ECDSA keypair found");
    }

    TEE_SetInstanceData(inst);

out:
    TEE_CloseObject(hobj_key);
    TEE_CloseObject(hobj_storage);

    if (res != TEE_SUCCESS) {
        if (inst != NULL) {
            if (inst->key_obj_id != NULL) {
                TEE_Free(inst->key_obj_id);
            }

            TEE_Free(inst);
        }
    }

    return res;
}

/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void TA_DestroyEntryPoint(void)
{
	DMSG("has been called");

	struct ecdsa_instance *inst = (struct ecdsa_instance *)TEE_GetInstanceData();

    if (inst != NULL) {
        if (inst->key_obj_id != NULL) {
            TEE_Free(inst->key_obj_id);
        }

        TEE_Free(inst);
        TEE_SetInstanceData(NULL);
    }
}

/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA. In this function you will normally do the global initialization for the
 * TA.
 */
TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types, TEE_Param __maybe_unused param[TEE_NUM_PARAMS], void **sess_ctx)
{
    DMSG("has been called");

    TEE_Result res;
    const struct ecdsa_instance *inst;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
                                               TEE_PARAM_TYPE_NONE,
                                               TEE_PARAM_TYPE_NONE,
                                               TEE_PARAM_TYPE_NONE);

    if (exp_param_types != param_types) {
        EMSG("Expected: 0x%x, got: 0x%x", exp_param_types, param_types);
        return TEE_ERROR_BAD_PARAMETERS;
    }

    // allocate session
    struct ecdsa_session *sess = TEE_Malloc(sizeof(struct ecdsa_session), TEE_MALLOC_FILL_ZERO);
    if (!sess) {
        EMSG("TEE_Malloc failed");
        return TEE_ERROR_OUT_OF_MEMORY;
    }

    inst = TEE_GetInstanceData();

    // load the key pairs
    if ((res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
                                        inst->key_obj_id, inst->key_obj_id_size,
                                        TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_SHARE_READ,
                                        &sess->key)) != TEE_SUCCESS) {
        EMSG("Call to TEE_CreatePersistentObject fail, res=0x%08x", res);
        return res;
    }

    *sess_ctx = sess;

    return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void TA_CloseSessionEntryPoint(void *sess_ctx)
{
    DMSG("has been called");

    struct ecdsa_session *sp = sess_ctx;

    if (sp != NULL) {
        TEE_CloseObject(sp->key);
        TEE_Free(sp);
    }
}

/*
 * Called when a TA is invoked. sess_ctx hold that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the paramters
 * comes from normal world.
 */
TEE_Result TA_InvokeCommandEntryPoint(void *sess_ctx, uint32_t cmd_id, uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS])
{
    DMSG("has been called");

    switch(cmd_id){
        case TA_SIGNER_TEE_CMD_GET_KEY:
            return get_ecdsa_key(sess_ctx, param_types, params);
        case TA_SIGNER_TEE_CMD_SIGN:
            return sign(sess_ctx, param_types, params);
        default:
            return TEE_ERROR_NOT_SUPPORTED;
    }
}
