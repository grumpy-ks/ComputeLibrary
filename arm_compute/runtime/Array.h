/*
 * Copyright (c) 2016-2021, 2025 Arm Limited.
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
#ifndef ACL_ARM_COMPUTE_RUNTIME_ARRAY_H
#define ACL_ARM_COMPUTE_RUNTIME_ARRAY_H

/** @file
 * @publicapi
 */

#include "arm_compute/core/IArray.h"
#include "arm_compute/core/Types.h"

#include <memory>

namespace arm_compute
{
/** Basic implementation of the IArray interface which allocates a static number of T values  */
template <class T>
class Array : public IArray<T>
{
public:
    /** Default constructor: empty array */
    Array() : IArray<T>(0), _values(nullptr)
    {
    }
    /** Constructor: initializes an array which can contain up to max_num_points values
     *
     * @param[in] max_num_values Maximum number of values the array will be able to stored
     */
    Array(size_t max_num_values) : IArray<T>(max_num_values), _values(std::make_unique<T[]>(max_num_values))
    {
    }

    // Inherited methods overridden:
    T *buffer() const override
    {
        return _values.get();
    }

private:
    std::unique_ptr<T[]> _values;
};

/** Array of uint8s. */
using UInt8Array = Array<uint8_t>;
/** Array of uint16s. */
using UInt16Array = Array<uint16_t>;
/** Array of uint32s. */
using UInt32Array = Array<uint32_t>;
/** Array of int16s. */
using Int16Array = Array<int16_t>;
/** Array of int32s. */
using Int32Array = Array<int32_t>;
/** Array of floats. */
using FloatArray = Array<float>;
} // namespace arm_compute
#endif // ACL_ARM_COMPUTE_RUNTIME_ARRAY_H
