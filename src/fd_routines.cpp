#include "fd_routines.hpp"

Vec_unique fd::gradient_to_global_unique(const DM &dmda, const Vec &localvec, integer dim)
{
  //  First sanity check we have a valid local vector for the DMDA
  //  (should have matching global, local and comm size)
  Vec dm_local_vec;
  PetscErrorCode perr = DMGetLocalVector(dmda, &dm_local_vec);CHKERRABORT(PETSC_COMM_WORLD, perr);
  if(!vecs_equivalent(dm_local_vec, localvec)){
    throw std::runtime_error("provided vector invalid for given dmda object");
  }
  perr = DMRestoreLocalVector(dmda, &dm_local_vec);CHKERRABORT(PETSC_COMM_WORLD, perr);

  // New global vec must be a dmda global 
  Vec_unique grad = create_unique_vec();
  perr = DMCreateGlobalVector(dmda, grad.get());CHKERRABORT(PETSC_COMM_WORLD, perr);

  // Access local of this to have ghosts, global of grad to avoid later copy
  // Recall grad and image share a DMDA
  floating ***img_array, ***grad_array; //acceptable to use raw ptrs here as memory handled by PETSc
  perr = DMDAVecGetArray(dmda, localvec, &img_array);CHKERRABORT(PETSC_COMM_WORLD, perr);
  perr = DMDAVecGetArray(dmda, *(grad.get()), &grad_array);CHKERRABORT(PETSC_COMM_WORLD, perr);

  integer i_lo, i_hi, j_lo, j_hi, k_lo, k_hi;
  // This returns corners + widths so add lo to get hi
  perr = DMDAGetCorners(dmda, &i_lo, &j_lo, &k_lo, &i_hi, &j_hi, &k_hi);CHKERRABORT(PETSC_COMM_WORLD, perr);
  i_hi += i_lo;
  j_hi += j_lo;
  k_hi += k_lo;

  intvector ofs = {0,0,0};
  ofs[dim] = 1;
  for(integer i=i_lo; i < i_hi; i++)
  {
    for(integer j=j_lo; j < j_hi; j++)
    {
      for(integer k=k_lo; k < k_hi; k++)
      {
        // Remember c-indexing is backwards because PETSc is a fortran monster
        grad_array[k][j][i] = 0.5*(img_array[k+ofs[2]][j+ofs[1]][i+ofs[0]]
                                   - img_array[k-ofs[2]][j-ofs[1]][i-ofs[0]]);
      }
    }
  }
  // Release pointers and allow petsc to cleanup
  perr = DMDAVecRestoreArray(dmda, localvec, &img_array);CHKERRABORT(PETSC_COMM_WORLD, perr);
  perr = DMDAVecRestoreArray(dmda, *(grad.get()), &grad_array);CHKERRABORT(PETSC_COMM_WORLD, perr);

  return grad;
}