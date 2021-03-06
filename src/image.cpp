//
//   Copyright 2019 University of Sheffield
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#include "image.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include <petscdmda.h>
#include <petscvec.h>

#include "fd_routines.hpp"
#include "iterator_routines.hpp"
#include "map.hpp"
#include "indexing.hpp"

#include "baseloader.hpp"

integer Image::instance_id_counter = 0;

// Public Methods

Image::Image(const intvector& shape, MPI_Comm comm)
  : m_comm(comm), m_ndim(shape.size()),
    m_shape(shape), // const on shape causes copy assignment (c++11)
    m_localvec(create_unique_vec()), m_globalvec(create_unique_vec()), m_dmda(create_shared_dm()),
    instance_id(instance_id_counter++)
{
  if (m_shape.size() != 3)
  {
    if (m_shape.size() == 2)
    {
      m_shape.push_back(1);
    }
    else
    {
      throw std::runtime_error("image shape should be 2D or 3D");
    }
  }
  if (m_shape[2] == 1)
  {
    m_ndim = 2;
  }

  initialize_dmda();
  initialize_vectors();
}

std::unique_ptr<Image> Image::duplicate() const
{
  return std::unique_ptr<Image>(new Image(*this));
}

std::unique_ptr<Image> Image::copy() const
{
  // private copy c'tor prohibits use of std::make_unique, would otherwise do:
  // std::unique_ptr<Image> new_img = std::make_unique<Image>(*this);
  std::unique_ptr<Image> new_img(new Image(*this));

  PetscErrorCode perr = VecCopy(*m_localvec, *new_img->m_localvec);
  CHKERRABORT(m_comm, perr);
  perr = VecCopy(*m_globalvec, *new_img->m_globalvec);
  CHKERRABORT(m_comm, perr);

  return new_img;
}

std::unique_ptr<Image>
Image::load_file(const std::string& path, const Image* existing, MPI_Comm comm)
{
  BaseLoader_unique loader = BaseLoader::find_loader(path, comm);

  // if image passed assert sizes match and duplicate, otherwise create new image given size
  std::unique_ptr<Image> new_image;
  if (existing != nullptr)
  {
    comm = existing->comm();
    if (!all_true(
            loader->shape().begin(), loader->shape().end(), existing->shape().begin(),
            existing->shape().end(), std::equal_to<>()))
    {
      throw std::runtime_error("New image must have same shape as existing");
    }
    new_image = existing->duplicate();
  }
  else
  {
    new_image = std::make_unique<Image>(loader->shape(), comm);
  }

  intvector shape(3, 0), offset(3, 0);
  PetscErrorCode perr = DMDAGetCorners(
      *new_image->dmda(), &offset[0], &offset[1], &offset[2], &shape[0], &shape[1], &shape[2]);
  CHKERRABORT(comm, perr);
  // std::transform(shape.cbegin(), shape.cend(), offset.cbegin(), shape.begin(), std::minus<>());

  floating*** vecptr(nullptr);
  perr = DMDAVecGetArray(*new_image->dmda(), *new_image->global_vec(), &vecptr);
  CHKERRABORT(comm, perr);
  loader->copy_scaled_chunk(vecptr, shape, offset);
  perr = DMDAVecRestoreArray(*new_image->dmda(), *new_image->global_vec(), &vecptr);
  CHKERRABORT(comm, perr);

  return new_image;
}
// Return scale factor
floating Image::normalize()
{
  floating norm;
  PetscErrorCode perr = VecSum(*m_globalvec, &norm);
  CHKERRABORT(m_comm, perr);
  norm = this->size() / norm;
  perr = VecScale(*m_globalvec, norm);
  CHKERRABORT(m_comm, perr);
  return norm;
}
/*
void Image::set_mask(std::shared_ptr<Mask> mask)
{
  //TODO iscompat()
  this->mask = mask;
}

void Image::masked_normalize()
{
  if(m_mask.get() == nullptr)
  {
    throw std::runtime_error("mask must be set first")
  }

  Vec_unique tmp = create_unique_vec();
  PetscErrorcode perr = VecDuplicate(*m_globalvec, tmp.get());CHKERRABORT(m_comm, perr);
  perr = VecPointwiseMult(*tmp, *this->mask->global_vec(), *m_globalvec);CHKERRABORT(m_comm, perr);

  floating norm;
  perr = VecSum(*tmp, &norm);CHKERRABORT(m_comm, perr);
  norm = this->mask->npoints() / norm;
  perr = VecScale(*m_globalvec, norm);CHKERRABORT(m_comm, perr);

  return norm;
}*/

// Protected Methods

Image::Image(const Image& image)
  : m_comm(image.m_comm), m_ndim(image.m_ndim), m_shape(image.m_shape),
    m_localvec(create_shared_vec()), m_globalvec(create_shared_vec()), m_dmda(image.m_dmda),
    instance_id(instance_id_counter++)
{
  initialize_vectors();
}

