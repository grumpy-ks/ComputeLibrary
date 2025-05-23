/*
 * Copyright (c) 2017-2025 Arm Limited.
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
#ifndef ACL_TESTS_DATASETS_SMALLGEMMDATASET_H
#define ACL_TESTS_DATASETS_SMALLGEMMDATASET_H

#include "arm_compute/core/TensorShape.h"

#include "tests/datasets/GEMMDataset.h"

namespace arm_compute
{
namespace test
{
namespace datasets
{
class SmallGEMMDataset final : public GEMMDataset
{
public:
    SmallGEMMDataset()
    {
        add_config(1, 17, 32, 0.4f, 0.7f);
        add_config(1, 23, 31, 1.0f, 0.0f);
        add_config(1, 23, 31, 1.0f, 1.0f);
        add_config(2, 16, 8, 1.0f, 0.0f);
        add_config(12, 21, 38, 0.2f, 1.2f);
        add_config(13, 33, 21, 1.0f, 0.0f);
    }
};

class SmallGEMMOutput3DDataset final : public GEMMDataset
{
public:
    SmallGEMMOutput3DDataset()
    {
        add_config(TensorShape(16U, 16U), TensorShape(8U, 16U), TensorShape(8U), TensorShape(8U, 8U, 2U), 1.0f, 0.0f);
        add_config(TensorShape(16U, 16U, 5U), TensorShape(8U, 16U, 5U), TensorShape(8U), TensorShape(8U, 8U, 2U, 5U), 1.0f, 0.0f);
        add_config(TensorShape(21U, 14U), TensorShape(34U, 21U), TensorShape(34U), TensorShape(34U, 7U, 2U), 1.0f, 0.0f);
        add_config(TensorShape(31U, 1U), TensorShape(23U, 31U), TensorShape(23U), TensorShape(23U, 1U, 1U), 1.0f, 0.0f);
        add_config(TensorShape(32U, 1U), TensorShape(17U, 32U), TensorShape(17U), TensorShape(17U, 1U, 1U), 0.4f, 0.7f);
        add_config(TensorShape(38U, 12U), TensorShape(21U, 38U), TensorShape(21U), TensorShape(21U, 4U, 3U), 0.2f, 1.2f);
    }
};

class SmallGEMMInputOutput3DDataset final : public GEMMDataset
{
public:
    SmallGEMMInputOutput3DDataset()
    {
        add_config(TensorShape(16U, 16U, 3U, 2U), TensorShape(8U, 16U), TensorShape(8U), TensorShape(8U, 16U, 3U, 2U), 1.0f, 0.0f);
        add_config(TensorShape(16U, 16U, 5U, 3U), TensorShape(8U, 16U), TensorShape(8U), TensorShape(8U, 16U, 5U, 3U), 1.0f, 0.3f);
        add_config(TensorShape(21U, 14U, 13U), TensorShape(34U, 21U), TensorShape(34U), TensorShape(34U, 14U, 13U), 1.0f, 0.0f);
        add_config(TensorShape(31U, 1U, 3U), TensorShape(23U, 31U), TensorShape(23U), TensorShape(23U, 1U, 3U), 1.0f, 0.0f);
        add_config(TensorShape(32U, 1U, 4U, 3U), TensorShape(17U, 32U), TensorShape(17U), TensorShape(17U, 1U, 4U, 3U), 0.4f, 0.7f);
        add_config(TensorShape(38U, 12U, 2U), TensorShape(21U, 38U), TensorShape(21U), TensorShape(21U, 12U, 2U), 0.2f, 1.2f);
    }
};

class SmallBatchedMatMulDataset final : public GEMMDataset
{
public:
    SmallBatchedMatMulDataset()
    {
        add_config(TensorShape(2U, 4U, 3U), TensorShape(5U, 2U, 3U), TensorShape(5U), TensorShape(5U, 4U, 3U), 1.0f, 0.0f);
        add_config(TensorShape(4U, 3U), TensorShape(2U, 4U), TensorShape(2U), TensorShape(2U, 3U), 1.0f, 0.0f);
        add_config(TensorShape(12U, 15U), TensorShape(7U, 12U), TensorShape(7U), TensorShape(7U, 15U), 1.0f, 0.0f);
        add_config(TensorShape(13U, 256U, 32U), TensorShape(19U, 13U, 32U), TensorShape(19U), TensorShape(19U, 256U, 32U), 1.0f, 0.0f);
        add_config(TensorShape(15U, 7U, 36U), TensorShape(29U, 15U, 36U), TensorShape(29U), TensorShape(29U, 7U, 36U), 1.0f, 0.0f);
        add_config(TensorShape(56U, 17U, 32U), TensorShape(5U, 56U, 32U), TensorShape(5U), TensorShape(5U, 17U, 32U), 1.0f, 0.0f);
        add_config(TensorShape(59U, 17U), TensorShape(36U, 59U), TensorShape(36U), TensorShape(36U, 17U), 1.0f, 0.0f);
        // Broadcast in RHS's batch dimension
        add_config(TensorShape(15U, 7U, 36U), TensorShape(29U, 15U), TensorShape(29U), TensorShape(29U, 7U, 36U), 1.0f, 0.0f);
        add_config(TensorShape(15U, 7U, 36U, 2U), TensorShape(29U, 15U), TensorShape(29U), TensorShape(29U, 7U, 36U, 2U), 1.0f, 0.0f);
    }
};

class SmallAccumulateGEMMDataset final : public GEMMDataset
{
public:
    SmallAccumulateGEMMDataset()
    {
        add_config(1, 23, 31, 1.0f, 0.0f);
        add_config(2, 16, 8, 1.0f, 0.0f);
        add_config(13, 33, 21, 1.0f, 0.0f);
    }
};

class SmallGEMMVectorBiasDataset final : public GEMMDataset
{
public:
    SmallGEMMVectorBiasDataset()
    {
        add_config(TensorShape(1U), TensorShape(1U), TensorShape(1U), TensorShape(1U), 1.0f, 1.0f); // scalar * scalar
        add_config(TensorShape(1U, 7U), TensorShape(5U), TensorShape(5U), TensorShape(5U, 7U), 1.0f, 1.0f); // column * row
        add_config(TensorShape(7U), TensorShape(1U, 7U), TensorShape(1U), TensorShape(1U), 1.0f, 1.0f); // row * column
        add_config(TensorShape(21U, 13U), TensorShape(33U, 21U), TensorShape(33U), TensorShape(33U, 13U), 1.0f, 1.0f); // matrix * matrix
        add_config(TensorShape(23U, 31U), TensorShape(1U, 23U), TensorShape(1U), TensorShape(1U, 31U), 1.0f, 1.0f); // matrix * column
        add_config(TensorShape(31U), TensorShape(23U, 31U), TensorShape(23U), TensorShape(23U), 1.0f, 1.0f); // row * matrix
    }
};

} // namespace datasets
} // namespace test
} // namespace arm_compute
#endif // ACL_TESTS_DATASETS_SMALLGEMMDATASET_H
