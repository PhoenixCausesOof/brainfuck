#include <boost/program_options.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>

#include "compiler.hpp"

int main(int argc, char **argv) {
  std::ios::sync_with_stdio(false);
  std::cin.tie();

  namespace po = boost::program_options;
  namespace fs = std::filesystem;

  po::options_description optionsDesc("options");
  optionsDesc.add_options()("help,h", "display this help message")(
      "source,s", po::value<fs::path>(), "source file to compile")(
      "size", po::value<std::uint32_t>()->default_value(30000),
      "size of stack")(
      "output,o", po::value<fs::path>(),
      "describes a path to the log file (if omitted, falls back to stdout)");

  po::positional_options_description posDesc;
  posDesc.add("source", 1);

  po::variables_map variableMap;
  po::store(po::command_line_parser(argc, argv)
                .options(optionsDesc)
                .positional(posDesc)
                .run(),
            variableMap);
  po::notify(variableMap);

  if (variableMap.contains("help")) {
    std::cout << "usage: " << argv[0] << " [options]\n";
    std::cout << optionsDesc << '\n';
    return EXIT_SUCCESS;
  }

  if (!variableMap.contains("source"))
    throw po::error("source file missing");

  std::ifstream file(variableMap["source"].as<fs::path>());

  if (!file.is_open())
    throw std::runtime_error("source file doesn't exist");


  compiler::Compiler ctx(variableMap["size"].as<std::uint32_t>());

  using namespace std::chrono;
  const auto compBegin = high_resolution_clock::now();
  asmjit::Error result = ctx.compile<>({file}, {});
  const auto compEnd = high_resolution_clock::now();

  if (result != asmjit::kErrorOk) {
    std::cerr << asmjit::DebugUtils::errorAsString(result) << '\n';
    return EXIT_FAILURE;
  }

  if (variableMap.contains("output")) {
    auto outputPath = variableMap["output"].as<fs::path>();

    if (outputPath.has_parent_path())
      if (!fs::exists(outputPath.parent_path())) {
        std::cerr << "output path's parent directory doesn't exist\n";
        return EXIT_FAILURE;
      }

    std::ofstream out(outputPath, std::ios::trunc);

    out << ctx.logs() << std::endl;
  } else {
    std::cout << ctx.logs() << std::endl;
  }

  const auto execBegin = high_resolution_clock::now();
  ctx.run();
  const auto execEnd = high_resolution_clock::now();

  std::cout << "compiled in: "
            << duration_cast<microseconds>(compEnd -
                                                                    compBegin)
            << '\n';
  std::cout << "executed in: "
            << duration_cast<
                   duration<float, std::ratio<1>>>(execEnd - execBegin)
            << '\n';
}