void Image::initialize_dmda()
{
  integer dof_per_node = 1;
  integer stencil_width = 1;
  PetscErrorCode perr;

  // Make sure things get gracefully cleaned up
  m_dmda = create_shared_dm();
  perr = DMDACreate3d(
      m_comm, DM_BOUNDARY_GHOSTED, DM_BOUNDARY_GHOSTED, DM_BOUNDARY_GHOSTED, // BCs
      DMDA_STENCIL_STAR,                                                     // stencil shape
      m_shape[0], m_shape[1], m_shape[2],                                    // global mesh shape
      PETSC_DECIDE, PETSC_DECIDE, PETSC_DECIDE,                              // ranks per dim
      dof_per_node, stencil_width, // dof per node, stencil size
      nullptr, nullptr, nullptr,   // partition sizes nullptr -> petsc chooses
      m_dmda.get());
  CHKERRABORT(m_comm, perr);

  perr = DMSetUp(*(m_dmda));
  CHKERRABORT(m_comm, perr);
}

void Image::initialize_vectors()
{
  PetscErrorCode perr;
  m_localvec = create_shared_vec();
  m_globalvec = create_shared_vec();
  perr = DMCreateGlobalVector(*m_dmda, m_globalvec.get());
  CHKERRABORT(m_comm, perr);
  debug_creation(*m_globalvec, std::string("image_global_") + std::to_string(instance_id));
  perr = DMCreateLocalVector(*m_dmda, m_localvec.get());
  CHKERRABORT(m_comm, perr);
  debug_creation(*m_localvec, std::string("image_local_") + std::to_string(instance_id));
}

void Image::update_local_from_global()
{
  PetscErrorCode perr = DMGlobalToLocalBegin(*m_dmda, *m_globalvec, INSERT_VALUES, *m_localvec);
  CHKERRABORT(PETSC_COMM_WORLD, perr);
  perr = DMGlobalToLocalEnd(*m_dmda, *m_globalvec, INSERT_VALUES, *m_localvec);
  CHKERRABORT(PETSC_COMM_WORLD, perr);
}

Vec_unique Image::gradient(integer dim)
{
  // New global vec must be a duplicate of image global
  Vec_shared grad = create_shared_vec();
  PetscErrorCode perr = VecDuplicate(*(m_globalvec), grad.get());
  CHKERRABORT(m_comm, perr);

  // Ensure we have up to date ghost cells
  DMGlobalToLocalBegin(*m_dmda, *m_globalvec, INSERT_VALUES, *m_localvec);
  // Nothing really useful to interleave
  DMGlobalToLocalEnd(*m_dmda, *m_globalvec, INSERT_VALUES, *m_localvec);

  return fd::gradient_to_global_unique(*m_dmda, *m_localvec, dim);
}

Vec_unique Image::scatter_to_zero(Vec& vec) const
{
  Vec_unique new_vec = create_unique_vec();
  VecScatter_unique sct = create_unique_vecscatter();
  VecScatterCreateToZero(vec, sct.get(), new_vec.get());
  VecScatterBegin(*sct, vec, *new_vec, INSERT_VALUES, SCATTER_FORWARD);
  VecScatterEnd(*sct, vec, *new_vec, INSERT_VALUES, SCATTER_FORWARD);

  return new_vec;
}

const floating* Image::get_raw_data_ro() const
{
  const floating* ptr;
  PetscErrorCode perr = VecGetArrayRead(*m_globalvec, &ptr);
  CHKERRABORT(m_comm, perr);
  return ptr;
}

void Image::release_raw_data_ro(const floating*& ptr) const
{
  PetscErrorCode perr = VecRestoreArrayRead(*m_globalvec, &ptr);
  CHKERRABORT(m_comm, perr);
}

Vec_unique Image::get_raw_data_row_major() const
{
  Vec_unique rmlocalpart = create_unique_vec();
  PetscErrorCode perr = VecDuplicate(*m_globalvec, rmlocalpart.get());
  CHKERRABORT(m_comm, perr);

  intvector locs(3, 0);
  intvector widths(3, 0);
  perr = DMDAGetCorners(*m_dmda, &locs[0], &locs[1], &locs[2], &widths[0], &widths[1], &widths[2]);
  CHKERRABORT(m_comm, perr);

  integer ownedlo;
  perr = VecGetOwnershipRange(*m_globalvec, &ownedlo, nullptr);
  integer localsize = std::accumulate(widths.begin(), widths.end(), 1, std::multiplies<>());
  intvector cmidxn(localsize);
  std::iota(cmidxn.begin(), cmidxn.end(), 0);

  std::transform(cmidxn.begin(), cmidxn.end(), cmidxn.begin(),
                 [widths, ownedlo](integer idx) -> integer {return idx_cmaj_to_rmaj(idx, widths) + ownedlo;});

  IS_unique src_is = create_unique_is(); 
  perr = ISCreateStride(m_comm, localsize, ownedlo, 1, src_is.get());
  CHKERRABORT(m_comm, perr);
  IS_unique tgt_is = create_unique_is();
  perr = ISCreateGeneral(m_comm, localsize, cmidxn.data(), PETSC_USE_POINTER, tgt_is.get());
  CHKERRABORT(m_comm, perr);
  VecScatter_unique sct = create_unique_vecscatter();
  perr = VecScatterCreate(*m_globalvec, *src_is, *rmlocalpart, *tgt_is, sct.get());
  CHKERRABORT(m_comm, perr);

  perr = VecScatterBegin(*sct, *m_globalvec, *rmlocalpart, INSERT_VALUES, SCATTER_FORWARD);
  CHKERRABORT(m_comm, perr);
  perr = VecScatterEnd(*sct, *m_globalvec, *rmlocalpart, INSERT_VALUES, SCATTER_FORWARD);
  CHKERRABORT(m_comm, perr);

  return rmlocalpart;
}
