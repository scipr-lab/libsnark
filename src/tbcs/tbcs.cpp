/** @file
 *****************************************************************************

 Implementation of interfaces for:
 - a TBCS gate,
 - a TBCS variable assignment, and
 - a TBCS constraint system.

 See tbcs.hpp .

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#include "tbcs/tbcs.hpp"

#include <algorithm>
#include "common/utils.hpp"

namespace libsnark {

bool tbcs_gate::evaluate(const tbcs_variable_assignment &input) const
{
    // This function is very tricky, see comment in tbcs.hpp

    const bool X = (left_wire == 0 ? true : input[left_wire - 1]);
    const bool Y = (right_wire == 0 ? true : input[right_wire - 1]);

    const size_t pos = 3 - ((X ? 2 : 0) + (Y ? 1 : 0)); /* 3 - ... inverts position */

    return (((int)type) & (1u << pos));
}

void tbcs_gate::print(const std::map<size_t, std::string> &variable_annotations) const
{
    // TODO: should find a clang++ libstd::map debug build bug and fix it together with this (too complicated to explain in a comment)
}

bool tbcs_gate::operator==(const tbcs_gate &other) const
{
    return (this->left_wire == other.left_wire &&
            this->right_wire == other.right_wire &&
            this->type == other.type &&
            this->output == other.output &&
            this->is_circuit_output == other.is_circuit_output);
}

std::ostream& operator<<(std::ostream &out, const tbcs_gate &g)
{
    out << g.left_wire << "\n";
    out << g.right_wire << "\n";
    out << (int)g.type << "\n";
    out << g.output << "\n";
    output_bool(out, g.is_circuit_output);

    return out;
}

std::istream& operator>>(std::istream &in, tbcs_gate &g)
{
    in >> g.left_wire;
    consume_newline(in);
    in >> g.right_wire;
    consume_newline(in);
    int tmp;
    in >> tmp;
    g.type = (tbcs_gate_type)tmp;
    consume_newline(in);
    in >> g.output;
    input_bool(in, g.is_circuit_output);

    return in;
}

std::vector<size_t> tbcs_circuit::wire_depths() const
{
    std::vector<size_t> depths(primary_input_size + auxiliary_input_size, 1);

    for (auto &g: gates)
    {
        depths.emplace_back(std::max(depths[g.left_wire], depths[g.right_wire]) + 1);
    }

    return depths;
}

size_t tbcs_circuit::depth() const
{
    std::vector<size_t> all_depths = this->wire_depths();
    return *(std::max_element(all_depths.begin(), all_depths.end()));
}

bool tbcs_circuit::is_valid() const
{
    /*
     Gates must be sorted topologically sorted, and
     the output wire of gates[i] must equal num_inputs+i+1.
     (The '+1' accounts for the constant wire.)
     */
    const size_t num_inputs = primary_input_size+auxiliary_input_size;

    for (size_t i = 0; i < gates.size(); ++i)
    {
        if (gates[i].output != num_inputs+i+1)
        {
            return false;
        }
    }

    return true;
}

tbcs_variable_assignment tbcs_circuit::get_all_wires(const tbcs_primary_input &primary_input,
                                                     const tbcs_auxiliary_input &auxiliary_input) const
{
    assert(primary_input.size() == primary_input_size);
    assert(auxiliary_input.size() == auxiliary_input_size);
    tbcs_variable_assignment result;
    result.insert(result.end(), primary_input.begin(), primary_input.end());
    result.insert(result.end(), auxiliary_input.begin(), auxiliary_input.end());

    for (auto &g : gates)
    {
        const bool gate_output = g.evaluate(result);
        result.push_back(gate_output);
    }

    return result;
}

tbcs_variable_assignment tbcs_circuit::get_all_outputs(const tbcs_primary_input &primary_input,
                                                       const tbcs_auxiliary_input &auxiliary_input) const
{
    const tbcs_variable_assignment all_wires = get_all_wires(primary_input, auxiliary_input);
    tbcs_variable_assignment all_outputs;

    for (auto &g : gates)
    {
        if (g.is_circuit_output)
        {
            all_outputs.push_back(all_wires[g.output-1]);
        }
    }

    return all_outputs;
}


bool tbcs_circuit::is_satisfied(const tbcs_primary_input &primary_input,
                                const tbcs_auxiliary_input &auxiliary_input) const
{
    const tbcs_variable_assignment all_outputs = get_all_outputs(primary_input, auxiliary_input);
    for (size_t i = 0; i < all_outputs.size(); ++i)
    {
        if (all_outputs[i])
        {
            return false;
        }
    }

    return true;
}

void tbcs_circuit::add_gate(const tbcs_gate &g)
{
    const size_t num_inputs = primary_input_size+auxiliary_input_size;
    assert(g.output == num_inputs+gates.size()+1);
    gates.emplace_back(g);
}

void tbcs_circuit::add_gate(const tbcs_gate &g, const std::string &annotation)
{
    const size_t num_inputs = primary_input_size+auxiliary_input_size;
    assert(g.output == num_inputs+gates.size()+1);
    gates.emplace_back(g);
#ifdef DEBUG
    gate_annotations[g.output] = annotation;
#endif
}

bool tbcs_circuit::operator==(const tbcs_circuit &other) const
{
    return (this->primary_input_size == other.primary_input_size &&
            this->auxiliary_input_size == other.auxiliary_input_size &&
            vectors_equal<tbcs_gate>(this->gates, other.gates));
}

std::ostream& operator<<(std::ostream &out, const tbcs_circuit &circuit)
{
    out << circuit.primary_input_size << "\n";
    out << circuit.auxiliary_input_size << "\n";
    out << circuit.gates << OUTPUT_NEWLINE;

    return out;
}

std::istream& operator>>(std::istream &in, tbcs_circuit &circuit)
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
