/** @file
 *****************************************************************************

 Implementation of interfaces for:
 - a BACS variable assigment,
 - a BACS gate,
 - a BACS primary input,
 - a BACS auxiliary input,
 - a BACS circuit.

 See bacs.hpp .

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef BACS_TCC_
#define BACS_TCC_

#include <algorithm>
#include "common/utils.hpp"

namespace libsnark {

template<typename FieldT>
FieldT bacs_gate<FieldT>::evaluate(const bacs_variable_assignment<FieldT> &input) const
{
    return lhs.evaluate(input) * rhs.evaluate(input);
}

template<typename FieldT>
void bacs_gate<FieldT>::print(const std::map<size_t, std::string> &variable_annotations) const
{
    printf("(\n");
    lhs.print(variable_annotations);
    printf(") * (\n");
    rhs.print(variable_annotations);
    printf(")\n");
}

template<typename FieldT>
bool bacs_gate<FieldT>::operator==(const bacs_gate<FieldT> &other) const
{
    return (this->lhs == other.lhs &&
            this->rhs == other.rhs &&
            this->output == other.output &&
            this->is_circuit_output == other.is_circuit_output);
}

template<typename FieldT>
std::ostream& operator<<(std::ostream &out, const bacs_gate<FieldT> &g)
{
    out << (g.is_circuit_output ? 1 : 0) << "\n";
    out << g.lhs << OUTPUT_NEWLINE;
    out << g.rhs << OUTPUT_NEWLINE;
    out << g.output.index << "\n";

    return out;
}

template<typename FieldT>
std::istream& operator>>(std::istream &in, bacs_gate<FieldT> &g)
{
    size_t tmp;
    in >> tmp;
    consume_newline(in);
    g.is_circuit_output = (tmp != 0 ? true : false);
    in >> g.lhs;
    consume_OUTPUT_NEWLINE(in);
    in >> g.rhs;
    consume_OUTPUT_NEWLINE(in);
    in >> g.output.index;
    consume_newline(in);

    return in;
}

template<typename FieldT>
std::vector<size_t> bacs_circuit<FieldT>::wire_depths() const
{
    std::vector<size_t> depths(primary_input_size+auxiliary_input_size, 1);

    for (auto &g: gates)
    {
        size_t max_depth = 0;
        for (auto &w : g.left_wires)
        {
            max_depth = std::max(max_depth, depths[w.wire]);
        }

        for (auto &w : g.right_wires)
        {
            max_depth = std::max(max_depth, depths[w.wire]);
        }

        depths.emplace_back(max_depth + 1);
    }

    return depths;
}

template<typename FieldT>
size_t bacs_circuit<FieldT>::depth() const
{
    std::vector<size_t> all_depths = this->wire_depths();
    return *(std::max_element(all_depths.begin(), all_depths.end()));
}

template<typename FieldT>
bool bacs_circuit<FieldT>::is_valid() const
{
    /*
      Gates must be sorted topologically sorted, and
      the output wire of gates[i] must equal num_inputs+i+1.
      (The '+1' accounts for the constant wire.)
     */
    const size_t num_inputs = primary_input_size+auxiliary_input_size;

    for (size_t i = 0; i < gates.size(); ++i)
    {
        if (gates[i].output.index != num_inputs+i+1)
        {
            return false;
        }

        if (!gates[i].lhs.is_valid(num_inputs) || !gates[i].rhs.is_valid(num_inputs))
        {
            return false;
        }
    }

    return true;
}

template<typename FieldT>
bacs_variable_assignment<FieldT> bacs_circuit<FieldT>::get_all_wires(const bacs_primary_input<FieldT> &primary_input,
                                                                     const bacs_auxiliary_input<FieldT> &auxiliary_input) const
{
    const size_t num_inputs = primary_input_size+auxiliary_input_size;
    bacs_variable_assignment<FieldT> input;
    input.insert(input.end(), primary_input.begin(), primary_input.end());
    input.insert(input.end(), auxiliary_input.begin(), auxiliary_input.end());

    assert(input.size() == num_inputs);
    bacs_variable_assignment<FieldT> result(input);

    for (auto &g : gates)
    {
        const FieldT gate_output = g.evaluate(result);
        result.emplace_back(gate_output);
    }

    return result;
}

template<typename FieldT>
bacs_variable_assignment<FieldT> bacs_circuit<FieldT>::get_all_outputs(const bacs_primary_input<FieldT> &primary_input,
                                                                       const bacs_auxiliary_input<FieldT> &auxiliary_input) const
{
    const bacs_variable_assignment<FieldT> all_wires = get_all_wires(primary_input, auxiliary_input);
    bacs_variable_assignment<FieldT> all_outputs;

    for (size_t i = 0; i < gates.size(); ++i)
    {
        if (gates[i].is_circuit_output)
        {
            all_outputs.emplace_back(all_wires[gates[i].output.index-1]);
        }
    }

    return all_outputs;
}

template<typename FieldT>
bool bacs_circuit<FieldT>::is_satisfied(const bacs_primary_input<FieldT> &primary_input,
                                        const bacs_auxiliary_input<FieldT> &auxiliary_input) const
{
    const bacs_variable_assignment<FieldT> all_outputs = get_all_outputs(primary_input, auxiliary_input);

    for (size_t i = 0; i < all_outputs.size(); ++i)
    {
        if (!all_outputs[i].is_zero())
        {
            return false;
        }
    }

    return true;
}

template<typename FieldT>
void bacs_circuit<FieldT>::add_gate(const bacs_gate<FieldT> &g)
{
    const size_t num_inputs = primary_input_size+auxiliary_input_size;
    assert(g.output.index == num_inputs+gates.size()+1);
    gates.emplace_back(g);
}

template<typename FieldT>
void bacs_circuit<FieldT>::add_gate(const bacs_gate<FieldT> &g, const std::string &annotation)
{
    const size_t num_inputs = primary_input_size+auxiliary_input_size;
    assert(g.output.index == num_inputs+gates.size()+1);
    gates.emplace_back(g);
#ifdef DEBUG
    gate_annotations[g.output.index] = annotation;
#endif
}

template<typename FieldT>
bool bacs_circuit<FieldT>::operator==(const bacs_circuit<FieldT> &other) const
{
    return (this->primary_input_size == other.primary_input_size &&
            this->auxiliary_input_size == other.auxiliary_input_size &&
            vectors_equal<bacs_gate<FieldT> >(this->gates, other.gates));
}

template<typename FieldT>
std::ostream& operator<<(std::ostream &out, const bacs_circuit<FieldT> &circuit)
{
    out << circuit.primary_input_size << "\n";
    out << circuit.auxiliary_input_size << "\n";
    out << circuit.gates << OUTPUT_NEWLINE;

    return out;
}

template<typename FieldT>
std::istream& operator>>(std::istream &in, bacs_circuit<FieldT> &circuit)
{
    in >> circuit.primary_input_size;
    consume_newline(in);
    in >> circuit.auxiliary_input_size;
    consume_newline(in);
    in >> circuit.gates;
    consume_OUTPUT_NEWLINE(in);

    return in;
}

} // libsnark

#endif // BACS_TCC_
