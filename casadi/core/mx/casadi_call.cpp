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


#include "casadi_call.hpp"
#include "../function/function_internal.hpp"
#include "../std_vector_tools.hpp"
#include "../mx/mx_tools.hpp"
#include "../matrix/matrix_tools.hpp"

using namespace std;

namespace casadi {

  Call::Call(const Function& fcn, vector<MX> arg) : fcn_(fcn) {

    // Number inputs and outputs
    int num_in = fcn.getNumInputs();
    casadi_assert_message(arg.size()<=num_in, "Argument list length (" << arg.size()
                          << ") exceeds number of inputs (" << num_in << ")");

    // Add arguments if needed
    arg.resize(num_in);

    // Assert argument of correct dimension and sparsity
    for (int i=0; i<arg.size(); ++i) {
      if (arg[i].shape()==fcn_.input(i).shape()) {
        // Insert sparsity projection nodes if needed
        arg[i] = arg[i].setSparse(fcn_.input(i).sparsity());
      } else {
        // Different dimensions
        if (arg[i].isEmpty() || fcn_.input(i).isEmpty()) { // NOTE: To permissive?
          // Replace nulls with zeros of the right dimension
          arg[i] = MX::zeros(fcn_.input(i).sparsity());
        } else if (arg[i].isScalar()) {
          // Scalar argument means set all
          arg[i] = MX(fcn_.input(i).sparsity(), arg[i]);
        } else {
          // Mismatching dimensions
          casadi_error("Cannot create function call node: Dimension mismatch for argument "
                       << i << ". Argument has shape " << arg[i].shape()
                       << " but function input is " << fcn_.input(i).shape());
        }
      }
    }

    setDependencies(arg);
    setSparsity(Sparsity::scalar());
  }

  Call* Call::clone() const {
    return new Call(*this);
  }

  void Call::printPart(std::ostream &stream, int part) const {
    fcn_->printPart(this, stream, part);
  }

  void Call::evalD(cp_double* arg, p_double* res,
                           int* itmp, double* rtmp) {
    fcn_->evalD(arg, res, itmp, rtmp);
  }

  int Call::nout() const {
    return fcn_.getNumOutputs();
  }

  const Sparsity& Call::sparsity(int oind) const {
    return fcn_.output(oind).sparsity();
  }

  Function& Call::getFunction() {
    return fcn_;
  }

  void Call::evalSX(cp_SXElement* arg, p_SXElement* res, int* itmp, SXElement* rtmp) {
    fcn_->evalSX(arg, res, itmp, rtmp);
  }

  void Call::evalMX(const vector<MX>& arg, vector<MX>& res) {
    res = fcn_->createCall(arg);
  }

  void Call::evalFwd(const vector<vector<MX> >& fseed,
                     vector<vector<MX> >& fsens) {
    // Nondifferentiated inputs and outputs
    vector<MX> arg(ndep());
    for (int i=0; i<arg.size(); ++i) arg[i] = dep(i);
    vector<MX> res(nout());
    for (int i=0; i<res.size(); ++i) res[i] = getOutput(i);

    // Call the cached functions
    fcn_.callForward(arg, res, fseed, fsens);
  }

  void Call::evalAdj(const vector<vector<MX> >& aseed,
                     vector<vector<MX> >& asens) {
    // Nondifferentiated inputs and outputs
    vector<MX> arg(ndep());
    for (int i=0; i<arg.size(); ++i) arg[i] = dep(i);
    vector<MX> res(nout());
    for (int i=0; i<res.size(); ++i) res[i] = getOutput(i);

    // Call the cached functions
    vector<vector<MX> > v;
    fcn_.callReverse(arg, res, aseed, v);
    for (int i=0; i<v.size(); ++i) {
      for (int j=0; j<v[i].size(); ++j) {
        if (!v[i][j].isEmpty()) { // TODO(@jaeandersson): Hack
          asens[i][j] += v[i][j];
        }
      }
    }
  }

  void Call::deepCopyMembers(std::map<SharedObjectNode*, SharedObject>& already_copied) {
    MXNode::deepCopyMembers(already_copied);
    fcn_ = deepcopy(fcn_, already_copied);
  }

  void Call::spFwd(cp_bvec_t* arg, p_bvec_t* res, int* itmp, bvec_t* rtmp) {
    fcn_.spFwd(arg, res, itmp, rtmp);
  }

  void Call::spAdj(p_bvec_t* arg, p_bvec_t* res, int* itmp, bvec_t* rtmp) {
    fcn_.spAdj(arg, res, itmp, rtmp);
  }

  void Call::generate(std::ostream &stream, const vector<int>& arg,
                      const vector<int>& res, CodeGenerator& gen) const {
    fcn_->generate(stream, arg, res, gen);
  }

  void Call::nTmp(size_t& ni, size_t& nr) {
    fcn_.nTmp(ni, nr);
  }

} // namespace casadi