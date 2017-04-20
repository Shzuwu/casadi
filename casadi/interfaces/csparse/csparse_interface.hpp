/*
 *    This file is part of CasADi.
 *
 *    CasADi -- A symbolic framework for dynamic optimization.
 *    Copyright (C) 2010-2014 Joel Andersson, Joris Gillis, Moritz Diehl,
 *                            K.U. Leuven. All rights reserved.
 *    Copyright (C) 2011-2014 Greg Horn
 *
 *    CasADi is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 3 of the License, or (at your option) any later version.
 *
 *    CasADi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with CasADi; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#ifndef CASADI_CSPARSE_INTERFACE_HPP
#define CASADI_CSPARSE_INTERFACE_HPP

/** \defgroup plugin_Linsol_csparse
 * Linsol with CSparse Interface
*/

/** \pluginsection{Linsol,csparse} */

/// \cond INTERNAL
#include <cs.h>
#include "casadi/core/linsol_internal.hpp"
#include <casadi/interfaces/csparse/casadi_linsol_csparse_export.h>

namespace casadi {
  struct CASADI_LINSOL_CSPARSE_EXPORT CsparseMemory : public LinsolMemory {
    // Destructor
    ~CsparseMemory();

    // The linear system CSparse form (CCS)
    cs A;

    // The symbolic factorization
    css *S;

    // The numeric factorization
    csn *N;

    // Temporary
    std::vector<double> temp_;
  };

  /** \brief \pluginbrief{LinsolInternal,csparse}
   * @copydoc LinsolInternal_doc
   * @copydoc plugin_LinsolInternal_csparse
   */
  class CASADI_LINSOL_CSPARSE_EXPORT CsparseInterface : public LinsolInternal {
  public:

    // Create a linear solver given a sparsity pattern and a number of right hand sides
    CsparseInterface(const std::string& name);

    /** \brief  Create a new LinsolInternal */
    static LinsolInternal* creator(const std::string& name) {
      return new CsparseInterface(name);
    }

    // Destructor
    virtual ~CsparseInterface();

    // Initialize the solver
    virtual void init(const Dict& opts);

    /** \brief Create memory block */
    virtual void* alloc_memory() const { return new CsparseMemory();}

    /** \brief Free memory block */
    virtual void free_memory(void *mem) const { delete static_cast<CsparseMemory*>(mem);}

    /** \brief Initalize memory block */
    virtual void init_memory(void* mem) const;

    // Set sparsity pattern
    virtual void reset(void* mem, const int* sp) const;

    // Symbolic factorization
    virtual void pivoting(void* mem, const double* A) const;

    // Factorize the linear system
    virtual void factorize(void* mem, const double* A) const;

    // Solve the linear system
    virtual void solve(void* mem, double* x, int nrhs, bool tr) const;

    // Obtain factorization
    virtual void linsol_lu(void* mem, DM& L, DM& U, DM& P1, DM& P2, bool tr) const;

    /// A documentation string
    static const std::string meta_doc;

    // Get name of the plugin
    virtual const char* plugin_name() const { return "csparse";}
  };

} // namespace casadi

/// \endcond

#endif // CASADI_CSPARSE_INTERFACE_HPP
