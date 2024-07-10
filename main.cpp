#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <algorithm>
#include <unordered_map>
#include <tuple>
#include <cstdint>
#include <iomanip>

const std::unordered_map<std::string, std::tuple<char, char, char, char>> c2_c4_map = {
    {"00", std::make_tuple('1', '0', '0', '0')},
    {"01", std::make_tuple('0', '1', '0', '0')},
    {"10", std::make_tuple('0', '0', '1', '0')},
    {"11", std::make_tuple('0', '0', '0', '1')},
    {"0x", std::make_tuple('1', '1', '0', '0')},
    {"x0", std::make_tuple('1', '0', '1', '0')},
    {"1x", std::make_tuple('0', '0', '1', '1')},
    {"x1", std::make_tuple('0', '1', '0', '1')},
    {"xx", std::make_tuple('1', '1', '1', '1')}
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

    std::string line;
    std::size_t sz = 0;
    std::vector<std::vector<std::string>> str_vec_vec{20};
    for (auto &strv : str_vec_vec) {
        strv.emplace_back("");
        strv.emplace_back("");
        strv.emplace_back("");
        strv.emplace_back("");
    }

    while (getline(ifstr, line)) {
        const std::regex r(R"([01x]{40})");
        if (!regex_match(line, r)) {
            std::cout << "tcam entry shall only include 40 collums of [01x]." << std::endl;
            return -1;
        }

        std::vector<std::string> str_vec;
        for (auto i = 0UL; i < line.size(); ++i, ++i) {
            str_vec.emplace_back(line.substr(i, 2));
        }

        std::reverse(str_vec.begin(), str_vec.end());

        auto i = 0UL;
        for (const auto &c2 : str_vec) {
            auto c4 = c2_c4_map.at(c2);
            str_vec_vec[i][0].insert(0, 1, std::get<0>(c4));
            str_vec_vec[i][1].insert(0, 1, std::get<1>(c4));
            str_vec_vec[i][2].insert(0, 1, std::get<2>(c4));
            str_vec_vec[i][3].insert(0, 1, std::get<3>(c4));
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
    for (const auto &str_vec : str_vec_vec) {
        ofstr << "sram chip #" << std::setw(2) << std::setfill('0') << i << "\n";
        for (const auto &str : str_vec) {
            ofstr << str << "\n";
            u32_vec.emplace_back(std::stoul(str, nullptr, 2));
            #ifdef DEBUG
            std::cout << str << "\n";
            std::cout << std::stoul(str, nullptr, 2) << "\n";
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