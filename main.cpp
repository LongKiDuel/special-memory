#include <CL/cl.h>
#include <CL/opencl.hpp>
#include <algorithm>
#include <format>
#include <iostream>
#include <source_location>
#include <string>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include <fstream>
#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>
#include <streambuf>
// Define your lower case namespace

void pattern_replace(std::string &src, std::string pattern, std::string value) {
  {
    auto pos = src.find(pattern);
    if (pos == std::string::npos) {
      return;
    }
    src.replace(pos, pattern.size(), value);
  }
}
std::string cl_error_info_str(cl_int error) {
  if (error != CL_SUCCESS) {
    switch (error) {
    case CL_DEVICE_NOT_FOUND:
      return "Device not found";
      break;
    case CL_DEVICE_NOT_AVAILABLE:
      return "Device not available";
      break;
    case CL_COMPILER_NOT_AVAILABLE:
      return "Compiler not available";
      break;
    case CL_MEM_OBJECT_ALLOCATION_FAILURE:
      return "Memory object allocation failure";
      break;
    // Add more cases for other error codes as needed
    default:
      return "unknown";
      break;
    }
  }
}
void check_err(cl_int error_number, std::source_location location =
                                        std::source_location::current()) {
  if (error_number != CL_SUCCESS) {
    SPDLOG_ERROR("opencl error: {} {} in [{}:{}]", error_number,
                 cl_error_info_str(error_number), location.file_name(),
                 location.line());
  }
}
namespace image_processing {

// Define your 'My_class' for image processing
class ImageBlur {
  const int channels = 1;

public:
  // Constructor
  ImageBlur(const char *imageFilePath, const char *kernelFilePath, int radius)
      : imageFilePath_(imageFilePath), kernelFilePath_(kernelFilePath),
        radius_(radius) {
    // Load the image using stb_image
    SPDLOG_INFO("start parse image");
    imageData_ = stbi_load(imageFilePath, &width_, &height_, nullptr, channels);
    if (!imageData_) {
      std::cerr << "Failed to load image: " << imageFilePath << std::endl;
      return;
    }
    SPDLOG_INFO("image loaded into memory");

    // Read the OpenCL kernel source code from the file
    std::ifstream kernelFile(kernelFilePath);
    if (!kernelFile.is_open()) {
      std::cerr << "Failed to open kernel file: " << kernelFilePath
                << std::endl;
      return;
    }

    kernelSource_ = std::string((std::istreambuf_iterator<char>(kernelFile)),
                                std::istreambuf_iterator<char>());

    pattern_replace(kernelSource_, "@radius@", std::to_string(radius));

    // Initialize OpenCL
    cl::Platform::get(&platforms_);
    if (platforms_.empty()) {
      std::cerr << "No OpenCL platforms found." << std::endl;
      return;
    }
    for (auto plat : platforms_) {

      // Choose the first platform
      platform_ = plat;

      // Get the devices for the chosen platform
      platform_.getDevices(CL_DEVICE_TYPE_GPU, &devices_);
      if (devices_.empty()) {
        continue;
      } else {
        break;
      }
    }

    // Choose the first device
    device_ = devices_[0];
    SPDLOG_INFO("using device: {}", device_.getInfo<CL_DEVICE_NAME>());

    // Create an OpenCL context
    context_ = cl::Context(device_);

    // Create a command queue
    queue_ = cl::CommandQueue(context_, device_);

    // Load and compile the OpenCL program
    cl::Program::Sources sources;
    sources.push_back({kernelSource_.c_str(), kernelSource_.length()});
    program_ = cl::Program(context_, sources);
    if (program_.build({device_}) != CL_SUCCESS) {
      std::cerr << "Error building OpenCL program." << std::endl;
      SPDLOG_ERROR("failed to build code: ");
      for (auto &[d, i] : program_.getBuildInfo<CL_PROGRAM_BUILD_LOG>()) {
        std::cerr << i << "\n";
      }
      exit(1);
      return;
    }

    // Create buffers for the image data
    inputImageBuffer_ =
        cl::Buffer(context_, CL_MEM_READ_ONLY,
                   width_ * height_ * channels * sizeof(unsigned char));
    outputImageBuffer_ =
        cl::Buffer(context_, CL_MEM_WRITE_ONLY,
                   width_ * height_ * channels * sizeof(unsigned char));
  }

  // Member function to apply blur to the loaded image
  void ApplyBlur() {
    // Transfer image data to the input buffer
    SPDLOG_INFO("upload image");
    queue_.enqueueWriteBuffer(
        inputImageBuffer_, CL_TRUE, 0,
        width_ * height_ * sizeof(unsigned char) * channels, imageData_);

    SPDLOG_INFO("start blur calculation.");
    spdlog::stopwatch sw;
    // Create a kernel and set its arguments
    cl::Kernel kernel(program_, "blur");
    kernel.setArg(0, inputImageBuffer_);
    kernel.setArg(1, outputImageBuffer_);
    kernel.setArg(2, width_);
    kernel.setArg(3, height_);

    // Execute the kernel
    cl::NDRange global(width_, height_);

    // kerenl runtime longer than 600ms will be killed on intel.
    queue_.enqueueNDRangeKernel(kernel, cl::NullRange, global);

    auto err = queue_.finish();
    check_err(err);
    SPDLOG_INFO("blur {} px finished, time: {} s", radius_, sw);
    SPDLOG_INFO("start download iamge");
    // Read back the blurred image
    queue_.enqueueReadBuffer(
        outputImageBuffer_, CL_TRUE, 0,
        width_ * height_ * sizeof(unsigned char) * channels, imageData_);
    queue_.finish();
    SPDLOG_INFO("finish download");
  }

  // Member function to save the blurred image to a file
  void SaveBlurredImage(const char *outputFilePath) {
    SPDLOG_INFO("start saving file");
    stbi_write_png(outputFilePath, width_, height_, channels, imageData_,
                   width_ * channels);
    SPDLOG_INFO("file saved");
  }

  // Destructor to free image data
  ~ImageBlur() {
    if (imageData_) {
      stbi_image_free(imageData_);
    }
  }

private:
  const char *imageFilePath_;
  const char *kernelFilePath_;
  unsigned char *imageData_ = nullptr;
  int width_;
  int height_;
  int radius_;
  cl::Platform platform_;
  std::vector<cl::Platform> platforms_;
  cl::Device device_;
  std::vector<cl::Device> devices_;
  cl::Context context_;
  cl::CommandQueue queue_;
  cl::Program program_;
  cl::Buffer inputImageBuffer_;
  cl::Buffer outputImageBuffer_;
  std::string kernelSource_;
};

} // namespace image_processing

int main(int argc, char **argv) {
  // Image file path
  const char *imageFilePath =
      argc == 1 ? "a.png" : argv[1]; // Replace with your image file path

  // Kernel file path
  const char *kernelFilePath = "image.cl"; // Replace with your kernel file path

  spdlog::set_pattern("[+ %5o ms] %v");
  // Create an instance of your 'My_class' and load the image and kernel
  for (int radius : {1, 3, 5, 10, 20, 40}) {

    image_processing::ImageBlur imageBlur(imageFilePath, kernelFilePath,
                                          radius);

    // Apply the blur
    imageBlur.ApplyBlur();

    // Save the blurred image to a file
    std::string outputFilePath = std::format(
        "blurred_image_{}.png", radius); // Specify the output file path
    imageBlur.SaveBlurredImage(outputFilePath.c_str());

    SPDLOG_INFO("Blurred image saved to: {}", outputFilePath);
  }

  return 0;
}
