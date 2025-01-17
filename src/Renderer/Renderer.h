//
// Created by ppwang on 2022/5/7.
//

#ifndef SANR_RENDERER_H
#define SANR_RENDERER_H

#pragma once
#include <vector>
#include <memory>
#include <yaml-cpp/yaml.h>
#include "../Utils/Pipe.h"
#include "../Field/Field.h"
#include "../Shader/Shader.h"
#include "../PtsSampler/PtsSampler.h"

struct RenderResult {
  using Tensor = torch::Tensor;
  Tensor colors;
  Tensor disparity;
  Tensor depth;
  Tensor weights;
  Tensor idx_start_end;
};

class Renderer : public Pipe {
  using Tensor = torch::Tensor;

public:
  Renderer(const YAML::Node & config, int n_images);
  RenderResult Render(const Tensor& rays_o, const Tensor& rays_d, const Tensor& emb_idx, RunningMode mode);

  int LoadStates(const std::vector<Tensor>& states, int idx) override;
  std::vector<Tensor> States() override ;
  std::vector<torch::optim::OptimizerParamGroup> OptimParamGroups(float lr) override;

  const YAML::Node config_;
  std::unique_ptr<PtsSampler> pts_sampler_;
  std::unique_ptr<Field> scene_field_;
  std::unique_ptr<Shader> shader_;

  bool use_app_emb_;
  Tensor app_emb_;
};

#endif //SANR_RENDERER_H
