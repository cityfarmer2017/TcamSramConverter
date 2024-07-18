/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#include <iostream>
#include <algorithm>
#include <filesystem>
#include "match_action_id.h"  // NOLINT [build/include_subdir]

constexpr auto MAX_STATE_NO = 256;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return -1;
    }

    std::map<std::string, std::string> file_stem_path_map;

    for (auto i = 1; i < argc; ++i) {
        if (!std::filesystem::exists(argv[i])) {
            std::cout << "source file does not exist." << std::endl;
            return -1;
        }

        if (std::filesystem::is_regular_file(argv[i])) {
            auto path = std::filesystem::path(argv[i]);
            if (path.extension() == ".tcam") {
                auto fstem = path.stem().string();
                auto ppath = path.has_parent_path() ? path.parent_path() : std::filesystem::current_path();
                if (!std::filesystem::exists(ppath.string() + "/" + fstem + ".aid")) {
                    std::cout << path << " corresponding '.aid' file does not exist." << std::endl;
                    return -1;
                }
                file_stem_path_map[fstem] = ppath;
            } else {
                std::cout << "source file must be a '.tcam' file." << std::endl;
                return -1;
            }
        } else {
            for (const auto &entry : std::filesystem::recursive_directory_iterator(argv[i])) {
                if (entry.path().extension() == ".tcam") {
                    auto fstem = entry.path().stem().string();
                    if (!std::filesystem::exists(entry.path().parent_path().string() + "/" + fstem + ".aid")) {
                        std::cout << entry.path() << " corresponding '.aid' file does not exist." << std::endl;
                        return -1;
                    }
                    file_stem_path_map[fstem] = entry.path().parent_path();
                }
            }
        }
    }

    if (file_stem_path_map.size() >= MAX_STATE_NO) {
        std::cout << "the number of tcam files shall not exceed " << MAX_STATE_NO << std::endl;
        return -1;
    }

    if (auto rc = convert_tcam_2_sram(file_stem_path_map)) {
        return rc;
    }

    if (auto rc = extract_action_ids(file_stem_path_map)) {
        return rc;
    }

    return 0;
}
