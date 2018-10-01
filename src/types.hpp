#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

#include<memory>
#include<vector>
#include<iostream>

#include<petscsys.h>
#include<petscvec.h>
#include<petscmat.h>
#include<petscdm.h>

using integer = PetscInt;
using intvector =  std::vector<integer>;
using intvector2d =  std::vector<intvector>;

using floating = PetscScalar;
using floatvector =  std::vector<floating>;
using floatvector2d =  std::vector<floatvector>;

//// Self-destructing PETSc objects
// Can apply identical treatment to many PETSc types

//// Vec
// typedef and helpers for unique_ptr
struct VecDeleter{void operator()(Vec* v) const{VecDestroy(v);} };
using Vec_unique = std::unique_ptr<Vec, VecDeleter>;
inline Vec_unique create_unique_vec(){return Vec_unique(new Vec);}

// typedef and helper for shared_ptr
using Vec_shared = std::shared_ptr<Vec>;
inline Vec_shared create_shared_vec(){return Vec_shared(new Vec, VecDestroy);}


//// Mat
// typedef and helpers for unique_ptr
struct MatDeleter{void operator()(Mat* v) const{MatDestroy(v);} };
using Mat_unique = std::unique_ptr<Mat, MatDeleter>;
inline Mat_unique create_unique_mat(){return Mat_unique(new Mat);}

// typedef and helper for shared_ptr
using Mat_shared = std::shared_ptr<Mat>;
inline Mat_shared create_shared_mat(){return Mat_shared(new Mat, MatDestroy);}


//// DM
// typedef and helpers for unique_ptr
struct DMDeleter{void operator()(DM* v) const{DMDestroy(v);} };
using DM_unique = std::unique_ptr<DM, DMDeleter>;
inline DM_unique create_unique_dm(){return DM_unique(new DM);}

// typedef and helper for shared_ptr
using DM_shared = std::shared_ptr<DM>;
inline DM_shared create_shared_dm(){return DM_shared(new DM, DMDestroy);}


//// IS
// typedef and helpers for unique_ptr
struct ISDeleter{void operator()(IS* v) const{ISDestroy(v);} };
using IS_unique = std::unique_ptr<IS, ISDeleter>;
inline IS_unique create_unique_is(){return IS_unique(new IS);}

// typedef and helper for shared_ptr
using IS_shared = std::shared_ptr<IS>;
inline IS_shared create_shared_is(){return IS_shared(new IS, ISDestroy);}


//// VecScatter
// typedef and helpers for unique_ptr
struct VecScatterDeleter{void operator()(VecScatter* v) const{VecScatterDestroy(v);} };
using VecScatter_unique = std::unique_ptr<VecScatter, VecScatterDeleter>;
inline VecScatter_unique create_unique_vecscatter(){return VecScatter_unique(new VecScatter);}

// typedef and helper for shared_ptr
using VecScatter_shared = std::shared_ptr<VecScatter>;
inline VecScatter_shared create_shared_vecscatter(){return VecScatter_shared(new VecScatter, VecScatterDestroy);}

#endif