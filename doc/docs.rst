.. pFIRE documentation master file, created by
   sphinx-quickstart on Tue Jul 24 10:49:47 2018.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

=====
pFIRE
=====

*pFIRE is a parallel framework for the elastic registration of large images.*

There are two ways to use pFIRE:

1. Use the provided executable tools to interact with pFIRE from the command line. These tools
   expose the majority of pFIRE's functionality through a simple interface.  If you just have a
   pair of images to register, start here.

2. Embed pFIRE functionality within your own code using the API.  This is primarily intended for
   more experienced users and developers to embed elastic registration within larger workflows or
   to extend pFIRE's functionality.

What is Elastic Registration?
-----------------------------

Image registration is a process by which an image is transformed to match a second image as closely
as possible.  The image which is transformed is known as the **moved image**, and the target image
to which is is matched the **fixed image**.

There are two types of image registration:

1. **Rigid registration** uses global transformations which affect the whole image in the same way.
   *Translation, scaling, linear shearing* and *rotation* are all examples of rigid registration.
   These are relatively simple to compute but cannot describe all changes to an image, such as
   non-linear shearing or local deformations.

2. **Elastic registration** creates a displacement map or field which describes how individual
   image pixels are moved to transform between the moved and fixed images.  This map can have an
   arbitrary resolution and can therefore, in principle, describe any transformation of one image
   to another.

Elastic registration is far more computationally intensive than rigid registration, however, the
ability to measure and describe nonlinear transformations is potentially extremely useful for e.g
determination of stress inside a deformed mechanical structure.


Index
-----

**Getting Started**

* :doc:`install`
* :doc:`cli_quickstart`

.. toctree::
  :maxdepth: 1
  :hidden:
  :caption: Getting Started
  
  install.rst
  cli_quickstart.rst

.. **pFIRE API Reference**

.. * :doc:`api_highlevel`

.. .. toctree::
  :maxdepth: 2
  :hidden:
  :caption: API Reference

..  api_highlevel.rst

Development of pFIRE is supported by the European Union’s Horizon 2020 research and innovation
programme under grant agreement No 675451.
