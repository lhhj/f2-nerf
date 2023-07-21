#include "../../ros2/src/ros2-f2-nerf/src/localizer_core.hpp"
#include "../../src/Utils/Utils.h"

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem::v1;

int main()
{
  std::cout << "unit tool" << std::endl;
  const std::string runtime_config_path = "../../ros2/runtime_config_awsim.yaml";

  LocalizerCoreParam param{};
  param.is_awsim = true;
  LocalizerCore core(runtime_config_path, param);

  constexpr int32_t iteration_num = 1;

  std::ofstream score_ofs("score.tsv");
  const std::string save_dir = "./result_images/";
  fs::create_directories(save_dir);

  for (int32_t i = 0; i < core.dataset_->n_images_; i++) {
    torch::Tensor initial_pose = core.dataset_->poses_[i];
    torch::Tensor image_tensor = core.dataset_->image_tensors_[i];
    Utils::WriteImageTensor(save_dir + "image_01_gt.png", image_tensor);

    // Before noise
    auto [score_before, nerf_image_before] =
      core.pred_image_and_calc_score(initial_pose, image_tensor);
    std::cout << "score_before: " << score_before << std::endl;
    Utils::WriteImageTensor(save_dir + "image_02_before.png", nerf_image_before);

    // Added noise
    initial_pose[0][3] += 0.0125f;
    auto [score_noised, nerf_image_noised] =
      core.pred_image_and_calc_score(initial_pose, image_tensor);
    std::cout << "score_noised: " << score_noised << std::endl;
    Utils::WriteImageTensor(save_dir + "image_03_noised.png", nerf_image_noised);

    torch::Tensor before_optim = initial_pose.clone();

    // Optimized
    torch::Tensor optimized_pose = initial_pose;
    for (int32_t j = 0; j < 100; j++) {
      optimized_pose = core.optimize_pose(optimized_pose, image_tensor, iteration_num);
      auto [score_after, nerf_image_after] =
        core.pred_image_and_calc_score(optimized_pose, image_tensor);
      std::cout << "score_after " << std::setw(2) << j << " : " << score_after << std::endl;
      std::stringstream ss;
      ss << save_dir << "image_04_after_" << std::setfill('0') << std::setw(4) << j << ".png";
      Utils::WriteImageTensor(ss.str(), nerf_image_after);

      score_ofs << j << "\t" << score_after << std::endl;
    }

    torch::Tensor diff = optimized_pose - before_optim;
    std::cout << "diff\n" << diff << std::endl;

    break;
  }
}
