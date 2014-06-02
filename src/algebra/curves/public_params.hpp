/** @file
 ********************************************************************************
 * @authors    Eli Ben-Sasson, Alessandro Chiesa, Daniel Genkin,
 *             Shaul Kfir, Eran Tromer, Madars Virza.
 * This file is part of libsnark, developed by SCIPR Lab <http://scipr-lab.org>.
 * @copyright  MIT license (see LICENSE file)
 *******************************************************************************/

#ifndef PUBLIC_PARAMS_HPP_
#define PUBLIC_PARAMS_HPP_
#include <vector>

namespace libsnark {

template<typename ppT>
class pp_selector {
    /*
      for every curve the user should define corresponding
      public_params and a template specialization for pp_selector
      with the following typedefs:

      Fp_type
      G1_type
      G2_type
      G1_precomp_type
      G2_precomp_type
      Fq_type
      Fqe_type
      Fqk_type
      GT_type
    */
};

template<typename ppT>
using Fr = typename pp_selector<ppT>::Fp_type;
template<typename ppT>
using G1 = typename pp_selector<ppT>::G1_type;
template<typename ppT>
using G2 = typename pp_selector<ppT>::G2_type;
template<typename ppT>
using G1_precomp = typename pp_selector<ppT>::G1_precomp_type;
template<typename ppT>
using G2_precomp = typename pp_selector<ppT>::G2_precomp_type;
template<typename ppT>
using affine_ate_G1_precomp = typename pp_selector<ppT>::affine_ate_G1_precomp_type;
template<typename ppT>
using affine_ate_G2_precomp = typename pp_selector<ppT>::affine_ate_G2_precomp_type;
template<typename ppT>
using Fq = typename pp_selector<ppT>::Fq_type;
template<typename ppT>
using Fqe = typename pp_selector<ppT>::Fqe_type;
template<typename ppT>
using Fqk = typename pp_selector<ppT>::Fqk_type;
template<typename ppT>
using GT = typename pp_selector<ppT>::GT_type;

template<typename ppT>
using Fr_vector = std::vector<Fr<ppT> >;
template<typename ppT>
using G1_vector = std::vector<G1<ppT> >;
template<typename ppT>
using G2_vector = std::vector<G2<ppT> >;

/* one should also define specializations for the following templatized functions */
template<typename ppT>
void init_public_params();

template<typename ppT>
GT<ppT> final_exponentiation(const Fqk<ppT> &elt);

template<typename ppT>
G1_precomp<ppT> precompute_G1(const G1<ppT> &P);

template<typename ppT>
G2_precomp<ppT> precompute_G2(const G2<ppT> &Q);

template<typename ppT>
Fqk<ppT> miller_loop(const G1_precomp<ppT> &prec_P,
                     const G2_precomp<ppT> &prec_Q);

template<typename ppT>
affine_ate_G1_precomp<ppT> affine_ate_precompute_G1(const G1<ppT> &P);

template<typename ppT>
affine_ate_G2_precomp<ppT> affine_ate_precompute_G2(const G2<ppT> &Q);

template<typename ppT>
Fqk<ppT> affine_ate_miller_loop(const affine_ate_G1_precomp<ppT> &prec_P,
                                const affine_ate_G2_precomp<ppT> &prec_Q);

template<typename ppT>
Fqk<ppT> double_miller_loop(const G1_precomp<ppT> &prec_P1,
                            const G2_precomp<ppT> &prec_Q1,
                            const G1_precomp<ppT> &prec_P2,
                            const G2_precomp<ppT> &prec_Q2);

/* the following are used in test files */
template<typename ppT>
Fqk<ppT> pairing(const G1<ppT> &P,
                 const G2<ppT> &Q);

template<typename ppT>
GT<ppT> reduced_pairing(const G1<ppT> &P,
                        const G2<ppT> &Q);

template<typename ppT>
GT<ppT> affine_reduced_pairing(const G1<ppT> &P,
                               const G2<ppT> &Q);

} // libsnark
#endif // PUBLIC_PARAMS_HPP_
