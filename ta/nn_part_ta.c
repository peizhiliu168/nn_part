/*
 * Copyright (c) 2016, Linaro Limited
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

#include <assert.h>

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <nn_part_ta.h>
#include <network.h>
#include <matrix.h>
#include <math_TA.h>
#include <data.h>

/*
 * Called when the instance of the TA is created. This is the first call in
 * the TA.
 */
TEE_Result TA_CreateEntryPoint(void)
{
	DMSG("has been called");

	return TEE_SUCCESS;
}

/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void TA_DestroyEntryPoint(void)
{
	DMSG("has been called");
}

/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA. In this function you will normally do the global initialization for the
 * TA.
 */
TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
		TEE_Param __maybe_unused params[4],
		void __maybe_unused **sess_ctx)
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	// DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Unused parameters */
	(void)&params;
	(void)&sess_ctx;

	/*
	 * The DMSG() macro is non-standard, TEE Internal API doesn't
	 * specify any means to logging from a TA.
	 */
	// IMSG("Hello World!\n");
	TEE_InitSctrace();
	
	// Allocate network
	nn = allocate_network();
	// DMSG("Finished allocating network.\n");

	// Allocate crypto
	nn->key_size = 256;
	TEE_AllocateTransientObject(TEE_TYPE_AES, nn->key_size, &(nn->aeskey));
	TEE_GenerateKey(nn->aeskey, nn->key_size, (TEE_Attribute *)NULL, 0);
	// DMSG("Finished generating key.\n");


	// TEE_Result res;
	// TEE_OperationHandle op_enc, op_dec;

	// res = TEE_AllocateOperation(&op_enc, TEE_ALG_AES_CTR, TEE_MODE_ENCRYPT, key_size);
	// res = TEE_AllocateOperation(&op_dec, TEE_ALG_AES_CTR, TEE_MODE_DECRYPT, key_size);
	// res = TEE_SetOperationKey(op_enc, aeskey);
	// res = TEE_SetOperationKey(op_dec, aeskey);

	// nn->enc_handle = op_enc;
	// nn->dec_handle = op_dec;


	/* If return value != TEE_SUCCESS the session will not be created. */
	return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void TA_CloseSessionEntryPoint(void __maybe_unused *sess_ctx)
{
	(void)&sess_ctx; /* Unused parameter */
	IMSG("Goodbye!\n");
}

static TEE_Result inc_value(uint32_t param_types,
	TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	// DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	IMSG("Got value: %u from NW", params[0].value.a);
	params[0].value.a++;
	IMSG("Increase value to: %u", params[0].value.a);


	// printf("----------------------------------------------------\n");
	// for (int i=0; i < data_loader->N; ++i ) {
	// 	for (int j=0; j < data_loader->feature_size; ++j) {
	// 		printf("%ld ", data_loader->features->vals[i][j]);
	// 	}
	// 	printf("\n");
	// }
	// printf("\n");


	// for (int i=0; i < data_loader->N; ++i ) {
	// 	for (int j=0; j < data_loader->classes; ++j) {
	// 		printf("%ld ", data_loader->labels->vals[i][j]);
	// 	}
	// 	printf("\n");
	// }
	// printf("----------------------------------------------------\n");

	// for (int i=0; i < 10; ++i) {
    //     DMSG("sample: %.6f\n", labels->vals[0][i]);
    // }
	// DMSG("calculating number: %d\n", (int) ta_ln(1e-2));
	train(10);
	// float loss = forward(features, labels);
	// DMSG("cost: %d\n", (int) loss);
	// backward(labels);

	// Destroy crypto
	// TEE_FreeOperation(nn->enc_handle);
	// TEE_FreeOperation(nn->dec_handle);

	TEE_FreeTransientObject(nn->aeskey);

	// Destroy network
	destroy_network();
	TEE_GetSctrace(4);

	return TEE_SUCCESS;
}

