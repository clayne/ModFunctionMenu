#pragma once

#include <absl/container/flat_hash_map.h>

#include <XSEPlugin/Util/Singleton.h>

class Translation final : public SingletonEx<Translation>
{
    friend class SingletonEx<Translation>;

public:
    // Initialize or reload translation.
    static void Init(bool a_abort = true);

    // Lookup translation text.
    [[nodiscard]] std::string Lookup(std::string_view a_key) const
    {
        if (auto it = _map.find(a_key); it != _map.end()) {
            return it->second;
        }
        return std::string{ a_key };
    }

    // Visit translation map.
    template <class Visitor>
    void Visit(Visitor&& a_visitor) const
    {
        for (auto&& [key, value] : _map) {
            a_visitor(key, value);
        }
    }

private:
    Translation() = default;

    ~Translation() = default;

private:
    void Load(bool a_abort);
    void LoadImpl(const std::filesystem::path& a_path);

    absl::flat_hash_map<std::string, std::string> _map;
};
