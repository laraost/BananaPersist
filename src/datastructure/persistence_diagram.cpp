#include <algorithm>
#include <iterator>
#include <optional>

#include "datastructure/persistence_diagram.h"
#include "datastructure/list_item.h"
#include "utility/errors.h"
#include "persistence_defs.h"

using namespace bananas;
using persistence_diagram::diagram_type::essential;
using persistence_diagram::diagram_type::ordinary;
using persistence_diagram::diagram_type::relative;

template<persistence_diagram::diagram_type dgm>
void persistence_diagram::add_pair(list_item* birth, list_item* death) {
    massert(!birth_pair_map.contains(birth), "Can't add a point that already exists.");
    massert(birth != nullptr, "Persistent pair needs to have a birth");
    massert(death != nullptr, "Persistent pair needs to have a death");

    auto new_pair = persistent_pair{birth, death};
    birth_pair_map.insert({birth, new_pair});
    if constexpr (dgm == ordinary) {
        ordinary_dgm.insert(new_pair);
    } else if constexpr (dgm == essential) {
        essential_dgm.insert(new_pair);
    } else if constexpr (dgm == relative) {
        relative_dgm.insert(new_pair);
    }
}
template void
    persistence_diagram::add_pair<essential>(list_item*, list_item*);
template void
    persistence_diagram::add_pair<ordinary>(list_item*, list_item*);
template void
    persistence_diagram::add_pair<relative>(list_item*, list_item*);

void persistence_diagram::add_arrow(list_item* birth_child, list_item* birth_parent) {
    massert(birth_pair_map.contains(birth_child), "Child needs to have an associated pair.");
    massert(birth_pair_map.contains(birth_parent), "Parent needs to have an associated pair.");

    arrow_map.insert({birth_child, birth_parent});
}

void persistence_diagram::clear_diagrams() {
    birth_pair_map.clear();
    ordinary_dgm.clear();
    essential_dgm.clear();
    relative_dgm.clear();
    arrow_map.clear();
}

list_item* persistence_diagram::get_death(list_item* birth) const {
    if (!birth_pair_map.contains(birth)) {
        return nullptr;
    }
    return birth_pair_map.at(birth).death;
}

std::optional<persistent_pair> persistence_diagram::get_parent(list_item* birth) const {
    if (!arrow_map.contains(birth)) {
        return std::nullopt;
    }
    return birth_pair_map.at(arrow_map.at(birth));
}

namespace detail {

struct arrow {
    persistent_pair child;
    persistent_pair parent;
};

bool operator<(const arrow& a, const arrow& b) {
    if (a.child < b.child) {
        return true;
    }
    if (b.child < a.child) {
        return false;
    }
    return a.parent < b.parent;
}

} // End of namespace `detail` 

persistence_diagram::difference persistence_diagram::symmetric_difference(const persistence_diagram& a, const persistence_diagram &b) {
    std::vector<persistent_pair> points_a;
    points_a.reserve(a.ordinary_dgm.size() + a.essential_dgm.size() + a.relative_dgm.size());
    points_a.insert(points_a.end(), a.ordinary_dgm.begin(), a.ordinary_dgm.end());
    points_a.insert(points_a.end(), a.essential_dgm.begin(), a.essential_dgm.end());
    points_a.insert(points_a.end(), a.relative_dgm.begin(), a.relative_dgm.end());
    std::vector<persistent_pair> points_b;
    points_b.reserve(b.ordinary_dgm.size() + b.essential_dgm.size() + b.relative_dgm.size());
    points_b.insert(points_b.end(), b.ordinary_dgm.begin(), b.ordinary_dgm.end());
    points_b.insert(points_b.end(), b.essential_dgm.begin(), b.essential_dgm.end());
    points_b.insert(points_b.end(), b.relative_dgm.begin(), b.relative_dgm.end());

    std::vector<detail::arrow> arrows_a;
    arrows_a.reserve(a.arrow_map.size());
    for (auto& arrow_pair: a.arrow_map) {
        arrows_a.push_back({a.birth_pair_map.at(arrow_pair.first), a.birth_pair_map.at(arrow_pair.second)});
    }
    std::vector<detail::arrow> arrows_b;
    arrows_b.reserve(b.arrow_map.size());
    for (auto& arrow_pair: b.arrow_map) {
        arrows_b.push_back({b.birth_pair_map.at(arrow_pair.first), b.birth_pair_map.at(arrow_pair.second)});
    }

    std::sort(points_a.begin(), points_a.end());
    std::sort(points_b.begin(), points_b.end());
    std::sort(arrows_a.begin(), arrows_a.end());
    std::sort(arrows_b.begin(), arrows_b.end());

    std::vector<persistent_pair> diff_points;
    std::vector<detail::arrow> diff_arrows;

    std::set_symmetric_difference(points_a.begin(), points_a.end(),
                                  points_b.begin(), points_b.end(),
                                  std::back_inserter(diff_points));
    std::set_symmetric_difference(arrows_a.begin(), arrows_a.end(),
                                  arrows_b.begin(), arrows_b.end(),
                                  std::back_inserter(diff_arrows));
    return {diff_points.size(), diff_arrows.size()};
}
