/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef WNAF_TCC_
#define WNAF_TCC_

namespace libsnark {

template<mp_size_t n>
std::vector<long> find_wNAF(const size_t w, const bigint<n> &exp)
{
    const size_t length = exp.max_bits(); // upper bound
    std::vector<long> res(length+1);
    bigint<n> c = exp;
    long j = 0;
    while (!c.is_zero())
    {
        long u;
        if ((c.data[0] & 1) == 1)
        {
            u = c.data[0] % (1u << (w+1));
            if (u > (1 << w))
            {
                u = u - (1 << (w+1));
            }

            if (u > 0)
            {
                mpn_sub_1(c.data, c.data, n, u);
            }
            else
            {
                mpn_add_1(c.data, c.data, n, -u);
            }
        }
        else
        {
            u = 0;
        }
        res[j] = u;
        ++j;

        mpn_rshift(c.data, c.data, n, 1); // c = c/2
    }

    return res;
}

template<typename T, mp_size_t n>
T fixed_window_wnaf_exp(const size_t window, const T &neutral, const T &base, const bigint<n> &scalar)
{
    std::vector<long> naf = find_wNAF(window, scalar);
    std::vector<T> table(1u<<(window-1));
    T tmp = base;
    T dbl = base.dbl();
    for (size_t i = 0; i < 1u<<(window-1); ++i)
    {
        table[i] = tmp;
        tmp = tmp + dbl;
    }

    T res = neutral;
    bool found_nonzero = false;
    for (long i = naf.size()-1; i >= 0; --i)
    {
        if (found_nonzero)
        {
            res = res.dbl();
        }

        if (naf[i] != 0)
        {
            found_nonzero = true;
            if (naf[i] > 0)
            {
                res = res + table[naf[i]/2];
            }
            else
            {
                res = res - table[(-naf[i])/2];
            }
        }
    }

    return res;
}

template<typename T, mp_size_t n>
T opt_window_wnaf_exp(const T &neutral, const T &base, const bigint<n> &scalar, const size_t scalar_bits)
{
    size_t best = 0;
    for (long i = T::wnaf_window_table.size() - 1; i >= 0; --i)
    {
        if (scalar_bits >= T::wnaf_window_table[i])
        {
            best = i+1;
            break;
        }
    }

    if (best > 0)
    {
        return fixed_window_wnaf_exp(best, neutral, base, scalar);
    }
    else
    {
        return scalar * base;
    }
}

} // libsnark

#endif // WNAF_TCC_
