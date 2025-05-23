/*
 * Copyright (c) 2023, 2025 Arm Limited.
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

#ifndef ACL_SRC_CORE_NEON_KERNELS_NEREORDERKERNEL_H
#define ACL_SRC_CORE_NEON_KERNELS_NEREORDERKERNEL_H

#if defined(__aarch64__)

#include "arm_compute/core/Types.h"

#include "src/core/NEON/INEKernel.h"

namespace arm_compute
{

/** Interface kernel to reorder tensor into blocked format. */
class NEReorderKernel : public INEKernel
{
public:
    const char *name() const override
    {
        return "NEReorderKernel";
    }

    /** Default constructor */
    NEReorderKernel();
    /** Prevent instances of this class from being copied (As this class contains pointers) */
    NEReorderKernel(const NEReorderKernel &) = delete;
    /** Prevent instances of this class from being copied (As this class contains pointers) */
    NEReorderKernel &operator=(const NEReorderKernel &) = delete;
    /** Allow instances of this class to be moved */
    NEReorderKernel(NEReorderKernel &&) = default;
    /** Allow instances of this class to be moved */
    NEReorderKernel &operator=(NEReorderKernel &&) = default;
    /** Default destructor */
    ~NEReorderKernel() = default;

    /** Initialise the kernel's input and outputs.
     *
     * @param[in]  input     Source tensor with 2 or 4 dimensions. Data types supported: F32.
     * @param[out] output    Destination tensor. Data type supported: same as @p input. Shape same as @p input expect last dimension which needs to be multiple of blocking parameter _ksize.
     * @param[in]  input_wf  WeightFormat of input.
     * @param[in]  output_wf WeightFormat of output.
     * @param[in]  transpose Whether to transpose or not.
     */
    void configure(const ITensor            *input,
                   ITensor                  *output,
                   arm_compute::WeightFormat input_wf,
                   arm_compute::WeightFormat output_wf,
                   bool                      transpose);

    /** Static function to check if given info will lead to a valid configuration of @ref NEReorderKernel
     *
     * @param[in] input     Source tensor with 2 or 4 dimensions. Data types supported: F32.
     * @param[in] output    Destination tensor. Data type supported: same as @p input. Shape same as @p input expect last dimension which needs to be multiple of blocking parameter _ksize.
     * @param[in] input_wf  WeightFormat of input.
     * @param[in] output_wf WeightFormat of output.
     * @param[in] transpose Whether to transpose or not.
     *
     * @return a status
     */
    static Status validate(const ITensorInfo        *input,
                           const ITensorInfo        *output,
                           arm_compute::WeightFormat input_wf,
                           arm_compute::WeightFormat output_wf,
                           bool                      transpose);

    // Inherited methods overridden:
    void run(const Window &window, const ThreadInfo &info) override;

    /*****************************************************************************/

private:
    const ITensor *_input{nullptr};  // Input tensor
    ITensor       *_output{nullptr}; // Output tensor
    int32_t        _ksize{0};        // Blocking parameter, how many rows kernel reorders on each call
    int32_t        _kmax{0};         // Rows in input tensor
    int32_t        _xmax{0};         // Columns in input tensor
    WeightFormat   _input_wf{WeightFormat::UNSPECIFIED};  // WeightFormat of input tensor
    WeightFormat   _output_wf{WeightFormat::UNSPECIFIED}; // WeightFormat of output tensor
    bool           _transpose = false;                    // Whether to transpose or not
};

} // namespace arm_compute

#endif // defined(__aarch64__)

#endif // ACL_SRC_CORE_NEON_KERNELS_NEREORDERKERNEL_H
