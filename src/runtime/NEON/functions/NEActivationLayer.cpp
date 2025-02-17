/*
 * Copyright (c) 2017-2021, 2024 Arm Limited.
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
#include "arm_compute/runtime/NEON/functions/NEActivationLayer.h"

#include "arm_compute/core/Validate.h"

#include "src/cpu/operators/CpuActivation.h"

namespace arm_compute
{
struct NEActivationLayer::Impl
{
    const ITensor                      *src{nullptr};
    ITensor                            *dst{nullptr};
    IRuntimeContext                    *ctx{nullptr};
    std::unique_ptr<cpu::CpuActivation> op{nullptr};
};

NEActivationLayer::NEActivationLayer(IRuntimeContext *ctx) : _impl(std::make_unique<Impl>())
{
    _impl->ctx = ctx;
}
NEActivationLayer::NEActivationLayer(NEActivationLayer &&)            = default;
NEActivationLayer &NEActivationLayer::operator=(NEActivationLayer &&) = default;
NEActivationLayer::~NEActivationLayer()                               = default;

void NEActivationLayer::configure(ITensor *input, ITensor *output, ActivationLayerInfo activation_info)
{
    _impl->src = input;
    _impl->dst = output == nullptr ? input : output;

    ARM_COMPUTE_ERROR_ON_NULLPTR(_impl->src, _impl->dst);

    _impl->op = std::make_unique<cpu::CpuActivation>();
    _impl->op->configure(_impl->src->info(), _impl->dst->info(), activation_info);
}

Status
NEActivationLayer::validate(const ITensorInfo *input, const ITensorInfo *output, const ActivationLayerInfo &act_info)
{
    ARM_COMPUTE_RETURN_ERROR_ON_DYNAMIC_SHAPE(input, output);
    return cpu::CpuActivation::validate(input, output, act_info);
}

void NEActivationLayer::run()
{
    ITensorPack pack;
    pack.add_tensor(TensorType::ACL_SRC, _impl->src);
    pack.add_tensor(TensorType::ACL_DST, _impl->dst);
    _impl->op->run(pack);
}
} // namespace arm_compute
