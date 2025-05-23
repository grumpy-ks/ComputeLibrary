/*
 * Copyright (c) 2018-2021, 2023-2024 Arm Limited.
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

#include "arm_compute/runtime/CL/functions/CLBatchToSpaceLayer.h"

#include "arm_compute/core/Error.h"
#include "arm_compute/core/TensorInfo.h"
#include "arm_compute/core/Types.h"
#include "arm_compute/core/Validate.h"
#include "arm_compute/runtime/CL/CLScheduler.h"

#include "src/common/utils/Log.h"
#include "src/core/CL/kernels/CLBatchToSpaceLayerKernel.h"

namespace arm_compute
{
CLBatchToSpaceLayer::CLBatchToSpaceLayer() : _batch_to_space_kernel(std::make_unique<CLBatchToSpaceLayerKernel>())
{
}

CLBatchToSpaceLayer::~CLBatchToSpaceLayer() = default;

void CLBatchToSpaceLayer::configure(const ICLTensor *input, const ICLTensor *block_shape, ICLTensor *output)
{
    ARM_COMPUTE_LOG_PARAMS(input, block_shape, output);
    _batch_to_space_kernel->configure(CLKernelLibrary::get().get_compile_context(), input, block_shape, output);
}

void CLBatchToSpaceLayer::configure(const CLCompileContext &compile_context,
                                    const ICLTensor        *input,
                                    const ICLTensor        *block_shape,
                                    ICLTensor              *output)
{
    ARM_COMPUTE_LOG_PARAMS(input, block_shape, output);
    _batch_to_space_kernel->configure(compile_context, input, block_shape, output);
}

void CLBatchToSpaceLayer::configure(
    const ICLTensor *input, int32_t block_shape_x, int32_t block_shape_y, ICLTensor *output, const CropInfo &crop_info)
{
    configure(CLKernelLibrary::get().get_compile_context(), input, block_shape_x, block_shape_y, output, crop_info);
}

void CLBatchToSpaceLayer::configure(const CLCompileContext &compile_context,
                                    const ICLTensor        *input,
                                    int32_t                 block_shape_x,
                                    int32_t                 block_shape_y,
                                    ICLTensor              *output,
                                    const CropInfo         &crop_info)
{
    ARM_COMPUTE_LOG_PARAMS(input, block_shape_x, block_shape_y, output);
    _batch_to_space_kernel->configure(compile_context, input, block_shape_x, block_shape_y, output, crop_info);
}

Status
CLBatchToSpaceLayer::validate(const ITensorInfo *input, const ITensorInfo *block_shape, const ITensorInfo *output)
{
    ARM_COMPUTE_RETURN_ERROR_ON_DYNAMIC_SHAPE(input, block_shape, output);
    return CLBatchToSpaceLayerKernel::validate(input, block_shape, output);
}

Status CLBatchToSpaceLayer::validate(const ITensorInfo *input,
                                     int32_t            block_shape_x,
                                     int32_t            block_shape_y,
                                     const ITensorInfo *output,
                                     const CropInfo    &crop_info)
{
    ARM_COMPUTE_RETURN_ERROR_ON_DYNAMIC_SHAPE(input, output);
    return CLBatchToSpaceLayerKernel::validate(input, block_shape_x, block_shape_y, output, crop_info);
}

void CLBatchToSpaceLayer::run()
{
    CLScheduler::get().enqueue(*_batch_to_space_kernel, true);
}
} // namespace arm_compute
