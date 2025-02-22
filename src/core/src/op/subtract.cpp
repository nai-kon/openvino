// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "openvino/op/subtract.hpp"

#include "element_visitor.hpp"
#include "itt.hpp"
#include "openvino/reference/subtract.hpp"
#include "utils.hpp"

namespace ov {
namespace op {
namespace subtract {
struct Evaluate : element::NoAction<bool> {
    using element::NoAction<bool>::visit;

    template <element::Type_t ET>
    static result_type visit(const Tensor& in0,
                             const Tensor& in1,
                             Tensor& out,
                             const AutoBroadcastSpec& broadcast_spec) {
        using T = typename element_type_traits<ET>::value_type;
        reference::subtract(in0.data<const T>(),
                            in1.data<const T>(),
                            out.data<T>(),
                            in0.get_shape(),
                            in1.get_shape(),
                            broadcast_spec);
        return true;
    }
};
}  // namespace subtract

// ------------------------------- v1 ------------------------------------------
namespace v1 {
Subtract::Subtract(const Output<Node>& arg0, const Output<Node>& arg1, const AutoBroadcastSpec& auto_broadcast)
    : BinaryElementwiseArithmetic(arg0, arg1, auto_broadcast) {
    constructor_validate_and_infer_types();
}

std::shared_ptr<Node> Subtract::clone_with_new_inputs(const OutputVector& new_args) const {
    OV_OP_SCOPE(v1_Subtract_clone_with_new_inputs);
    check_new_args_count(this, new_args);
    return std::make_shared<Subtract>(new_args.at(0), new_args.at(1), get_autob());
}

bool Subtract::evaluate(TensorVector& outputs, const TensorVector& inputs) const {
    OV_OP_SCOPE(v1_Subtract_evaluate);
    OPENVINO_ASSERT(outputs.size() == 1);
    OPENVINO_ASSERT(inputs.size() == 2);

    outputs[0].set_shape(infer_broadcast_shape(this, inputs[0].get_shape(), inputs[1].get_shape()));
    using namespace ov::element;
    return IfTypeOf<bf16, f16, f32, i8, i32, i64, u8, u32, u64>::apply<subtract::Evaluate>(inputs[0].get_element_type(),
                                                                                           inputs[0],
                                                                                           inputs[1],
                                                                                           outputs[0],
                                                                                           get_autob());
}

bool Subtract::has_evaluate() const {
    OV_OP_SCOPE(v1_Subtract_has_evaluate);
    switch (get_input_element_type(0)) {
    case element::bf16:
    case element::f16:
    case element::f32:
    case element::i8:
    case element::i32:
    case element::i64:
    case element::u8:
    case element::u32:
    case element::u64:
        return true;
    default:
        return false;
    }
}
}  // namespace v1
}  // namespace op
}  // namespace ov
