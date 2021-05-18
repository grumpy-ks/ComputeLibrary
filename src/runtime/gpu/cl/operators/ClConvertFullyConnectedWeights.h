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
#ifndef ARM_COMPUTE_CL_CONVERTFULLYCONNECTEDWEIGHTS_H
#define ARM_COMPUTE_CL_CONVERTFULLYCONNECTEDWEIGHTS_H

#include "src/core/gpu/cl/ClCompileContext.h"
#include "src/runtime/gpu/cl/IClOperator.h"

namespace arm_compute
{
namespace opencl
{
/** Basic function to run @ref kernels::ClConvertFullyConnectedWeightsKernel */
class ClConvertFullyConnectedWeights : public IClOperator
{
public:
    /** Constructor */
    ClConvertFullyConnectedWeights() = default;
    /** Initialise the kernel's inputs and outputs
     *
     * @param[in] compile_context    The compile context to be used.
     * @param[in] src                The src tensor info. Data types supported: All.
     * @param[in] dst                The dst tensor info. Data types supported: Same as @p src
     * @param[in] original_src_shape Shape of the original src tensor (the one entering fully connected layer).
     * @param[in] data_layout        The data layout the weights have been trained in.
     */
    void configure(const CLCompileContext &compile_context, const ITensorInfo *src, ITensorInfo *dst, const TensorShape &original_src_shape, DataLayout data_layout);
    /**  Static function to check if given info will lead to a valid configuration of @ref kernels::ClConvertFullyConnectedWeightsKernel.
     *
     * @param[in] src                First tensor src info. Data types supported: All.
     * @param[in] dst                Output tensor info. Data types supported: same as @p src.
     * @param[in] original_src_shape Shape of the original src tensor (the one entering fully connected layer).
     * @param[in] data_layout        The data layout the weights have been trained in.
     *
     * @return a status
     */
    static Status validate(const ITensorInfo *src, const ITensorInfo *dst, const TensorShape &original_src_shape, DataLayout data_layout);
};
} // namespace opencl
} // namespace arm_compute
#endif /* ARM_COMPUTE_CL_CONVERTFULLYCONNECTEDWEIGHTS_H */
