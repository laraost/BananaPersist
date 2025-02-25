#pragma once

#include "datastructure/list_item.h"
#include "persistence_defs.h"
#include "utility/recycling_object_pool.h"
#include <bit>
#include <cstdint>
#include <functional>
#include <optional>
#include <unordered_map>
#include <unordered_set>

namespace bananas {

class persistent_pair {
    
public:
    list_item* birth;
    list_item* death;
};

inline bool operator==(const persistent_pair& a, const persistent_pair& b) {
    return (a.birth == b.birth) && (a.death == b.death);
}

inline bool operator<(const persistent_pair& a, const persistent_pair& b) {
    if (a.birth < b.birth) {
        return true;
    }
    if (b.birth < a.birth) {
        return false;
    }
    return a.death < b.death;
}

class persistence_diagram {

struct pair_hash {

    [[nodiscard]] inline uint64_t operator()(const persistent_pair &pair) const {
        const auto hash_a = std::hash<list_item*>()(pair.birth);
        return std::rotl(hash_a, 1) + hash_a + std::hash<list_item*>()(pair.death);
    }

};

public:
    enum class diagram_type {
        ordinary,
        essential,
        relative
    };

    struct difference {
        size_t points;
        size_t arrows;
    };

    using arrow_map_t = std::unordered_map<list_item*, list_item*>;

public:

    template<diagram_type dgm>
    void add_pair(list_item* birth, list_item* death);

    void add_arrow(list_item* birth_child, list_item* birth_parent);

    void clear_diagrams();

    list_item* get_death(list_item* birth) const;
    std::optional<persistent_pair> get_parent(list_item* birth) const;

    static difference symmetric_difference(const persistence_diagram& a, const persistence_diagram& b);

private:
    std::unordered_set<persistent_pair, pair_hash> ordinary_dgm;
    std::unordered_set<persistent_pair, pair_hash> essential_dgm;
    std::unordered_set<persistent_pair, pair_hash> relative_dgm;

    arrow_map_t arrow_map;

    std::unordered_map<list_item*, persistent_pair> birth_pair_map;

};

} // End of namespace `bananas`
