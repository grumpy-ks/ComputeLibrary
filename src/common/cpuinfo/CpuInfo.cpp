/*
 * Copyright (c) 2021-2025 Arm Limited.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "src/common/cpuinfo/CpuInfo.h"

#include "arm_compute/core/Error.h"
#include "arm_compute/core/Log.h"

#include "support/StringSupport.h"
#include "support/ToolchainSupport.h"

#include <map>
#include <sstream>

#if !defined(BARE_METAL)
#include <algorithm>
#include <cstring>
#include <fstream>
#if !defined(_WIN64)
#include <regex.h> /* C++ std::regex takes up a lot of space in the standalone builds */
#include <sched.h>
#else  /*  !defined(_WIN64) */
// clang-format off
#include <windows.h>
#include <sysinfoapi.h>
#include <processthreadsapi.h>
// clang-format on
#endif /* !defined(_WIN64) */

#include <thread>
#include <unordered_map>
#endif /* !defined(BARE_METAL) */

#if !defined(_WIN64)
#if !defined(BARE_METAL) && !defined(__APPLE__) && !defined(__OpenBSD__) && !defined(__QNX__) && \
    (defined(__arm__) || defined(__aarch64__))
#include <asm/hwcap.h> /* Get HWCAP bits from asm/hwcap.h */
#include <sys/auxv.h>
#elif (defined(__OpenBSD__) || defined(__APPLE__)) && defined(__aarch64__)
#include <sys/sysctl.h>
#include <sys/types.h>
#endif /* defined(__APPLE__) && defined(__aarch64__)) */
#endif /* !defined(BARE_METAL) && !defined(__APPLE__) && !defined(__OpenBSD__) && !defined(__QNX__) && (defined(__arm__) || defined(__aarch64__)) */

