#pragma once
#include <string_view>
#include <vector>
#include <algorithm>
#include <cctype>
#include <ftxui/component/event.hpp>
#include <rapidfuzz/fuzz.hpp>

std::vector<std::string> split_csv_line(std::string_view line, char delimiter = ',');
std::vector<std::string_view> split_csv_line_view(std::string_view line, char delimiter = ',');
std::string EventToString(const ftxui::Event& event);

std::string ExecAndCapture(const std::string& cmd);
std::string substitute_template(std::string_view template_str, const std::vector<std::string>& data);
std::string trim(std::string_view str);

std::vector<std::string> ExtractURLs(const std::string& text);
std::string ExtractFirstURL(const std::string& text);

std::vector<std::string> SplitCommand(std::string_view cmd);
int ExecNoShell(std::string_view cmd);

// Smart case: case-insensitive if query is all lowercase, case-sensitive if any uppercase
inline bool hasUppercase(std::string_view str) {
    return std::ranges::any_of(str, [](unsigned char c) { return std::isupper(c); });
}

inline std::string toLower(std::string_view str) {
    std::string result(str);
    std::ranges::transform(result, result.begin(), [](unsigned char c) { return std::tolower(c); });
    return result;
}

// Returns vector of (original_index, score) pairs for choices that pass the cutoff
template <typename Sentence1, typename Iterable>
std::vector<std::pair<size_t, double>>
extract(const Sentence1& query, const Iterable& choices, const double score_cutoff = 70.0)
{
    std::vector<std::pair<size_t, double>> results;

    bool caseSensitive = hasUppercase(query);

    if (caseSensitive) {
        rapidfuzz::fuzz::CachedPartialRatio<typename Sentence1::value_type> scorer(query);
        size_t idx = 0;
        for (const auto& choice : choices) {
            double score = scorer.similarity(choice, score_cutoff);
            if (score >= score_cutoff) {
                results.emplace_back(idx, score);
            }
            ++idx;
        }
    } else {
        std::string lowerQuery = toLower(query);
        rapidfuzz::fuzz::CachedPartialRatio<char> scorer(lowerQuery);
        size_t idx = 0;
        for (const auto& choice : choices) {
            std::string lowerChoice = toLower(choice);
            double score = scorer.similarity(lowerChoice, score_cutoff);
            if (score >= score_cutoff) {
                results.emplace_back(idx, score);
            }
            ++idx;
        }
    }

    return results;
}
