
#include "pattern.h"
#include <algorithm>
#include <fstream>
#include <regex>
#include <string>

namespace app {

using std::string;
using std::string_view;

namespace {

    class Loader {
    public:
        [[nodiscard]] virtual bool isComment(const string& line) const = 0;
        [[nodiscard]] virtual string extractName(const string& line) const = 0;
        [[nodiscard]] virtual Pattern load(string_view name, const std::vector<string>& strings) const = 0;
    };

    class TxtLoader : public Loader {
    public:
        [[nodiscard]] bool isComment(const string& line) const override {
            return line.starts_with('!');
        }

        [[nodiscard]] string extractName(const string& line) const override {
            std::smatch match;
            if (std::regex_match(line, match, namePattern) && match.size() == 3) {
                return match[2].str();
            }
            return "";
        }

        [[nodiscard]] Pattern load(string_view name, const std::vector<string>& strings) const override {
            return loadPlaintext(name, strings);
        }

    private:
        std::regex namePattern{"!\\s*(Name)?:?\\s(.+)"};
    };

    class RleLoader : public Loader {
    public:
        [[nodiscard]] bool isComment(const string& line) const override {
            return line.starts_with('#') || line.starts_with('x');
        }

        [[nodiscard]] string extractName(const string& line) const override {
            std::smatch match;
            if (std::regex_match(line, match, namePattern) && match.size() == 2) {
                return match[1].str();
            }
            return "";
        }

        [[nodiscard]] Pattern load(string_view name, const std::vector<string>& strings) const override {
            return loadRle(name, strings);
        }

    private:
        std::regex namePattern{"#N?\\s*(.+)"};
    };

    const Loader* getLoader(string_view fileName) {
        static TxtLoader txtLoader;
        static RleLoader rleLoader;
        string::size_type idx = fileName.rfind(".");
        if (idx == string::npos) {
            return nullptr;
        }
        auto ext = fileName.substr(idx + 1);
        if (ext == "txt" || ext == "cells") {
            return &txtLoader;
        }
        if (ext == "rle") {
            return &rleLoader;
        }
        return nullptr;
    }

    Size getSize(Pattern::TCells cells) {
        auto min_max_x = std::minmax_element(cells.begin(), cells.end(), [](auto p1, auto p2) { return p1.x < p2.x; });
        auto min_max_y = std::minmax_element(cells.begin(), cells.end(), [](auto p1, auto p2) { return p1.y < p2.y; });
        return Size{
            min_max_x.second->x - min_max_x.first->x + 1,
            min_max_y.second->y - min_max_y.first->y + 1
        };
    }

} // anonymous namespace


Pattern::Pattern(string_view name, TCells aliveCells) : m_name{name}, m_aliveCells{std::move(aliveCells)},
    m_size{getSize(m_aliveCells)} {
}

Pattern loadPlaintext(string_view name, const std::vector<string>& strings) {
    Pattern::TCells cells;
    for (int y = 0; const auto& line : strings) {
        for (int x = 0; auto c : line) {
            if (c == 'O') {
                cells.push_back({x, y});
            }
            x++;
        }
        y++;
    }
    return Pattern{name, cells};
}

Pattern loadRle(string_view name, const std::vector<string>& strings) {
    Pattern::TCells cells;

    for (int y = 0, x = 0; const auto& line : strings) {
        for (string::size_type pos = 0, tagPos{}; (tagPos = line.find_first_of("bo$!", pos)) != string::npos; ++pos) {
            auto tag = line[tagPos];
            if (tag == '!') {
                // end of file
                return Pattern{name, cells};
            }
            auto countStr = line.substr(pos, tagPos - pos);
            int count = countStr.empty() ? 1 : std::stoi(countStr);
            if (tag == '$') {
                // new lines
                y += count;
                x = 0;
            } else if (tag == 'b') {
                // dead
                x += count;
            } else if (tag == 'o') {
                // alive
                for (int i = 0; i < count; i++) {
                    cells.push_back({x++, y});
                }
            }
            pos = tagPos;
        }
    }
    return Pattern{name, cells};
}

std::optional<Pattern> loadFromFile(string_view fileName, string_view filePath) {
    const Loader* loader = getLoader(fileName);
    if (loader == nullptr) {
        return std::nullopt;
    }

    std::ifstream file{filePath.data()};
    string line;
    std::vector<string> strings;
    string name;

    while (std::getline(file, line)) {
        if (!loader->isComment(line)) {
            strings.push_back(line);
        } else if (name.empty()) {
            name = loader->extractName(line);
        }
    }
    if (name.empty()) {
        name = fileName;
    }
    return strings.empty() ? std::nullopt : std::optional(loader->load(name, strings));
}

}  // namespace app