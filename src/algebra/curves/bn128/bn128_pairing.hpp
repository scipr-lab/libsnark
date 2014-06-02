/** @file
 ********************************************************************************
 Declares functions for computing Ate pairings over the bn128 curves, split into a
 offline and online stages.

 This code is based on the ate-pairing pairing code <https://github.com/herumi/ate-pairing>,
 split into offline and online stages.
 ********************************************************************************
 * @authors    Eli Ben-Sasson, Alessandro Chiesa, Daniel Genkin,
 *             Shaul Kfir, Eran Tromer, Madars Virza.
 * This file is part of libsnark, developed by SCIPR Lab <http://scipr-lab.org>.
 * Uses portions of ate-pairing <https://github.com/herumi/ate-pairing> by
 *             Shigeo Mitsunari, Tadanori Teruya.
 * @copyright  modified new BSD License (http://opensource.org/licenses/BSD-3-Clause)
 *             (see LICENSE file)
 *******************************************************************************/

/*
The "bn128" curve choice included in libsnark is based on the ate-pairing library by
MITSUNARI Shigeo and TERUYA Tadanori [https://github.com/herumi/ate-pairing].

The ate-pairing library is licensed under "modified new BSD License"
[http://opensource.org/licenses/BSD-3-Clause].

While the ate-pairing library does not include explicit license text, the license
template at the link above is:

Copyright (c) <YEAR>, <OWNER>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef BN128_PAIRING_HPP_
#define BN128_PAIRING_HPP_
#include "algebra/curves/bn128/bn128_g1.hpp"
#include "algebra/curves/bn128/bn128_g2.hpp"
#include "algebra/curves/bn128/bn128_gt.hpp"
#include "bn.h"

namespace libsnark {

struct bn128_ate_G1_precomp {
    bn::Fp P[3];

    bool operator==(const bn128_ate_G1_precomp &other) const;
    friend std::ostream& operator<<(std::ostream &out, const bn128_ate_G1_precomp &prec_P);
    friend std::istream& operator>>(std::istream &in, bn128_ate_G1_precomp &prec_P);
};

typedef bn::Fp6 bn128_ate_ell_coeffs;

struct bn128_ate_G2_precomp {
    bn::Fp2 Q[3];
    std::vector<bn128_ate_ell_coeffs> coeffs;

    bool operator==(const bn128_ate_G2_precomp &other) const;
    friend std::ostream& operator<<(std::ostream &out, const bn128_ate_G2_precomp &prec_Q);
    friend std::istream& operator>>(std::istream &in, bn128_ate_G2_precomp &prec_Q);
};

bn128_ate_G1_precomp bn128_ate_precompute_G1(const bn128_G1& P);
bn128_ate_G2_precomp bn128_ate_precompute_G2(const bn128_G2& Q);

bn128_Fq12 bn128_double_ate_miller_loop(const bn128_ate_G1_precomp &prec_P1,
                                        const bn128_ate_G2_precomp &prec_Q1,
                                        const bn128_ate_G1_precomp &prec_P2,
                                        const bn128_ate_G2_precomp &prec_Q2);
bn128_Fq12 bn128_ate_miller_loop(const bn128_ate_G1_precomp &prec_P,
                                 const bn128_ate_G2_precomp &prec_Q);

bn128_GT bn128_final_exponentiation(const bn128_Fq12 &elt);

} // libsnark
#endif // BN128_PAIRING_HPP_
