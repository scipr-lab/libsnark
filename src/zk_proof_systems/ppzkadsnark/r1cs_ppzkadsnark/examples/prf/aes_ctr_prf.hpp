/** @file
 *****************************************************************************

 AES-Based PRF for ADSNARK.

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef AESCTRPRF_HPP_
#define AESCTRPRF_HPP_

#include "../../r1cs_ppzkadsnark_prf.hpp"

namespace libsnark {

class aesPrfKeyT {
public:
    unsigned char key_bytes[32];
};

} // libsnark

#endif // AESCTRPRF_HPP_
