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

/*
 * Create ECDSA key-pair for TA instance (one per device)
 */
TEE_Result TA_CreateEntryPoint(void)
{
    DMSG("has been called");

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

    res = TEE_SUCCESS;

    TEE_SetInstanceData(inst);

out:
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

    // allocate session
    struct ecdsa_session *sess = TEE_Malloc(sizeof(struct ecdsa_session), TEE_MALLOC_FILL_ZERO);
    if (!sess) {
        EMSG("TEE_Malloc failed");
        return TEE_ERROR_OUT_OF_MEMORY;
    }

    inst = TEE_GetInstanceData();

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

    return TEE_ERROR_NOT_SUPPORTED;
}
