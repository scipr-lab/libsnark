/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef GADGET_HPP_
#define GADGET_HPP_

#include "protoboard.hpp"

namespace libsnark {

template<typename FieldT>
class gadget {
protected:
    protoboard<FieldT> &pb;
    const std::string annotation_prefix;
public:
    gadget(protoboard<FieldT> &pb, const std::string &annotation_prefix="");
};

} // libsnark
#include "gadget.tcc"

#endif // GADGET_HPP_
