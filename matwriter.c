/**
 @author: Zane Jakobs
 @brief: creates and writes a PETSc matrix of random integers
 for benchmarking purposes
 */

static char help[] = "Creates and writes a 1000x1000 sparse PETSc AIJ matrix of random integers to a binary file. Run with -mat_type seqaij \n\n";

/*Probability that a given element is filled. Default 0.4*/
#define ELEM_FILL_PROB 0.4
/*max value of a random integer*/
#define MAX_RAND_VAL 300

#define FILENAME "matrix1.dat"

#include <petscmat.h>
#include <stdlib.h>
#include <time.h>
int main(int argc, char **args)
{
    /*declare matrix, ints, and viewer. Use 1 core, merge all I/O by default.*/
    Mat writeMat;
    PetscInt row=0,col=0,rowSize=1000,colSize=1000,m=1, n=1;
    PetscViewer viewer;
    /*MPI process identifier, number of MPI processes*/
    PetscMPIInt rank,size;
    /*return type*/
    PetscErrorCode ierr;
    
    /* initialize PETSc database and MPI, return an error if one exists*/
    ierr = PetscInitialize(&argc, &args, NULL, help);
    if(ierr) { return ierr; }
    /*use default communicator, check for errors*/
    ierr = MPI_Comm_rank(PETSC_COMM_WORLD, &rank); CHKERRQ(ierr);
    ierr = MPI_Comm_size(PETSC_COMM_WORLD, &size); CHKERRQ(ierr);
    /*set num cores, I/O merging cores*/
    ierr = PetscOptionsGetInt(NULL, NULL, "-n", &n, NULL); CHKERRQ(ierr);
    ierr = PetscOptionsGetInt(NULL, NULL, "-m", &m, NULL); CHKERRQ(ierr);
    
    /*initialize matrix with default local rows/cols*/
    ierr = MatCreate(PETSC_COMM_WORLD, &writeMat); CHKERRQ(ierr);
    ierr = MatSetSizes(writeMat, PETSC_DECIDE, PETSC_DECIDE, rowSize, colSize); CHKERRQ(ierr);
    
    /*preallocate number of nonzeros per row to be ELEM_FILL_PROB * rowSize*/
    PetscInt nz = (unsigned)(ELEM_FILL_PROB * rowSize);
    ierr = MatSetFromOptions(writeMat); CHKERRQ(ierr);
    ierr = MatSeqAIJSetPreallocation(writeMat, nz, NULL); CHKERRQ(ierr);
    ierr = MatSetUp(writeMat); CHKERRQ(ierr);
    
    /* get range of rows owned by this processor*/
    PetscInt rStart, rEnd;
    ierr = MatGetOwnershipRange(writeMat, &rStart, &rEnd); CHKERRQ(ierr);
    
    /*don't throw an error if malloc is needed in initialization of matrix*/
    ierr = MatSetOption(writeMat, MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_FALSE); CHKERRQ(ierr);
    /*fill array of scalars to be matrix elements, and initialize PRNG*/
    PetscInt colID[colSize];
    PetscScalar values[rowSize];
    time_t now;
    srand( (unsigned) time(&now));
    PetscInt nFilled;
    /* fill matrix rowwise*/
    for(row = rStart; row < rEnd; row++){
        /*for each row*/
        nFilled = 0;
        for(col = 0; col < colSize; col++){
            /*make value nonzero w/ probability ELEM_FILL_PROB*/
            if(rand() % 100 < 100 * ELEM_FILL_PROB){
                values[col] = rand() % MAX_RAND_VAL;
                colID[nFilled] = col;
                nFilled++;
            }
        }
        /*array of nonzero col id's*/
        PetscInt nzCol[nFilled];
        PetscInt nzId;
        for(nzId = 0; nzId < nFilled; nzId++){
            nzCol[nzId] = colID[nzId];
        }
        /*set values of this row*/
        ierr = MatSetValues(writeMat, 1, &row, nFilled, nzCol, values, INSERT_VALUES); CHKERRQ(ierr);
    }/*end outer for*/
    /*assemble matrix*/
    ierr = MatAssemblyBegin(writeMat, MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
    ierr = MatAssemblyEnd(writeMat, MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
    
    ierr = PetscPrintf(PETSC_COMM_WORLD, "Writing matrix to binary file %s...\n",FILENAME
                       ); CHKERRQ(ierr);
    ierr = PetscViewerBinaryOpen(PETSC_COMM_WORLD, FILENAME, FILE_MODE_WRITE, &viewer); CHKERRQ(ierr);
    ierr = MatView(writeMat, viewer); CHKERRQ(ierr);
    /*free memory*/
    ierr = PetscViewerDestroy(&viewer); CHKERRQ(ierr);
    ierr = MatDestroy(&writeMat); CHKERRQ(ierr);
    
    ierr = PetscFinalize();
    return ierr;
}
