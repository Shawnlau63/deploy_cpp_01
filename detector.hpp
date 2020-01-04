//
// Created by qinrui on 2019/12/2.
//

#ifndef TEST02_DETECTOR_HPP
#define TEST02_DETECTOR_HPP

#include <iostream>
#include <torch/script.h>
#include <vector>

#include "utils.hpp"

using namespace std;


struct Detector {

    const float const_anchors[3][3][2] = {
            {{58, 45}, {78, 99}, {120, 160}},
            {{15, 30}, {31, 22}, {30,  40}},
            {{5,  7},  {8,  15}, {16,  11}},
    };

    const float const_grid_sizes[3] = {32, 16, 8};

    const float confidence_threshold = 0.6;

    const float nms_union_threshold = 0.3;

    torch::jit::script::Module module;

    Detector(string mfile) {
        module = torch::jit::load(mfile);
        module.to(torch::kCUDA);
    }

    torch::Tensor forward(const torch::Tensor &image) {
        auto tensor_image = preview(image);

        std::vector<torch::jit::IValue> inputs;
        inputs.push_back(tensor_image.to(torch::kCUDA));
        auto vs = module.forward(inputs).toTuple()->elements();

        auto output_0 = vs[0].toTensor().to(torch::kCPU);
        auto output_1 = vs[1].toTensor().to(torch::kCPU);
        auto output_2 = vs[2].toTensor().to(torch::kCPU);

        auto boxes_0 = parse(output_0, const_grid_sizes[0], const_anchors[0]);
        auto boxes_1 = parse(output_1, const_grid_sizes[1], const_anchors[1]);
        auto boxes_2 = parse(output_2, const_grid_sizes[2], const_anchors[2]);

        auto boxes = torch::cat({boxes_0, boxes_1, boxes_2}, 0);

        auto rst = ::nms(::xywh2xyxy(boxes), nms_union_threshold);

        return rst;
    }

private:
    torch::Tensor preview(const torch::Tensor &image) {
        auto tensor_image = image.toType(torch::kFloat).permute({0, 3, 1, 2}).div(255);
        return tensor_image;
    }

    torch::Tensor parse(torch::Tensor &output, const float grid_size, const float anchors[3][2]) {
        vector<torch::Tensor> vs;
        auto output_anchors = output.reshape({output.size(0), 3, -1, output.size(2), output.size(3)});
        for (int i = 0; i < 3; i++) {
            auto output_anchor = output_anchors.select(1, i);
            vs.push_back(filter(output_anchor, grid_size, anchors[i][0], anchors[i][1]));
        }
        auto rst = torch::cat(vs, 0);
        return rst;
    }

    torch::Tensor filter(const torch::Tensor &feature, int grid_size, int anchor_w, int anchor_h) {

        auto feature_cf = torch::sigmoid(feature.select(1, 4));
        auto mask = feature_cf.gt(confidence_threshold);

        auto cf = feature_cf.masked_select(mask);
        auto ox = torch::sigmoid(feature.select(1, 0).masked_select(mask));
        auto oy = torch::sigmoid(feature.select(1, 1).masked_select(mask));
        auto pw = feature.select(1, 2).masked_select(mask);
        auto ph = feature.select(1, 3).masked_select(mask);


        auto idxs = mask.nonzero();

        auto idxs_h = idxs.select(1, 1);
        auto idxs_w = idxs.select(1, 2);

        auto cx = (idxs_w + ox) * grid_size;
        auto cy = (idxs_h + oy) * grid_size;
        auto w = anchor_w * pw.exp();
        auto h = anchor_h * ph.exp();

        auto rst = torch::stack({cx, cy, w, h, cf}, 1);

        return rst;
    }
};

#endif //TEST02_DETECTOR_HPP
