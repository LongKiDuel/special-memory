#pragma once

#include <assimp/ProgressHandler.hpp>
#include <spdlog/spdlog.h>

class Progress_bar : public Assimp::ProgressHandler {
  bool Update(float ratio) override {
    if (ratio == -1) {
      SPDLOG_INFO("Prepare for loading model...");
    } else if (ratio < 0.5) {
      SPDLOG_INFO("Loading model from file: {}%", ratio * 100);
    } else {
      SPDLOG_INFO("Do post process on model: {}%", ratio * 100);
    }

    return true;
  }
};