/*
 * Copyright (c) 2021 Arm Limited.
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
#ifndef ARM_COMPUTE_CPU_QUANTIZATION_H
#define ARM_COMPUTE_CPU_QUANTIZATION_H

#include "arm_compute/core/ITensorInfo.h"
#include "arm_compute/core/experimental/Types.h"
#include "src/core/cpu/ICpuKernel.h"
#include "src/runtime/cpu/ICpuOperator.h"

#include <memory>

namespace arm_compute
{
namespace cpu
{
/** Basic function to simulate a quantization layer. This function calls the following Arm(R) Neon(TM) kernels:
 *
 *
 * -# @ref kernels::CpuQuantizationKernel
 *
 */
class CpuQuantization : public ICpuOperator
{
public:
    /** Default Constructor */
    CpuQuantization() = default;
    /** Set the input and output tensors.
     *
     * @param[in]  src Source tensor info. The dimensions over the third will be interpreted as batches. Data types supported: QASYMM8/QASYMM8_SIGNED/F32/F16.
     * @param[out] dst Destination tensor info with the same dimensions of input. Data types supported: QASYMM8/QASYMM8_SIGNED/QASYMM16
     */
    void configure(ITensorInfo *src, ITensorInfo *dst);
    /** Static function to check if given info will lead to a valid configuration of @ref CpuQuantization
     *
     * @param[in] src Input tensor info. The dimensions over the third will be interpreted as batches. Data types supported: QASYMM8/QASYMM8_SIGNED/F32/F16.
     * @param[in] dst Output tensor info. Data types supported: QASYMM8/QASYMM8_SIGNED/QASYMM16
     *
     * @return a status
     */
    static Status validate(const ITensorInfo *src, const ITensorInfo *dst);

    // Inherited methods overridden:
    void run(ITensorPack &tensors) override;
};
} // namespace cpu
} // namespace arm_compute
#endif /* ARM_COMPUTE_CPU_QUANTIZATION_H */
