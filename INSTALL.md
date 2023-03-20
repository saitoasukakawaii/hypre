<!--
Copyright (c) 1998 Lawrence Livermore National Security, LLC and other
HYPRE Project Developers. See the top-level COPYRIGHT file for details.

SPDX-License-Identifier: (Apache-2.0 OR MIT)
-->

HYPRE Installation Information for Unix-based systems
=====================================================

The simplest way to build HYPRE is:

1. From within the `src` directory, type `configure` to configure the package
   for your system.

   While configure runs, it prints messages indicating which features it is
   checking for.  Two output files are created: config.status and config.log.
   The config.status file can be run to recreate the current configuration, and
   config.log is useful for debugging configure.  Upon successful completion,
   the file `config/Makefile.config` is created from its template
   `Makefile.config.in` and HYPRE is ready to be made.

2. Type `make install` to compile and install HYPRE.

3. You can remove the program binaries and object files from the source code by
   typing `make clean`.  To remove additional files created by configure, type
   `make distclean`.

## Optional Features:

Configure has many options to allow the user to override and refine the defaults
of any system. To display the available options, type `configure --help`.

The make step in building HYPRE is where the compiling, loading and creation of
libraries occurs. Make has several options called targets, which can be listed
by running `make help`.

When building HYPRE without the install target, the libraries and include files
are copied into the directories, `src/hypre/lib` and `src/hypre/include`.

When building with the install target, the libraries and files are copied into
the directories specified by the configure option, `--prefix=/usr/apps`. If none
were specified, the default directories are used, `hypre/lib` and `hypre/include`.

## Building HYPRE for NVIDIA GPUs (CUDA)

We recommend the following configure options when building HYPRE for NVIDIA GPUs:

```
./configure --with-gpu-arch=${CUDA_SM}\
            --with-cuda \
            --with-umpire \
            --with-umpire-include=${UMPIRE_INCLUDE_DIR} \
            --with-umpire-lib-dirs=${UMPIRE_LIB_DIR} \
            --with-umpire-libs=umpire
```
In the configure line above, `${CUDA_SM}` is a list of architectures to generate device
code for, e.g., `${CUDA_SM}="70 80"` targets NVIDIA's Volta and Ampere
architectures. The user can define as many device architectures as needed in the
`${CUDA_SM}` variable, including just a single one, which is the most common use
case. [Umpire](https://github.com/LLNL/Umpire) is a third-party library developed at LLNL
that improves memory allocation and deallocation times on GPUs through a technique called
memory pooling. We recommend the usage of Umpire when running HYPRE on GPUs since it
reduces the setup times of many preconditioners (e.g., BoomerAMG). Note that the user is
responsible for installing Umpire and providing to HYPRE's `configure` the directories
where its headers (`${UMPIRE_INCLUDE_DIR}`) and library (`${UMPIRE_LIB_DIR}`) files are
located. If the user is unable to employ Umpire, another option is to use the configure
option `--enable-device-memory-pool`, which enables HYPRE's internal memory pooling
implementation based on [cub](https://github.com/NVIDIA/cub). Lastly, other configure
options might be necessary depending on the use case, e.g., `--enable-mixedint` when
dealing with matrices/vectors whose number of rows is larger than `2,147,483,647`.

## Building hypre for AMD GPUs (HIP)

TODO

## Building hypre for Intel GPUs (SYCL)

TODO

HYPRE Installation Information using CMake
==========================================

CMake is another way to build HYPRE that is particularly useful for building the
code on Windows machines.  CMake provides a uniform interface for setting
configuration options on different platforms.  It does not actually build the
code, but generates input for other "native" build systems such as Make (Unix
platforms) or Visual Studio (Windows).  Here are the basic steps:

1. First, ensure that CMake version 3.13.0 or later is installed on the system.

2. To build the library, run CMake on the top-level HYPRE source directory to
   generate files appropriate for the native build system.  To prevent writing
   over the Makefiles in HYPRE's configure/make system above, only out-of-source
   builds are currently allowed with CMake. The directory `src/cmbuild`
   is provided in the release for convenience, but
   alternative build directories may be created by the user:

   - Unix: From the `src/cmbuild` directory, type `cmake ..`.

   - Windows: Set the source and build directories to `src` and `src/cmbuild`,
     then click on 'Configure' following by 'Generate'.

3. To complete the build of the library, compile with the native build system:

   - Unix: From the `src/cmbuild` directory, type `make install`.

   - Windows Visual Studio: Open the 'hypre' VS solution file generated by CMake
     and build the 'ALL_BUILD' target, then the 'INSTALL' target.

Optional Features:

Various configuration options can be set from within CMake.  Change these as
appropriate, then reconfigure/generate:

- Unix: From the `src/cmbuild` directory, type `ccmake ..`, change options, then
  type `c` then `q`.

- Windows VS: Change options, then click on 'Configure' then 'Generate'.


ABI Compatibility
=================

The hypre team currently does nothing to ensure application binary interface
(ABI) compatibility.  As a result, all releases (major, minor, or patch) should
be treated as incompatible.
