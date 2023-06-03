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

#include <err.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/time.h>

/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>

/* For the UUID (found in the TA's h-file(s)) */
#include <nn_part_ta.h>
#include <network.h>
#include <math_TA.h>
#include <data.h>
#include <matrix.h>

#define ADD_SCTRACE_NUMBER 436
#define RUN_REE 0

TEEC_Session sess;

void ree_nn(void) {
	int N = 3000;
	int w = 28;
	int h = 28;
	int c = 1;
	int classes = 10;
	char identifier[] = "_c";
	char directory[] = "/root/mnist/images";


	struct timeval stop, start;
	gettimeofday(&start, NULL);

	matrix_t* features = create_matrix(N, w*h*c);
	matrix_t* labels = create_matrix(N, classes);
	get_data_matrix_from_image_dir(directory, N,
									w, h, c, 
									identifier, classes,
									features, labels);

	init_network();
	init_data(features, labels, nn->batch_size);
	train(10);
	destroy_network();
	
    gettimeofday(&stop, NULL);
	printf("took %lu us\n", (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec); 
}

void TEEC_SendData(char* directory, int N,
              int w, int h, int c, 
              char* identifier, int classes) {
	syscall(ADD_SCTRACE_NUMBER, 555);
	// setup RPC params
	TEEC_Operation op;
    uint32_t origin;
    TEEC_Result res;
	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
										TEEC_VALUE_INPUT,
										TEEC_MEMREF_TEMP_INPUT,
										TEEC_VALUE_INPUT);

	// fetch data and populate matrix
	matrix_t* features = create_matrix(N, w*h*c);
	matrix_t* labels = create_matrix(N, classes);
	get_data_matrix_from_image_dir(directory, N,
									w, h, c, 
									identifier, classes,
									features, labels);

	// create parameter buffers
	float* params0 = malloc(sizeof(float) * features->rows * features->cols);
	float* params1 = malloc(sizeof(float) * labels->rows * labels->cols);
	for (int i=0; i < features->rows; ++i){
		for (int j=0; j < features->cols; ++j){
			params0[i*features->cols + j] = features->vals[i][j];
		}	
	}
	for (int i=0; i < labels->rows; ++i){
		for (int j=0; j < labels->cols; ++j){
			params1[i*labels->cols + j] = labels->vals[i][j];
		}	
	}


	// set the network params	
	op.params[0].tmpref.buffer = params0;
	op.params[0].tmpref.size = sizeof(float) * features->rows * features->cols;
	op.params[1].value.a = features->rows; // a --> number of rows
	op.params[1].value.b = features->cols; // b --> number of columns
	op.params[2].tmpref.buffer = params1;
	op.params[2].tmpref.size = sizeof(float) * labels->rows * labels->cols;
	op.params[3].value.a = labels->rows; // a --> number of rows
	op.params[3].value.b = labels->cols; // b --> number of columns

	// printf("----------------------------------------------------\n");
	// for (int i=0; i < features->rows * features->cols; ++i ) {
	// 	printf("%f ", op.params[0].tmpref.buffer);
	// }
	// printf("\n");


	// for (int i=0; i < labels->rows * labels->cols; ++i ) {
	// 	printf("%f ", op.params[1].tmpref.buffer);
	// }
	// printf("----------------------------------------------------\n");

	printf("Started sending training data...\n");

	// send RPC call
	res = TEEC_InvokeCommand(&sess, TA_NN_PART_CMD_SEND_DATA, &op, &origin);

	if (res != TEEC_SUCCESS) {
		errx(1, "TEEC_InvokeCommand(TA_NN_PART_CMD_SEND_DATA) failed 0x%x origin 0x%x",
         	res, origin);
	}

	printf("Finished sending training data to TA.\n");


	// for (int i=0; i < features->rows; ++i){
	// 	for (int j=0; j < features->cols; ++j){
	// 		printf("%f ", features->vals[i][j]);
	// 	}	
	// 	printf("\n");
	// }
	// printf("\n");

	// for (int i=0; i < labels->rows; ++i){
	// 	for (int j=0; j < labels->cols; ++j){
	// 		printf("%f ", labels->vals[i][j]);
	// 	}	
	// 	printf("\n");
	// }
	// printf("\n");

	free(params0);
	free(params1);

	destroy_matrix(features);
	destroy_matrix(labels);

	printf("finished send data \n");
}

int main(void)
{
	if (RUN_REE) {
		ree_nn();
		return;
	}

	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Operation op;
	TEEC_UUID uuid = TA_NN_PART_UUID;
	uint32_t err_origin;

	/* Initialize a context connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

	/*
	 * Open a session to the "hello world" TA, the TA will print "hello
	 * world!" in the log when the session is created.
	 */
	res = TEEC_OpenSession(&ctx, &sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
			res, err_origin);

		printf("Allocating shared memory...\n");


	// Allocate shared memory
    TEEC_SharedMemory shared_mem;
	shared_mem.buffer = NULL;
    shared_mem.size = 4000000; // 64 MB
    shared_mem.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
	
    res = TEEC_AllocateSharedMemory(&ctx, &shared_mem);
    if (res != TEEC_SUCCESS) {
        printf("Failed to allocate shared memory: 0x%x\n", res);
        TEEC_CloseSession(&sess);
        TEEC_FinalizeContext(&ctx);
        return res;
    }

	memset(shared_mem.buffer, 0, shared_mem.size);
	printf("Finished allocation.\n");

	// Set the shared memory reference in the trusted application
	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_PARTIAL_INOUT, 
						TEEC_NONE, TEEC_NONE, TEEC_NONE);

    op.params[0].memref.parent = &shared_mem;
	op.params[0].memref.offset = 0;
	op.params[0].memref.size = shared_mem.size;
	printf("5\n");

	res = TEEC_InvokeCommand(&sess, TA_NN_PART_CMD_SHARE_MEM, &op,
				 &err_origin);
	printf("6\n");
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
			res, err_origin);

	printf("Registered shared memory with TA.\n");

	/*
	 * TA_HELLO_WORLD_CMD_INC_VALUE is the actual function in the TA to be
	 * called.
	 */
	char dir[] = "/root/mnist/images";
	TEEC_SendData(dir, 3000, 28, 28, 1, "_c", 10);

	/*
	 * Prepare the argument. Pass a value in the first parameter,
	 * the remaining three parameters are unused.
	 */
	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_PARTIAL_INOUT, 
						TEEC_NONE, TEEC_NONE, TEEC_NONE);

    op.params[0].memref.parent = &shared_mem;
	op.params[0].memref.offset = 0;
	op.params[0].memref.size = shared_mem.size;

	printf("Invoking TA to train network\n");
	
	res = TEEC_InvokeCommand(&sess, TA_NN_PART_CMD_TRAIN_NET, &op, &err_origin);

	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
			res, err_origin);


	/*
	 * We're done with the TA, close the session and
	 * destroy the context.
	 *
	 * The TA will print "Goodbye!" in the log when the
	 * session is closed.
	 */

	TEEC_ReleaseSharedMemory(&shared_mem);

	TEEC_CloseSession(&sess);

	TEEC_FinalizeContext(&ctx);

	return 0;
}
