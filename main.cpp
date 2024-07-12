#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <algorithm>
#include <unordered_map>
#include <iomanip>

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
    if (argc != 2) {
        return -1;
    }

    if (!std::filesystem::exists(argv[1])) {
        std::cout << "source file dose not exists." << std::endl;
        return -1;
    }

    if (!std::filesystem::is_regular_file(argv[1])) {
        std::cout << "source must be a regular file." << std::endl;
        return -1;
    }

    if (std::filesystem::directory_entry(argv[1]).path().extension() != ".tcam") {
        std::cout << "source file must be a '.tcam' file." << std::endl;
        return -1;
    }

    auto fname = std::string(argv[1]);
    std::ifstream ifstr(fname);
    if (!ifstr.is_open()) {
        std::cout << "cannot open source file: " << fname << std::endl;
        return -1;
    }

    std::vector<std::vector<std::string>> sram_chip_vec{20};
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
            sram_chip_vec[i][0].insert(0, 1, c4[0]);
            sram_chip_vec[i][1].insert(0, 1, c4[1]);
            sram_chip_vec[i][2].insert(0, 1, c4[2]);
            sram_chip_vec[i][3].insert(0, 1, c4[3]);
            ++i;
        }

        ++sz;
    }

    if (sz != 32) {
        std::cout << "a tcam table must be exactly 32 entries." << std::endl;
        return -1;
    }

    auto dst_fname = std::string(std::filesystem::directory_entry(fname).path().stem());
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