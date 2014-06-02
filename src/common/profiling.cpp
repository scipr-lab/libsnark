/** @file
 ********************************************************************************
 Implementation of functions for profiling code blocks.
 Reports time, op counts, memory usage etc.
 ********************************************************************************
 * @authors    Eli Ben-Sasson, Alessandro Chiesa, Daniel Genkin,
 *             Shaul Kfir, Eran Tromer, Madars Virza.
 * This file is part of libsnark, developed by SCIPR Lab <http://scipr-lab.org>.
 * @copyright  MIT license (see LICENSE file)
 *******************************************************************************/

#include "common/profiling.hpp"
#include <cassert>
#include <ctime>
#include <cstdio>
#include <list>
#include <vector>
#include "common/types.hpp"

#ifndef MINDEPS
#include <proc/readproc.h>
#endif

namespace libsnark {

timespec start_time, last_time;

int64_t nsec_diff(timespec stop, timespec start)
{
    return
        1000000000LL * (stop.tv_sec - start.tv_sec) +
        stop.tv_nsec - start.tv_nsec;
}

long long get_nsec_time()
{
    timespec cur;
    clock_gettime(CLOCK_REALTIME, &cur);
    return 1000000000LL * cur.tv_sec + cur.tv_nsec;
}

void start_profiling()
{
    printf("Reset time counters for profiling\n");

    clock_gettime(CLOCK_REALTIME, &start_time);
    last_time = start_time;
}

std::map<std::string, size_t> invocation_counts;
std::map<std::string, timespec> enter_times;
std::map<std::string, long long> last_times;
std::map<std::string, long long> cumulative_times;
std::map<std::pair<std::string, std::string>, long long> op_counts;
std::map<std::pair<std::string, std::string>, long long> cumulative_op_counts; // ((msg, data_point), value)
size_t indentation = 0;

std::vector<std::string> block_names;

std::list<std::pair<std::string, long long*> > op_data_points = {
#ifdef PROFILE_OP_COUNTS
    std::make_pair("Fradd", &Fr<default_pp>::add_cnt),
    std::make_pair("Frsub", &Fr<default_pp>::sub_cnt),
    std::make_pair("Frmul", &Fr<default_pp>::mul_cnt),
    std::make_pair("Frinv", &Fr<default_pp>::inv_cnt),
    std::make_pair("Fqadd", &Fq<default_pp>::add_cnt),
    std::make_pair("Fqsub", &Fq<default_pp>::sub_cnt),
    std::make_pair("Fqmul", &Fq<default_pp>::mul_cnt),
    std::make_pair("Fqinv", &Fq<default_pp>::inv_cnt),
    std::make_pair("G1add", &G1<default_pp>::add_cnt),
    std::make_pair("G1dbl", &G1<default_pp>::dbl_cnt),
    std::make_pair("G2add", &G2<default_pp>::add_cnt),
    std::make_pair("G2dbl", &G2<default_pp>::dbl_cnt)
#endif
};

bool inhibit_profiling_info = false;
bool inhibit_profiling_counters = false;

void print_cumulative_times(const long long factor)
{
    printf("Dumping times:\n");
    for (auto& kv : cumulative_times)
    {
        printf("   %-45s: %8.5fms\n", kv.first.c_str(), (kv.second * 1e-6) / factor);
    }
}

void print_cumulative_op_counts(const bool only_fq)
{
#ifdef PROFILE_OP_COUNTS
    printf("Dumping operation counts:\n");
    for (auto& msg : invocation_counts)
    {
        printf("  %-45s: ", msg.first.c_str());
        bool first = true;
        for (auto& data_point : op_data_points)
        {
            if (only_fq && data_point.first.compare(0, 2, "Fq") != 0)
            {
                continue;
            }

            if (!first)
            {
                printf(", ");
            }
            printf("%-5s = %7.0f (%3zu)",
                   data_point.first.c_str(),
                   1. * cumulative_op_counts[std::make_pair(msg.first, data_point.first)] / msg.second,
                   msg.second);
            first = false;
        }
        printf("\n");
    }
#endif
}

void print_op_profiling(const std::string &msg)
{
#ifdef PROFILE_OP_COUNTS
    printf("\n");
    print_indent();

    printf("(opcounts) = (");
    bool first = true;
    for (std::pair<std::string, long long*> p : op_data_points)
    {
        if (!first)
        {
            printf(", ");
        }

        printf("%s=%lld", p.first.c_str(), *(p.second)-op_counts[std::make_pair(msg, p.first)]);
        first = false;
    }
    printf(")");
#endif
}

void print_time(const char* msg)
{
    if (inhibit_profiling_info)
    {
        return;
    }

    timespec t;
    clock_gettime(CLOCK_REALTIME, &t);

    printf("%-35s\t[%0.4fs]\t(%0.4fs from start)",
           msg, nsec_diff(t, last_time) * 1e-9, nsec_diff(t, start_time) * 1e-9);

#ifdef PROFILE_OP_COUNTS
    print_op_profiling(msg);
#endif
    printf("\n");

    fflush(stdout);
    last_time = t;
}

void print_header(const char *msg)
{
    printf("\n================================================================================\n");
    printf("%s\n", msg);
    printf("================================================================================\n\n");
}

void print_indent()
{
    for (size_t i = 0; i < indentation; ++i)
    {
        printf("  ");
    }
}

void op_profiling_enter(const std::string &msg)
{
    for (std::pair<std::string, long long*> p : op_data_points)
    {
        op_counts[std::make_pair(msg, p.first)] = *(p.second);
    }
}

void enter_block(const std::string &msg, const bool indent)
{
    if (inhibit_profiling_counters)
    {
        return;
    }

    block_names.emplace_back(msg);
    timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    enter_times[msg] = t;

    if (inhibit_profiling_info)
    {
        return;
    }
#pragma omp critical
    {
        op_profiling_enter(msg);

        print_indent();
        printf("(enter) %-35s\t[0s]\t(%0.4fs from start)\n",
               msg.c_str(), nsec_diff(t, start_time) * 1e-9);
        fflush(stdout);

        if (indent)
        {
            ++indentation;
        }
    }
}

void leave_block(const std::string &msg, const bool indent)
{
    if (inhibit_profiling_counters)
    {
        return;
    }

#ifndef MULTICORE
    assert(*(--block_names.end()) == msg);
#endif
    block_names.pop_back();

    ++invocation_counts[msg];

    timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    last_times[msg] = nsec_diff(t, enter_times[msg]);
    cumulative_times[msg] += nsec_diff(t, enter_times[msg]);

#ifdef PROFILE_OP_COUNTS
    for (std::pair<std::string, long long*> p : op_data_points)
    {
        cumulative_op_counts[std::make_pair(msg, p.first)] += *(p.second)-op_counts[std::make_pair(msg, p.first)];
    }
#endif

    if (inhibit_profiling_info)
    {
        return;
    }
#pragma omp critical
    {
        if (indent)
        {
            --indentation;
        }

        print_indent();
        printf("(leave) %-35s\t[%0.4fs]\t(%0.4fs from start)",
               msg.c_str(), nsec_diff(t, enter_times[msg]) * 1e-9, nsec_diff(t, start_time) * 1e-9);
        print_op_profiling(msg);
        printf("\n");
        fflush(stdout);
    }
}

double get_time()
{
    timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return nsec_diff(t, start_time) * 1e-9;
}

void print_mem(const std::string &s)
{
#ifndef MINDEPS
    struct proc_t usage;
    look_up_our_self(&usage);
    if (s.empty())
    {
        printf("* Peak vsize (physical memory+swap) in mebibytes: %lu\n", usage.vsize >> 20);
    }
    else
    {
        printf("* Peak vsize (physical memory+swap) in mebibytes (%s): %lu\n", s.c_str(), usage.vsize >> 20);
    }
#else
    printf("* Memory profiling not supported in MINDEPS mode\n");
#endif
}

void print_compilation_info()
{
#ifdef __GNUC__
    printf("g++ version: %s\n", __VERSION__);
    printf("Compiled on %s %s\n", __DATE__, __TIME__);
#endif
#ifdef STATIC
    printf("STATIC: yes\n");
#else
    printf("STATIC: no\n");
#endif
#ifdef MULTICORE
    printf("MULTICORE: yes\n");
#else
    printf("MULTICORE: no\n");
#endif
#ifdef DEBUG
    printf("DEBUG: yes\n");
#else
    printf("DEBUG: no\n");
#endif
#ifdef PROFILE_OP_COUNTS
    printf("PROFILE_OP_COUNTS: yes\n");
#else
    printf("PROFILE_OP_COUNTS: no\n");
#endif
#ifdef _GLIBCXX_DEBUG
    printf("_GLIBCXX_DEBUG: yes\n");
#else
    printf("_GLIBCXX_DEBUG: no\n");
#endif
}

} // libsnark
