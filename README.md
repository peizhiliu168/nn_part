# NN-Partition
This is a neural network implementation with layer-based partitioning in the TA. 
## How to run
To run with traces uncommented out, you must use the benchmark framework as implemented in https://github.com/peizhiliu168/optee-rpi3. Afterwards, copy the contents of this repo into a directory called `nn_part` in `optee_examples`. Place datasets in the `out-br/target/root` directory. Then configure network parameters in `ta/network.c` and the necessary data parameters in `data.c`. Finally, in the normal world, run nn_part to begin the training process.
## Traces
To see the output traces after training, open the CSV file named /tmp/trace_\<number\>.csv.
## Branches
- optimization-swapping: This branch contains the baseline implementation with doubles. It just contains bug fixes for the code in the main branch.
- optimization-float: This branch contains the implementation with floats.
- secure-store-ree: This branch contains changes part of optimization-float as well as additional changes for using shared memory to store layers in layer-based partitioning. This is the fastest one.
