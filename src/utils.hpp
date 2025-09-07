#pragma once
#include <string_view>
#include <vector>
#include <ftxui/component/event.hpp>
#include <rapidfuzz/fuzz.hpp>

std::vector<std::string> split_csv_line(std::string_view line, char delimiter = ',');
std::vector<std::string_view> split_csv_line_view(std::string_view line, char delimiter = ',');
std::string EventToString(const ftxui::Event& event);

std::string ExecAndCapture(const std::string& cmd);
std::string substitute_template(std::string_view template_str, const std::vector<std::string>& data);
std::string trim(const std::string& str);

std::vector<std::string> ExtractURLs(const std::string& text);
std::string ExtractFirstURL(const std::string& text);

template <typename Sentence1,
typename Iterable, typename Sentence2 = typename Iterable::value_type>
std::vector<std::pair<Sentence2, double>>
extract(const Sentence1& query, const Iterable& choices, const double score_cutoff = 0.0)
{
    std::vector<std::pair<Sentence2, double>> results;
    rapidfuzz::fuzz::CachedPartialTokenSetRatio<typename Sentence1::value_type> scorer(query);

    for (const auto& choice : choices) {
        double score = scorer.similarity(choice, score_cutoff);

        if (score >= score_cutoff) {
            results.emplace_back(choice, score);
        }
    }

    return results;
}
