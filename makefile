PETSC_DIR        =/usr/local/include/petsc
PETSC_ARCH       =arch-darwin-c-debug
CC		 = clang
CFLAGS		 = -O3 -Werror -I$(PETSC_DIR)/include -I$(PETSC_DIR)/$(PETSC_ARCH)/include
GFORTPATH	 = /usr/local/Cellar/gcc/9.1.0/lib/gcc/9
LDFLAGS		 = -L $(PETSC_DIR)/$(PETSC_ARCH)/lib -L$(GFORTPATH) -lgfortran -lpetsc -lmpi
FILENAMES	 = 
NUM_CORES	 =1

include ${PETSC_DIR}/lib/petsc/conf/variables

matWrite: matwriter.c
	$(CC) $(CFLAGS) matwriter.c -o matwriter $(LDFLAGS)
	mpiexec -n 1 ./matwriter -mat_type seqaij 

matMult: MMMultTest.c
	$(CC) $(CFLAGS) MMMultTest.c -o matMult $(LDFLAGS)
	mpiexec -n $(NUM_CORES) ./matMult 
	rm -rf matMult
	

