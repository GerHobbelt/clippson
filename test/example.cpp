#include <clippson/clippson.hpp>

#include <limits>
#include <iostream>

struct Parameters {
    bool BOOL = false;
    int INT = std::numeric_limits<int>::max();
    long LONG = std::numeric_limits<long>::max();
    unsigned UNSIGNED = std::numeric_limits<unsigned>::max();
    size_t SIZE_T = std::numeric_limits<size_t>::max();
    double DOUBLE = 0.0;
    std::string STRING = "Hello, world!";
    std::vector<int> VECTOR = {0, 1};

    clipp::group cli(nlohmann::json* vm) {
        return (
          wtl::option(vm, {"b", "bool"}, &BOOL),
          wtl::option(vm, {"i", "int"}, &INT),
          wtl::option(vm, {"l", "long"}, &LONG),
          wtl::option(vm, {"u", "unsigned"}, &UNSIGNED),
          wtl::option(vm, {"s", "size_t"}, &SIZE_T),
          wtl::option(vm, {"d", "double"}, &DOUBLE),
          wtl::option(vm, {"c", "string"}, &STRING),
          wtl::option(vm, {"v", "vector"}, &VECTOR)
        ).doc("Notified to both json and targets:");
    }
};

int main(int argc, char* argv[]) {
    bool help = false;
    int answer = 42;
    auto to_targets = (
      wtl::option({"h", "help"}, &help, "Print help"),
      wtl::option({"a", "answer"}, &answer, "Answer")
    ).doc("Notified to targets:");

    nlohmann::json vm;
    auto to_json = (
      wtl::option(&vm, {"version"}, false, "Print version"),
      wtl::option(&vm, {"whoami"}, "24601"),
      wtl::option(&vm, {"year", "y"}, 2112)
    ).doc("Notified to json:");

    Parameters params;
    auto to_json_and_targets = params.cli(&vm);

    auto positional = (
      wtl::value<int>(&vm, "nsam", "Number of samples"),
      wtl::value<std::string>(&vm, "howmany", "tears")
    ).doc("Positional (required):");

    auto cli = clipp::joinable(
      // positional,
      to_targets,
      to_json,
      to_json_and_targets
    );
    std::string default_values = vm.dump(2);
    auto fmt = wtl::doc_format();
    wtl::parse(cli, argc, argv);
    if (help) {
        std::cout << clipp::documentation(cli, fmt) << "\n";
        return 0;
    }
    if (vm["version"]) {
        std::cout << "clipp 1.2.0\n";
        return 0;
    }
    std::cout << "Default values: " << default_values << "\n";
    std::cout << "Current values: " << vm.dump(2) << "\n";
    auto args = wtl::arg_list(vm);
    std::cout << argv[0];
    for (const auto& x: args) {std::cout << " {" << x << "}";}
    std::cout << "\n";
    if (vm.find("--") != vm.end()) vm["--"].clear();
    wtl::parse(cli, args);
    std::cout << "Round trip: " << vm.dump(2) << "\n";
    std::cout << "Answer: " << answer << "\n";
    return 0;
}
