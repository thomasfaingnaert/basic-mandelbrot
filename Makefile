all:
	g++ cpu.cpp -o cpu
	nvcc cuda.cu -o cuda
