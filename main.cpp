#include <iostream>
#include <fstream>
#include <algorithm>
#include "utility.h"

constexpr auto MAX_STATE_NO = 256;

int main(int argc, char *argv[])
{
    if (argc < 2) {
        return -1;
    }

    std::vector<std::filesystem::path> tcam_file_paths, aid_file_paths;

    for (auto i = 1; i < argc; ++i) {
        if (!std::filesystem::exists(argv[i])) {
            std::cout << "source file dose not exists." << std::endl;
            return -1;
        }

        if (std::filesystem::is_regular_file(argv[i])) {
            auto path = std::filesystem::directory_entry(argv[i]).path();
            if (path.extension() == ".tcam") {
                tcam_file_paths.emplace_back(path);
            } else if (path.extension() == ".aid") {
                aid_file_paths.emplace_back(path);
            } else {
                std::cout << "source file must be a '.tcam' or '.aid' file." << std::endl;
                return -1;
            }
        } else {
            for (const auto &entry : std::filesystem::recursive_directory_iterator(argv[i])) {
                if (entry.path().extension() == ".tcam") {
                    tcam_file_paths.emplace_back(entry.path());
                } else if (entry.path().extension() == ".aid") {
                    aid_file_paths.emplace_back(entry.path());
                }
            }
        }
    }

    if (tcam_file_paths.size() >= MAX_STATE_NO || aid_file_paths.size() >= MAX_STATE_NO) {
        std::cout << "the number of tcam or action_id tables shall not exceed " << MAX_STATE_NO << std::endl;
        return -1;
    }

    std::sort(tcam_file_paths.begin(), tcam_file_paths.end());
    std::sort(aid_file_paths.begin(), aid_file_paths.end());

    if (tcam_file_paths.size() != aid_file_paths.size()) {
        std::cout << "the number of '.tcam' one '.aid' does not match." << std::endl;
        return -1;
    }

    for (auto i = 0UL; i < tcam_file_paths.size(); ++i) {
        if (tcam_file_paths[i].stem() != aid_file_paths[i].stem()) {
            std::cout << "the name of '.tcam' and '.aid' does not match." << std::endl;
            return -1;
        }
    }

    if (auto rc = convert_tcam_2_sram(tcam_file_paths)) {
        return rc;
    }

    if (auto rc = extract_action_ids(aid_file_paths)) {
        return rc;
    }

    return 0;
}