static TEE_Result dec_value(uint32_t param_types,
	TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	// DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	IMSG("Got value: %u from NW", params[0].value.a);
	params[0].value.a--;
	IMSG("Decrease value to: %u", params[0].value.a);

	return TEE_SUCCESS;
}

static TEE_Result share_mem(uint32_t param_types,
	TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INOUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	// DMSG("Share Mem command called.\n");
	
	if (param_types != exp_param_types) {
		// DMSG("Bad parameters\n");
		return TEE_ERROR_BAD_PARAMETERS;
	}

	nn->shmem = params[0].memref.buffer;
	nn->shmem_size = params[0].memref.size;
	// DMSG("Set shared memory pointer. %d, size: %d, contents: %d\n", nn->shmem, nn->shmem_size, *((uint32_t*) nn->shmem));

	// Initialize network
	init_network();
	// DMSG("Initialized network.\n");
	

	return TEE_SUCCESS;
}

static TEE_Result train_net(uint32_t param_types,
	TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INOUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);
	// DMSG("train_net has been called");

	if (param_types != exp_param_types) {
		// DMSG("Bad parameters\n");
		return TEE_ERROR_BAD_PARAMETERS;
	}
	
	nn->shmem = params[0].memref.buffer;
	nn->shmem_size = params[0].memref.size;

	train(10);

	TEE_FreeTransientObject(nn->aeskey);

	// Destroy network
	destroy_network();
	TEE_GetSctrace(4);

	return TEE_SUCCESS;
}


static TEE_Result send_data(uint32_t param_types, TEE_Param params[4]){
	assert(nn != NULL);
	TEE_AddSctrace(555);

	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
						   TEE_PARAM_TYPE_VALUE_INPUT,
						   TEE_PARAM_TYPE_MEMREF_INPUT,
						   TEE_PARAM_TYPE_VALUE_INPUT);
	// DMSG("send_data has been called");

	if (param_types != exp_param_types) return TEE_ERROR_BAD_PARAMETERS;

	float *features = params[0].memref.buffer; // a single row is all the pixels 
	int features_size = params[0].memref.size / sizeof(float);

	int feature_rows = params[1].value.a;
	int feature_cols = params[1].value.b;

	float *labels = params[2].memref.buffer; // one hot encoding
	int labels_size = params[2].memref.size  / sizeof(float);

	int label_rows = params[3].value.a;
	int label_cols = params[3].value.b;

	// printf("----------------------------------------------------\n");
	// for (int i=0; i < feature_rows * feature_cols; ++i ) {
	// 	printf("%ld ", features[i]);
	// }
	// printf("\n");


	// for (int i=0; i < label_rows * label_cols; ++i ) {
	// 	printf("%ld ", labels[i]);
	// }
	// printf("----------------------------------------------------\n");

	matrix_t* wrapped_features = wrap_data(features, features_size, feature_rows, feature_cols);
	matrix_t* wrapped_labels = wrap_data(labels, labels_size, label_rows, label_cols);

	init_data(wrapped_features, wrapped_labels, nn->batch_size);
	// DMSG("send_data has finished");

	TEE_AddSctrace(555);
	return TEE_SUCCESS;
}

/*
 * Called when a TA is invoked. sess_ctx hold that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the paramters
 * comes from normal world.
 */
TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
			uint32_t cmd_id,
			uint32_t param_types, TEE_Param params[4])
{
	(void)&sess_ctx; /* Unused parameter */

	switch (cmd_id) {
	case TA_NN_PART_CMD_INC_VALUE:
		return inc_value(param_types, params);
	case TA_NN_PART_CMD_DEC_VALUE:
		return dec_value(param_types, params);
	case TA_NN_PART_CMD_SHARE_MEM:
		return share_mem(param_types, params);
	case TA_NN_PART_CMD_SEND_DATA:
		return send_data(param_types, params);
	case TA_NN_PART_CMD_TRAIN_NET:
		return train_net(param_types, params);
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
