#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <algorithm>
#include <unordered_map>
#include <iomanip>

constexpr auto MAX_STATE_NO = 256;

const std::unordered_map<std::string, std::string> c2_c4_map = {
    {"00", "1000"},
    {"01", "0100"},
    {"10", "0010"},
    {"11", "0001"},
    {"0x", "1100"},
    {"x0", "1010"},
    {"1x", "0011"},
    {"x1", "0101"},
    {"xx", "1111"}
};

int main(int argc, char *argv[])
{
    if (argc < 2) {
        return -1;
    }

    std::vector<std::filesystem::path> src_file_paths;

    for (auto i = 1; i < argc; ++i) {
        if (!std::filesystem::exists(argv[i])) {
            std::cout << "source file dose not exists." << std::endl;
            return -1;
        }
        
        if (std::filesystem::is_regular_file(argv[i])) {
            auto path = std::filesystem::directory_entry(argv[i]).path();
            if (path.extension() != ".tcam") {
                std::cout << "source file must be a '.tcam' file." << std::endl;
                return -1;
            }

            src_file_paths.emplace_back(path);
        } else {
            for (const auto &entry : std::filesystem::directory_iterator(argv[i])) {
                if (entry.path().extension() != ".tcam") {
                    continue;
                }
                src_file_paths.emplace_back(entry.path());
            }
        }
    }

    if (src_file_paths.size() >= MAX_STATE_NO) {
        std::cout << "the number of tcam tables shall not exceed " << MAX_STATE_NO << std::endl;
        return -1;
    }

    std::vector<std::vector<std::string>> sram_chip_vec{20};

    std::size_t n = 0;
    for (const auto &path : src_file_paths) {
        std::ifstream ifstr(path.filename().c_str());
        if (!ifstr.is_open()) {
            std::cout << "cannot open source file: " << path.filename().c_str() << std::endl;
            return -1;
        }

        for (auto &sram_entry_vec : sram_chip_vec) {
            sram_entry_vec.emplace_back("");
            sram_entry_vec.emplace_back("");
            sram_entry_vec.emplace_back("");
            sram_entry_vec.emplace_back("");
        }

        std::string line;
        std::size_t sz = 0;
        while (getline(ifstr, line)) {
            const std::regex r(R"([01x]{40})");
            if (!regex_match(line, r)) {
                std::cout << "tcam entry shall only include 40 collums of [01x]." << std::endl;
                return -1;
            }

            std::vector<std::string> tcam_entry_vec;
            for (auto i = 0UL; i < line.size(); ++i, ++i) {
                tcam_entry_vec.emplace_back(line.substr(i, 2));
            }

            std::reverse(tcam_entry_vec.begin(), tcam_entry_vec.end());

            auto i = 0UL;
            for (const auto &c2 : tcam_entry_vec) {
                auto c4 = c2_c4_map.at(c2);
                sram_chip_vec[i][n*4+0].insert(0, 1, c4[0]);
                sram_chip_vec[i][n*4+1].insert(0, 1, c4[1]);
                sram_chip_vec[i][n*4+2].insert(0, 1, c4[2]);
                sram_chip_vec[i][n*4+3].insert(0, 1, c4[3]);
                ++i;
            }

            ++sz;
        }

        if (sz != 32) {
            std::cout << "a tcam table must be exactly 32 entries." << std::endl;
            return -1;
        }

        ++n;
    }

    auto dst_fname = std::string(src_file_paths.cbegin()->stem());
    std::ofstream ofstr;

    std::vector<std::uint32_t> u32_vec;
    ofstr.open(dst_fname + ".sram.txt");
    if (!ofstr.is_open()) {
        std::cout << "cannot open dest file: " << dst_fname + ".sram.txt" << std::endl;
        return -1;
    }
    auto i = 0UL;
    for (const auto &sram_entry_vec : sram_chip_vec) {
        ofstr << "//sram chip #" << std::setw(2) << std::setfill('0') << i << "\n";
        for (const auto &entry_str : sram_entry_vec) {
            ofstr << entry_str << "\n";
            u32_vec.emplace_back(std::stoul(entry_str, nullptr, 2));
            #ifdef DEBUG
            std::cout << entry_str << "\n";
            std::cout << std::stoul(entry_str, nullptr, 2) << "\n";
            #endif
        }
        ++i;
    }
    ofstr << std::flush;
    ofstr.close();

    ofstr.open(dst_fname + ".sram.dat", std::ios::binary);
    if (!ofstr.is_open()) {
        std::cout << "cannot open dest file: " << dst_fname + ".sram.dat" << std::endl;
        return -1;
    }
    ofstr.write(reinterpret_cast<const char*>(u32_vec.data()), sizeof(u32_vec[0]) * u32_vec.size());
    ofstr.close();

    return 0;
}