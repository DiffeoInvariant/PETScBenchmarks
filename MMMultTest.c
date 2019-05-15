/**
 @author: Zane Jakobs
 @brief: read and multiply two PETSc AIJ matrices with a defined implementation
 of MatMatMult_SeqAIJ_SeqAIJ()
 */
static char help[] = "Tests speed of multiplying two PETSc AIJ matrices with a single implementation that is defined by the programmer at compile time. See matmatmult.c in the PETSc library for implementations corresponding to integers 1 through 8.\n\n";

#include <petscmat.h>
#include <stdlib.h>
#include <mpi.h>


#define MATRIX1_FILENAME "matrix1.dat"
#define MATRIX2_FILENAME "matrix2.dat"

#define SMALL_ITER 30

/**
 reads matrix from filename, fills readMat
 */
PetscErrorCode readPetscMatrix(char filename[], Mat* readMat)
{
    PetscErrorCode  ierr;
    PetscViewer     viewer;
    
    ierr = PetscPrintf(PETSC_COMM_WORLD, "Reading in matrix from %s...\n",
                       filename); CHKERRQ(ierr);
    ierr = PetscViewerBinaryOpen(PETSC_COMM_WORLD, filename,
                                 FILE_MODE_READ, &viewer); CHKERRQ(ierr);
    
    ierr = MatCreate(PETSC_COMM_WORLD, readMat); CHKERRQ(ierr);
    ierr = MatLoad(*readMat, viewer); CHKERRQ(ierr);
    ierr = PetscViewerDestroy(&viewer); CHKERRQ(ierr);
    
    ierr = PetscPrintf(PETSC_COMM_WORLD, "Successfully read matrix from %s.\n", filename); CHKERRQ(ierr);
    
    return ierr;
}


int main(int argc, char** argv)
{
    PetscMPIInt     rank,size;
    Mat             Mat1,Mat2,Mat3;
    PetscReal       fill;
    PetscErrorCode  ierr;
    double          startT, stopT;
    
    ierr = PetscInitialize(&argc, &argv, NULL, help); CHKERRQ(ierr);
    ierr = MPI_Comm_rank(PETSC_COMM_WORLD, &rank); CHKERRQ(ierr);
    ierr = MPI_Comm_size(PETSC_COMM_WORLD, &size); CHKERRQ(ierr);
    
    /*read in matrices*/
    ierr = readPetscMatrix(MATRIX1_FILENAME, &Mat1); CHKERRQ(ierr);
    ierr = MPI_Barrier(PETSC_COMM_WORLD); CHKERRQ(ierr);
    
    ierr = readPetscMatrix(MATRIX2_FILENAME, &Mat2); CHKERRQ(ierr);
    /*make all processors wait until this is done*/
    ierr = MPI_Barrier(PETSC_COMM_WORLD); CHKERRQ(ierr);
    /*test multiplications*/
    unsigned i;
    startT = MPI_Wtime();
    ierr = PetscPrintf(PETSC_COMM_WORLD, "Running benchmark. This could take a while...\n"); CHKERRQ(ierr);
    #if defined(LARGE_ITER)
    for(i = 0; i < 250; i++){
        ierr = MatMatMultSymbolic(Mat1, Mat2, PETSC_DEFAULT, &Mat3); CHKERRQ(ierr);
    }
    #else
    for(i = 0; i < SMALL_ITER; i++){
        ierr = MatMatMultSymbolic(Mat1, Mat2, PETSC_DEFAULT, &Mat3); CHKERRQ(ierr);
    }
    #endif
    stopT = MPI_Wtime();
    
    ierr = PetscPrintf(PETSC_COMM_WORLD, "Time elapsed: %f\n", stopT - startT); CHKERRQ(ierr);
    
    /*clean up memory*/
    ierr = MatDestroy(&Mat1); CHKERRQ(ierr);
    ierr = MatDestroy(&Mat2); CHKERRQ(ierr);
    ierr = MatDestroy(&Mat3); CHKERRQ(ierr);
    
    ierr = PetscFinalize();
    return ierr;
}
