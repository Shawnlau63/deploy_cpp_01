//
// Created by qinrui on 2019/12/2.
//

#ifndef TEST02_UTILS_HPP
#define TEST02_UTILS_HPP

#include <iostream>
#include <torch/script.h>
#include <vector>

using namespace std;

torch::Tensor iou(const torch::Tensor box, const torch::Tensor boxes) {
    auto box_x1 = box.select(0, 0);
    auto box_y1 = box.select(0, 1);
    auto box_x2 = box.select(0, 2);
    auto box_y2 = box.select(0, 3);

    auto boxes_x1 = boxes.select(1, 0);
    auto boxes_y1 = boxes.select(1, 1);
    auto boxes_x2 = boxes.select(1, 2);
    auto boxes_y2 = boxes.select(1, 3);

    auto box_area = (box_x2 - box_x1) * (box_y2 - box_y1);;
    auto boxes_area = (boxes_x2 - boxes_x1) * (boxes_y2 - boxes_y1);

    auto x1 = torch::max(box_x1, boxes_x1);
    auto y1 = torch::max(box_y1, boxes_y1);
    auto x2 = torch::min(box_x2, boxes_x2);
    auto y2 = torch::min(box_y2, boxes_y2);

    auto w = torch::clamp(x2 - x1, 0);
    auto h = torch::clamp(y2 - y1, 0);

    auto inter = w * h;

    auto v = inter / (box_area + boxes_area - inter);

    return v;
}

torch::Tensor nms(const torch::Tensor boxes, const float thresh = 0.3) {
    vector<torch::Tensor> keep_boxes;

    if (boxes.size(0) == 0) return torch::empty({0,0});

    auto args = boxes.select(1, 4).argsort(-1, true);
    auto sort_boxes = boxes.index_select(0, args);

    while (sort_boxes.size(0) > 0) {

        auto box = sort_boxes[0];
        keep_boxes.push_back(box);

        if (sort_boxes.size(0) > 1) {

            auto tmp_boxes = sort_boxes.slice(0, 1);

            auto u = iou(box, tmp_boxes);

            auto mask = u.le(thresh);
            auto idx = mask.nonzero().select(1, 0);
            sort_boxes = tmp_boxes.index_select(0, idx);
        } else {
            break;
        }
    }

    auto rst = torch::stack(keep_boxes);

    return rst;
}

torch::Tensor xywh2xyxy(torch::Tensor boxes) {
    auto cx = boxes.select(1, 0);
    auto cy = boxes.select(1, 1);
    auto w = boxes.select(1, 2);
    auto h = boxes.select(1, 3);
    auto cf = boxes.select(1, 4);

    auto half_w = w / 2;
    auto half_h = h / 2;

    auto x1 = cx - half_w;
    auto y1 = cy - half_h;
    auto x2 = cx + half_w;
    auto y2 = cy + half_h;

    auto rst = torch::stack({x1, y1, x2, y2, cf}, 1);

    return rst;
}



#endif //TEST02_UTILS_HPP
