# NN-Partition
This is a neural network implementation with layer-based partitioning in the TA. 
## How to run
To run, copy the contents of this repo into a directory called `nn_part` in `optee_examples`. Place datasets in the `out-br/target/root` directory. Then configure network parameters in `ta/network.c` and the necessary data parameters in `data.c`. Finally, in the normal world, run nn_part to begin the training process. 