#define ARM_COMPUTE_CPU_FEATURE_HWCAP_CPUID    (1 << 11)
#define ARM_COMPUTE_GET_FEATURE_REG(var, freg) __asm __volatile("MRS %0, " #freg : "=r"(var))
namespace arm_compute
{
namespace cpuinfo
{
namespace
{
#if !defined(_WIN64) && !defined(BARE_METAL) && !defined(__APPLE__) && !defined(__OpenBSD__) && !defined(__QNX__) && \
    (defined(__arm__) || defined(__aarch64__))
/** Extract MIDR using CPUID information that are exposed to user-space
 *
 * @param[in] max_num_cpus Maximum number of possible CPUs
 *
 * @return std::vector<uint32_t> A list of the MIDR of each core
 */
std::vector<uint32_t> midr_from_cpuid(uint32_t max_num_cpus)
{
    std::vector<uint32_t> cpus;
    for (unsigned int i = 0; i < max_num_cpus; ++i)
    {
        std::stringstream str;
        str << "/sys/devices/system/cpu/cpu" << i << "/regs/identification/midr_el1";
        std::ifstream file(str.str(), std::ios::in);
        if (file.is_open())
        {
            std::string line;
            if (bool(getline(file, line)))
            {
                cpus.emplace_back(support::cpp11::stoul(line, nullptr, support::cpp11::NumericBase::BASE_16));
            }
        }
    }
    return cpus;
}

/** Extract MIDR by parsing the /proc/cpuinfo meta-data
 *
 * @param[in] max_num_cpus Maximum number of possible CPUs
 *
 * @return std::vector<uint32_t> A list of the MIDR of each core
 */
std::vector<uint32_t> midr_from_proc_cpuinfo(int max_num_cpus)
{
    std::vector<uint32_t> cpus;

    regex_t proc_regex;
    regex_t imp_regex;
    regex_t var_regex;
    regex_t part_regex;
    regex_t rev_regex;

    memset(&proc_regex, 0, sizeof(regex_t));
    memset(&imp_regex, 0, sizeof(regex_t));
    memset(&var_regex, 0, sizeof(regex_t));
    memset(&part_regex, 0, sizeof(regex_t));
    memset(&rev_regex, 0, sizeof(regex_t));

    int ret_status = 0;
    // If "long-form" cpuinfo is present, parse that to populate models.
    ret_status |= regcomp(&proc_regex, R"(^processor.*([[:digit:]]+)$)", REG_EXTENDED);
    ret_status |= regcomp(&imp_regex, R"(^CPU implementer.*0x(..)$)", REG_EXTENDED);
    ret_status |= regcomp(&var_regex, R"(^CPU variant.*0x(.)$)", REG_EXTENDED);
    ret_status |= regcomp(&part_regex, R"(^CPU part.*0x(...)$)", REG_EXTENDED);
    ret_status |= regcomp(&rev_regex, R"(^CPU revision.*([[:digit:]]+)$)", REG_EXTENDED);
    ARM_COMPUTE_UNUSED(ret_status);
    ARM_COMPUTE_ERROR_ON_MSG(ret_status != 0, "Regex compilation failed.");

    std::ifstream file("/proc/cpuinfo", std::ios::in);
    if (file.is_open())
    {
        std::string line;
        int         midr   = 0;
        int         curcpu = -1;

        while (bool(getline(file, line)))
        {
            std::array<regmatch_t, 2> match;
            ret_status = regexec(&proc_regex, line.c_str(), 2, match.data(), 0);
            if (ret_status == 0)
            {
                std::string id     = line.substr(match[1].rm_so, (match[1].rm_eo - match[1].rm_so));
                int         newcpu = support::cpp11::stoi(id, nullptr);

                if (curcpu >= 0 && midr == 0)
                {
                    // Matched a new CPU ID without any description of the previous one - looks like old format.
                    return {};
                }

                if (curcpu >= 0 && curcpu < max_num_cpus)
                {
                    cpus.emplace_back(midr);
                }
                else
                {
                    ARM_COMPUTE_LOG_INFO_MSG_CORE(
                        "Trying to populate a core id with id greater than the expected number of cores!");
                }

                midr   = 0;
                curcpu = newcpu;

                continue;
            }

            ret_status = regexec(&imp_regex, line.c_str(), 2, match.data(), 0);
            if (ret_status == 0)
            {
                std::string subexp = line.substr(match[1].rm_so, (match[1].rm_eo - match[1].rm_so));
                int         impv   = support::cpp11::stoi(subexp, nullptr, support::cpp11::NumericBase::BASE_16);
                midr |= (impv << 24);

                continue;
            }

            ret_status = regexec(&var_regex, line.c_str(), 2, match.data(), 0);
            if (ret_status == 0)
            {
                std::string subexp = line.substr(match[1].rm_so, (match[1].rm_eo - match[1].rm_so));
                int         varv   = support::cpp11::stoi(subexp, nullptr, support::cpp11::NumericBase::BASE_16);
                midr |= (varv << 20);

                continue;
            }

            ret_status = regexec(&part_regex, line.c_str(), 2, match.data(), 0);
            if (ret_status == 0)
            {
                std::string subexp = line.substr(match[1].rm_so, (match[1].rm_eo - match[1].rm_so));
                int         partv  = support::cpp11::stoi(subexp, nullptr, support::cpp11::NumericBase::BASE_16);
                midr |= (partv << 4);

                continue;
            }

            ret_status = regexec(&rev_regex, line.c_str(), 2, match.data(), 0);
            if (ret_status == 0)
            {
                std::string subexp = line.substr(match[1].rm_so, (match[1].rm_eo - match[1].rm_so));
                int         regv   = support::cpp11::stoi(subexp, nullptr);
                midr |= (regv);
                midr |= (0xf << 16);

                continue;
            }
        }

        if (curcpu >= 0 && curcpu < max_num_cpus)
        {
            cpus.emplace_back(midr);
        }
        else
        {
            ARM_COMPUTE_LOG_INFO_MSG_CORE(
                "Trying to populate a core id with id greater than the expected number of cores!");
        }
    }

    // Free allocated memory
    regfree(&proc_regex);
    regfree(&imp_regex);
    regfree(&var_regex);
    regfree(&part_regex);
    regfree(&rev_regex);

    return cpus;
}

/** Get the maximim number of CPUs in the system by parsing /sys/devices/system/cpu/present
 *
 * @return int Maximum number of CPUs
 */
int get_max_cpus()
{
    int           max_cpus = 1;
    std::ifstream CPUspresent;
    CPUspresent.open("/sys/devices/system/cpu/present", std::ios::in);
    bool success = false;

    if (CPUspresent.is_open())
    {
        std::string line;

        if (bool(getline(CPUspresent, line)))
        {
            /* The content of this file is a list of ranges or single values, e.g.
                 * 0-5, or 1-3,5,7 or similar.  As we are interested in the
                 * max valid ID, we just need to find the last valid
                 * delimiter ('-' or ',') and parse the integer immediately after that.
                 */
            auto startfrom = line.begin();

            for (auto i = line.begin(); i < line.end(); ++i)
            {
                if (*i == '-' || *i == ',')
                {
                    startfrom = i + 1;
                }
            }

            line.erase(line.begin(), startfrom);

            max_cpus = support::cpp11::stoi(line, nullptr) + 1;
            success  = true;
        }
    }

    // Return std::thread::hardware_concurrency() as a fallback.
    if (!success)
    {
        max_cpus = std::thread::hardware_concurrency();
    }
    return max_cpus;
}
#if defined(__ANDROID__)
std::vector<uint32_t> get_cpu_capacities()
{
    std::vector<uint32_t> cpu_capacities;
    for (int i = 0; i < get_max_cpus(); ++i)
    {
        std::stringstream str;
        str << "/sys/devices/system/cpu/cpu" << i << "/cpu_capacity";
        std::ifstream file(str.str(), std::ios::in);
        if (file.is_open())
        {
            std::string line;
            if (bool(getline(file, line)))
            {
                cpu_capacities.emplace_back(support::cpp11::stoul(line));
            }
        }
    }

    return cpu_capacities;
}

uint32_t not_little_num_cpus_internal()
{
    std::vector<uint32_t> cpus_all = get_cpu_capacities();
    std::vector<uint32_t> cpus_not_little;

    std::vector<uint32_t>::iterator result       = std::max_element(cpus_all.begin(), cpus_all.end());
    uint32_t                        max_capacity = *result;
    uint32_t                        threshold    = max_capacity / 2;
    for (unsigned int i = 0; i < cpus_all.size(); i++)
    {
        if (!(cpus_all[i] < threshold))
        {
            cpus_not_little.emplace_back(cpus_all[i]);
        }
    }
    return cpus_not_little.size();
}
#endif /* defined(__ANDROID__) */
#elif defined(__aarch64__) && \
    defined(__APPLE__) /* !defined(BARE_METAL) && !defined(__APPLE__) && (defined(__arm__) || defined(__aarch64__)) */
/** Query features through sysctlbyname
  *
  * @return int value queried
  */
int get_hw_capability(const std::string &cap)
{
    int64_t result(0);
    size_t  size = sizeof(result);
    sysctlbyname(cap.c_str(), &result, &size, NULL, 0);
    return result;
}
#endif /* !defined(BARE_METAL) && !defined(__APPLE__) && !defined(__OpenBSD__) && (defined(__arm__) || defined(__aarch64__)) */

#if defined(BARE_METAL) && defined(__aarch64__)
uint64_t get_sve_feature_reg()
{
    uint64_t svefr0 = 0;
    __asm __volatile(".inst 0xd5380483 // mrs x3, ID_AA64ZFR0_EL1\n"
                     "MOV  %0, X3"
                     : "=r"(svefr0)
                     :
                     : "x3");
    return svefr0;
}
#endif /* defined(BARE_METAL) && defined(__aarch64__) */
} // namespace

CpuInfo::CpuInfo(CpuIsaInfo isa, std::vector<CpuModel> cpus) : _isa(std::move(isa)), _cpus(std::move(cpus))
{
}

CpuInfo CpuInfo::build()
{
#if !defined(_WIN64) && !defined(BARE_METAL) && !defined(__APPLE__) && !defined(__OpenBSD__) && !defined(__QNX__) && \
    (defined(__arm__) || defined(__aarch64__))
    const uint32_t hwcaps   = getauxval(AT_HWCAP);
    const uint32_t hwcaps2  = getauxval(AT_HWCAP2);
    const uint32_t max_cpus = get_max_cpus();

    // Populate midr values
    std::vector<uint32_t> cpus_midr;
    if (hwcaps & ARM_COMPUTE_CPU_FEATURE_HWCAP_CPUID)
    {
        cpus_midr = midr_from_cpuid(max_cpus);
    }
    if (cpus_midr.empty())
    {
        cpus_midr = midr_from_proc_cpuinfo(max_cpus);
    }
    if (cpus_midr.empty())
    {
        cpus_midr.resize(max_cpus, 0);
    }

    // Populate isa (Assume homogeneous ISA specification)
    CpuIsaInfo isa = init_cpu_isa_from_hwcaps(hwcaps, hwcaps2, cpus_midr.back());

    // Convert midr to models
    std::vector<CpuModel> cpus_model;
    std::transform(std::begin(cpus_midr), std::end(cpus_midr), std::back_inserter(cpus_model),
                   [](uint32_t midr) -> CpuModel { return midr_to_model(midr); });

    CpuInfo info(isa, cpus_model);
    return info;
#elif defined(__OpenBSD__)
    int    mib[2] = {0, 0};
    int    ncpu   = {1};
    size_t len    = sizeof(ncpu);
    mib[0]        = CTL_HW;
    mib[1]        = HW_NCPU;
    if (sysctl(mib, 2, &ncpu, &len, NULL, 0) == -1)
    {
        // if the system call fails we set number of cpus to 1
        ncpu = 1;
    }
    CpuIsaInfo            isainfo;
    std::vector<CpuModel> cpus_model(ncpu);
    isainfo.neon = true;
    CpuInfo info(isainfo, cpus_model);
    return info;
#elif (BARE_METAL) && \
    defined(          \
        __aarch64__) /* !defined(BARE_METAL) && !defined(__APPLE__) && !defined(__OpenBSD__) && !defined(__QNX__) && (defined(__arm__) || defined(__aarch64__)) */

    // Assume single CPU in bare metal mode.  Just read the ID register and feature bits directly.
    uint64_t isar0 = 0, isar1 = 0, pfr0 = 0, pfr1 = 0, svefr0 = 0, midr = 0;
    ARM_COMPUTE_GET_FEATURE_REG(isar0, ID_AA64ISAR0_EL1);
    ARM_COMPUTE_GET_FEATURE_REG(isar1, ID_AA64ISAR1_EL1);
    ARM_COMPUTE_GET_FEATURE_REG(pfr0, ID_AA64PFR0_EL1);
    ARM_COMPUTE_GET_FEATURE_REG(pfr1, ID_AA64PFR1_EL1);
    ARM_COMPUTE_GET_FEATURE_REG(midr, MIDR_EL1);
    if ((pfr0 >> 32) & 0xf)
    {
        svefr0 = get_sve_feature_reg();
    }

    CpuIsaInfo            isa = init_cpu_isa_from_regs(isar0, isar1, pfr0, pfr1, svefr0, midr);
    std::vector<CpuModel> cpus_model(1, midr_to_model(midr));
    CpuInfo               info(isa, cpus_model);
    return info;
#elif defined(__aarch64__) && \
    (defined(__OpenBSD__) || defined(__APPLE__)) /* #elif(BARE_METAL) && defined(__aarch64__) */
    int                   ncpus = get_hw_capability("hw.perflevel0.logicalcpu");
    CpuIsaInfo            isainfo;
    std::vector<CpuModel> cpus_model(ncpus);
    isainfo.neon = get_hw_capability("hw.optional.neon");
    isainfo.fp16 = get_hw_capability("hw.optional.neon_fp16");
    isainfo.dot  = get_hw_capability("hw.optional.arm.FEAT_DotProd");
    isainfo.bf16 = get_hw_capability("hw.optional.arm.FEAT_BF16");
    isainfo.i8mm = get_hw_capability("hw.optional.arm.FEAT_I8MM");
    isainfo.sme  = get_hw_capability("hw.optional.arm.FEAT_SME");
    isainfo.sme2 = get_hw_capability("hw.optional.arm.FEAT_SME2");
    CpuInfo info(isainfo, cpus_model);
    return info;
#elif defined(__aarch64__) && defined(_WIN64)    /* #elif defined(__aarch64__) && defined(__APPLE__) */
    CpuIsaInfo isainfo;
    isainfo.neon = IsProcessorFeaturePresent(PF_ARM_NEON_INSTRUCTIONS_AVAILABLE);
    isainfo.dot  = IsProcessorFeaturePresent(PF_ARM_V82_DP_INSTRUCTIONS_AVAILABLE);
    if (NTDDI_VERSION >= NTDDI_WIN11_GE)
    {
        isainfo.fp16 = IsProcessorFeaturePresent(PF_ARM_SVE_INSTRUCTIONS_AVAILABLE);
        isainfo.sve  = IsProcessorFeaturePresent(PF_ARM_SVE_INSTRUCTIONS_AVAILABLE);
        isainfo.i8mm = IsProcessorFeaturePresent(PF_ARM_SVE_I8MM_INSTRUCTIONS_AVAILABLE);
    }
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    const int             ncpus = sysinfo.dwNumberOfProcessors;
    std::vector<CpuModel> cpus_model(ncpus);
    CpuInfo               info(isainfo, cpus_model);
    return info;
#else                                            /* #elif defined(__aarch64__) && defined(_WIN64) */
    CpuInfo info(CpuIsaInfo(), {CpuModel::GENERIC});
    return info;
#endif /* !defined(BARE_METAL) && !defined(__APPLE__) && !defined(__OpenBSD__) && (defined(__arm__) || defined(__aarch64__)) */
}

CpuModel CpuInfo::cpu_model(uint32_t cpuid) const
{
    if (cpuid < _cpus.size())
    {
        return _cpus[cpuid];
    }
    return CpuModel::GENERIC;
}

CpuModel CpuInfo::cpu_model() const
{
#if defined(_WIN64) || defined(BARE_METAL) || defined(__APPLE__) || defined(__OpenBSD__) || defined(__QNX__) || \
    (!defined(__arm__) && !defined(__aarch64__))
    return cpu_model(0);
#else /* defined(BARE_METAL) || defined(__APPLE__) || defined(__OpenBSD__) || defined(__QNX__) || (!defined(__arm__) && !defined(__aarch64__)) */
    return cpu_model(sched_getcpu());
#endif /* defined(BARE_METAL) || defined(__APPLE__) || defined(__OpenBSD__) || (!defined(__arm__) && !defined(__aarch64__)) */
}

uint32_t CpuInfo::num_cpus() const
{
    return _cpus.size();
}

uint32_t CpuInfo::not_little_num_cpus() const
{
#if defined(__ANDROID__)
    return not_little_num_cpus_internal();
#else  /* defined(__ANDROID__) */
    return num_cpus();
#endif /* defined(__ANDROID__) */
}

uint32_t num_threads_hint()
{
    unsigned int num_threads_hint = 1;

#if !defined(BARE_METAL) && !defined(_WIN64) && !defined(ARM_COMPUTE_DISABLE_THREADS_HINT)
    std::vector<std::string> cpus;
    cpus.reserve(64);

    // CPU part regex
    regex_t cpu_part_rgx;
    memset(&cpu_part_rgx, 0, sizeof(regex_t));
    int ret_status = regcomp(&cpu_part_rgx, R"(.*CPU part.+/?\:[[:space:]]+([[:alnum:]]+).*)", REG_EXTENDED);
    ARM_COMPUTE_UNUSED(ret_status);
    ARM_COMPUTE_ERROR_ON_MSG(ret_status != 0, "Regex compilation failed.");

    // Read cpuinfo and get occurrence of each core
    std::ifstream cpuinfo_file("/proc/cpuinfo", std::ios::in);
    if (cpuinfo_file.is_open())
    {
        std::string line;
        while (bool(getline(cpuinfo_file, line)))
        {
            std::array<regmatch_t, 2> match;
            if (regexec(&cpu_part_rgx, line.c_str(), 2, match.data(), 0) == 0)
            {
                cpus.emplace_back(line.substr(match[1].rm_so, (match[1].rm_eo - match[1].rm_so)));
            }
        }
    }
    regfree(&cpu_part_rgx);

    // Get min number of threads
    std::sort(std::begin(cpus), std::end(cpus));
    auto least_frequent_cpu_occurences = [](const std::vector<std::string> &cpus) -> uint32_t
    {
        std::unordered_map<std::string, uint32_t> cpus_freq;
        for (const auto &cpu : cpus)
        {
            cpus_freq[cpu]++;
        }

        uint32_t vmin = cpus.size() + 1;
        for (const auto &cpu_freq : cpus_freq)
        {
            vmin = std::min(vmin, cpu_freq.second);
        }
        return vmin;
    };

    // Set thread hint
    num_threads_hint = cpus.empty() ? std::thread::hardware_concurrency() : least_frequent_cpu_occurences(cpus);
#endif /* !defined(BARE_METAL) && !defined(_WIN64) && !defined(ARM_COMPUTE_DISABLE_THREADS_HINT) */

    return num_threads_hint;
}
} // namespace cpuinfo
} // namespace arm_compute
