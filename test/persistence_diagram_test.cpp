#include <gtest/gtest.h>

#include "datastructure/persistence_diagram.h"
#include "example_trees/paper_tree.h"

TEST_F(PaperTreePairTest, ExtractsPaperExampleCorrectly) {
    persistence_diagram diagram;
    persistence.extract_persistence_diagram(diagram);

    // Partners in essential and ordinary subdiagrams
    EXPECT_EQ(diagram.get_death(&items[d]), &items[e]);
    EXPECT_EQ(diagram.get_death(&items[f]), &items[g]);
    EXPECT_EQ(diagram.get_death(&items[h]), &items[i]);
    EXPECT_EQ(diagram.get_death(&items[j]), &items[o]);
    EXPECT_EQ(diagram.get_death(&items[n]), &items[k]);
    EXPECT_EQ(diagram.get_death(&items[l]), &items[m]);

    // Arrows in essential and ordinary subdiagrams
    EXPECT_EQ(diagram.get_parent(&items[d])->birth, &items[j]);
    EXPECT_EQ(diagram.get_parent(&items[f])->birth, &items[j]);
    EXPECT_EQ(diagram.get_parent(&items[h])->birth, &items[j]);
    EXPECT_EQ(diagram.get_parent(&items[j]), std::nullopt);
    EXPECT_EQ(diagram.get_parent(&items[n])->birth, &items[j]);
    EXPECT_EQ(diagram.get_parent(&items[l])->birth, &items[n]);

    // Partners in relative subdiagrams
    EXPECT_EQ(diagram.get_death(&items[c]), &items[d]);
    EXPECT_EQ(diagram.get_death(&items[e]), &items[j]);
    EXPECT_EQ(diagram.get_death(&items[g]), &items[f]);
    EXPECT_EQ(diagram.get_death(&items[i]), &items[h]);
    EXPECT_EQ(diagram.get_death(&items[k]), &items[n]);
    EXPECT_EQ(diagram.get_death(&items[m]), &items[l]);

    // Arrows in relative subdiagram
    EXPECT_EQ(diagram.get_parent(&items[c])->birth, &items[e]);
    EXPECT_EQ(diagram.get_parent(&items[g])->birth, &items[e]);
    EXPECT_EQ(diagram.get_parent(&items[i])->birth, &items[e]);
    EXPECT_EQ(diagram.get_parent(&items[m])->birth, &items[k]);
}

TEST(PersistenceDiagram, SymmetricDifference) {
    list_item a{0, 0};
    list_item b{1, 1};
    list_item c{2, 2};
    list_item d{3, 3};
    list_item e{4, 4};
    list_item f{5, 5};

    persistence_diagram pd_1;
    persistence_diagram pd_2;

    pd_1.add_pair<bananas::persistence_diagram::diagram_type::essential>(&a, &b);
    pd_1.add_pair<bananas::persistence_diagram::diagram_type::essential>(&c, &d);
    pd_1.add_pair<bananas::persistence_diagram::diagram_type::essential>(&e, &f);
    pd_1.add_arrow(&a, &e);
    pd_1.add_arrow(&c, &e);

    pd_2.add_pair<bananas::persistence_diagram::diagram_type::essential>(&a, &b);
    pd_2.add_pair<bananas::persistence_diagram::diagram_type::essential>(&c, &e);
    pd_2.add_arrow(&a, &c);

    auto sym_diff = persistence_diagram::symmetric_difference(pd_1, pd_2);
    EXPECT_EQ(sym_diff.points, 3);
    EXPECT_EQ(sym_diff.arrows, 3);
}
