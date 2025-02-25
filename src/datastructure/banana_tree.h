#pragma once

#include <iterator>
#include <optional>
#include <ostream>
#include <utility>
#include <variant>
#include <vector>

#include "datastructure/dictionary.h"
#include "datastructure/persistence_diagram.h"
#include "persistence_defs.h"
#include "datastructure/list_item.h"
#include "utility/recycling_object_pool.h"

namespace bananas {

    template<int sign>
        requires sign_integral<decltype(sign), sign>
    class banana_tree;
    template<int sign>
        requires sign_integral<decltype(sign), sign>
    class banana_tree_node;

    template<typename node_type>
    class walk_iterator;
    template<typename node_type>
    class string_iterator;

    namespace internal {
        enum class spine_pos {
            not_on_spine,
            on_left_spine,
            on_right_spine,
            on_both_spines
        };

        // A pair of banana tree nodes.
        // More specifically, a pair of minimum and maximum.
        template<int sign>
            requires sign_integral<decltype(sign), sign>
        struct item_pair {
        public:
            using item_ptr_type = list_item*;

            item_pair(item_ptr_type min, item_ptr_type max) : min(min), max(max) {}

            // Get the node which, in a banana tree of sign `S`,
            // is a minimum.
            template<int S>
                requires sign_integral<decltype(S), S>
            item_ptr_type get_min() const {
                if constexpr (S == sign) {
                    return min;
                } else {
                    return max;
                }
            }
            // Get the node which, in a banana tree of sign `S`,
            // is a maximum.
            template<int S>
                requires sign_integral<decltype(S), S>
            item_ptr_type get_max() const {
                if constexpr (S == sign) {
                    return max;
                } else {
                    return min;
                }
            }

        private:
            item_ptr_type min;
            item_ptr_type max;

        };

        // A stack of bananas as used in the splitting algorithm.
        // The intended use is to push some bananas onto the stack,
        // then "iterate" through them multiple times.
        // 
        // There is no way to remove bananas from the stack.
        template<int sign>
            requires sign_integral<decltype(sign), sign>
        class banana_stack {
        public:
            using banana_type = item_pair<sign>;

            // Push the given banana onto the stack.
            // Resets the `top` pointer to point to the element just pushed onto the stack.
            void push(const banana_type &banana);
            // "Remove" the topmost banana from the stack.
            // This doesn't actually remove the banana,
            // but rather moves a pointer around,
            // such that we can reuse the same stack later.
            void pop();
            // Actually, really, totally forever remove the topmost banana off the stack.
            // Also resets the top-pointer.
            void actually_pop();
            // Get the banana on top of the stack.
            banana_type& top() const;
            // Reset the `top` pointer to point to the topmost banana on the stack.
            void reset_top();

            // Checks whether no more bananas can be popped of the stack.
            bool empty() const;

        private:
            std::vector<banana_type> stack;
            std::vector<banana_type>::reverse_iterator top_iter;

        };

        using stack_variant = std::variant<std::reference_wrapper<internal::banana_stack<1>>,
                                           std::reference_wrapper<internal::banana_stack<-1>>>;

        template<int sign>
            requires sign_integral<decltype(sign), sign>
        std::optional<stack_variant> top_banana(banana_stack<sign> &L_stack,
                                                banana_stack<sign> &M_stack,
                                                banana_stack<sign> &R_stack,
                                                banana_stack<-sign> &L_inv_stack,
                                                banana_stack<-sign> &R_inv_stack);

        void pop_from_var_stack(internal::stack_variant &var);
        void actually_pop_from_var_stack(internal::stack_variant &var);
        template<int sign>
            requires sign_integral<decltype(sign), sign>
        internal::item_pair<sign> top_of_var_stack(internal::stack_variant &var);
        template<int sign>
            requires sign_integral<decltype(sign), sign>
        bool holds_stack(const internal::stack_variant &var, const banana_stack<sign> &stack);

        // Add the missing short wave banana to `L_inv_stack` or `R_inv_stack`
        // that is not found by `banana_tree::load_stacks`, if it exists.
        template<int sign>
            requires sign_integral<decltype(sign), sign>
        std::optional<stack_variant> add_missing_short_wave_banana(internal::banana_stack<sign> &L_stack,
                                                                   internal::banana_stack<sign> &M_stack,
                                                                   internal::banana_stack<sign> &R_stack,
                                                                   internal::banana_stack<-sign> &L_inv_stack,
                                                                   internal::banana_stack<-sign> &R_inv_stack,
                                                                   function_value_type cut_value);

        // Helper function for `add_missing_short_wave_banana` treating the special case
        // where the maximum of the topmost banana is the special root.
        // `top_banana` is the topmost banana, passed as an argument to avoid calling top_banana twice.
        template<int sign>
            requires sign_integral<decltype(sign), sign>
        std::optional<stack_variant> add_missing_short_wave_banana_sr(internal::stack_variant &top_banana,
                                                                      internal::banana_stack<sign> &M_stack,
                                                                      internal::banana_stack<sign> &R_stack,
                                                                      internal::banana_stack<-sign> &L_inv_stack,
                                                                      internal::banana_stack<-sign> &R_inv_stack,
                                                                      function_value_type cut_value);

    } // End of namespace `internal`

    // A node in a banana tree.
    // The template parameter `sign` is either 1 or -1;
    // 1 for use in the up-tree, -1 for use in the down-tree.
    template<int sign>
        requires sign_integral<decltype(sign), sign>
    class banana_tree_node {

        public:
            constexpr static int sign_v = sign;
            using self = banana_tree_node<sign>;
            using opposite_type = banana_tree_node< -sign>;

            // Construct a `banana_tree_node` associated with the given `list_item`,
            // and assign the constructed node to `item`.
            // All pointers are initialized with `nullptr`.
            banana_tree_node(list_item* item);

            // Explicitly set the pointers of this node.
            void set_pointers(self* up_ptr,
                              self* down_ptr,
                              self* in_ptr,
                              self* mid_ptr,
                              self* low_ptr,
                              self* death_ptr);

            // Interchange this node with its parent.
            // This node must be a maximum and `up` may not be null.
            void max_interchange_with_parent();

            // Interchange `this` with `other`, where both `this` and `other` are minima
            // and `this` has lower value than `other`.
            void min_interchange_below(self* other);

            // Replace the item stored at this node with `new_item`
            // and change the assignment of nodes to items accordingly.
            // Expects that `new_item->get_node<sign>() == nullptr`.
            void replace_item(list_item* new_item);

            // Return `true` if `this` is on an in-trail, i.e.,
            // if `this = this->low->in(->up)*`
            [[nodiscard]] bool is_on_in_trail() const;
            // Return `true` if `this` is on a mid-trail, i.e.,
            // if `this = this->low->mid(->up)*`
            [[nodiscard]] bool is_on_mid_trail() const;

            [[nodiscard]] list_item* get_item() const { return item; }
            [[nodiscard]] function_value_type get_value() const { return item->value<sign>(); }

            [[nodiscard]] const self* get_up() const { return up; }
            [[nodiscard]] const self* get_down() const { return down; }
            [[nodiscard]] const self* get_in() const { return in; }
            [[nodiscard]] const self* get_mid() const { return mid; }
            [[nodiscard]] const self* get_low() const { return low; }
            [[nodiscard]] const self* get_death() const { return death; }

            // Get `this` node's partner, i.e., the node `a` for which `a->death = this`.
            [[nodiscard]] const self* get_birth() const { return in->low; }
            [[nodiscard]] self* get_birth() { return in->low; }

            [[nodiscard]] bool is_special_root() const { return low == in->low && low != this; }
            [[nodiscard]] bool is_leaf() const { return low == this; }
            [[nodiscard]] bool is_internal() const { return low != this; }
            [[nodiscard]] bool is_hook() const { return item->left_neighbor() == nullptr && item->right_neighbor() == nullptr; }
            [[nodiscard]] bool has_empty_banana() const { return in == mid; }

            [[nodiscard]] bool is_on_left_spine() const { return spine_label == internal::spine_pos::on_left_spine ||
                                                                 spine_label == internal::spine_pos::on_both_spines; }
            [[nodiscard]] bool is_on_right_spine() const { return spine_label == internal::spine_pos::on_right_spine ||
                                                                  spine_label == internal::spine_pos::on_both_spines; }
            [[nodiscard]] bool is_on_both_spines() const { return spine_label == internal::spine_pos::on_both_spines; }
            [[nodiscard]] bool is_on_spine() const { return spine_label != internal::spine_pos::not_on_spine; }

            [[nodiscard]] opposite_type* get_opposite_node() const {
                return item->get_node< -sign>();
            }

        private:
            // The `list_item` represented by this node.
            list_item* item = nullptr;

            // Next node upwards on the trail
            self* up = nullptr;
            // Next node downwards on the trail
            self* down = nullptr;
            // First node on the in-trail
            self* in = nullptr;
            // First node on the mid-trail
            self* mid = nullptr;
            // Lower end of the banana
            self* low = nullptr;
            // For minima `death` points to the other end of the banana
            // For maxima this pointer is meaningless.
            self* death = nullptr;
            // Label to indicate whether this node is on a left/right spine or not
            internal::spine_pos spine_label = internal::spine_pos::not_on_spine;
            
            friend class banana_tree<sign>;
            friend class walk_iterator<self>;



            // If `this` is a leaf, set the in-pointer to point to `node`,
            // else set the up-pointer to point to `node`.
            void set_in_or_up(self* node);
            // If `this` is a leaf, set the mid-pointer to point to `node`,
            // else set the up-pointer to point to `node`.
            void set_mid_or_up(self* node);

            // If `set_in` is true, set `this->in` to point to `node`,
            // else set `this->down` to point to `node`.
            void set_in_or_down(self* node, bool set_in);
            // If `set_mid` is true, set `this->mid` to point to `node`,
            // else set `this->down` to point to `node`.
            void set_mid_or_down(self* node, bool set_mid);


            // Helper functions for local operations

            // Interchange `this` with the maximum above (i.e., with `this->up`),
            // where `this->up` and `this` are on the same trail
            // and `this->up->get_birth()` has value greater than `this->get_birth()`.
            // Case 1 in the paper.
            void parallel_max_interchange_without_swap();
            // Interchange `this` with the maximum above (i.e., with `this->up`),
            // where `this->up` and `this` are on the same trail
            // and `this->up->get_birth()` has value less than `this->get_birth()`.
            // Case 2 in the paper.
            void parallel_max_interchange_with_swap();

            // Interchange `this` with the maximum above (i.e., with `this->up`),
            // where `this` is at the top of `this->up`s in-trail (i.e., `this->up->in == this`).
            // Reverse of case 1 in the paper.
            void nested_max_interchange_in_trail();
            // Interchange `this` with the maximum above (i.e., with `this->up`),
            // where `this` is at the top of `this->up`s mid-trail (i.e., `this->up->mid == this`).
            // Reverse of case 1 in the paper.
            void nested_max_interchange_mid_trail();

            // Unlink this *internal* node from its trail and patch the hole that's left.
            // This node must not be a leaf and have non-null `up` and `down` pointers.
            void unlink_from_trail();

            // Insert `node` at the top of this node's in-trail.
            // `this` is expected to be an internal node.
            void insert_node_on_top_of_in(self* node);
            // Insert `node` at the top of this node's mid-trail.
            // `this` is expected to be an internal node.
            void insert_node_on_top_of_mid(self* node);

            // Insert `node` at the bottom of this node's in-trail.
            // `this` is expected to be a leaf.
            void insert_node_on_bottom_of_in(self* node);
            // Insert `node` at the bottom of this node's mid-trail.
            // `this` is expected to be a leaf.
            void insert_node_on_bottom_of_mid(self* node);

            // Insert `this` above `node` in the same trail as `node`,
            // such that `node->up = this`.
            // `node` needs to be on a trail, i.e., `node->up != nullptr` and `node->down != nullptr`
            void insert_this_above(self* node);

            // Insert `this` below `node` in the same trail as `node,
            // such that `node->down = this`.
            // `node` needs to be on a trail, i.e., `node->up != nullptr` and `node->down != nullptr`.
            void insert_this_below(self* node);

            // Swap the trails beginning at `this` with the trails beginning at `node`.
            // `this` and `node` must both be internal nodes.
            void swap_bananas_with_internal_node(self* node);
            // Swap only the in-trails beginning at `this` and `node`.
            // `this` and `node` must both be internal nodes.
            // Warning: this leaves the banana tree in an invalid state!
            void swap_in_trail_with_internal_node(self* node);
            // Swap only the mid-trails beginning at `this` and `node`.
            // `this` and `node` must both be internal nodes.
            // Warning: this leaves the banana tree in an invalid state!
            void swap_mid_trail_with_internal_node(self* node);

            // Merge the in-trail beginning at `this` to the trail containing `this`.
            // `this->in` becomes `down` of `this->up`.
            void merge_in_trail_to_up();
            // Merge the mid-trail beginning at `this` to the trail containing `this`.
            // `this->mid` becomes `down` of `this->up`.
            void merge_mid_trail_to_up();
    };

    using up_tree_node = banana_tree_node<1>;
    using down_tree_node = banana_tree_node<-1>;

    // A banana tree, i.e., either the up- or down-tree
    // The template parameter `sign` is either 1 or -1.
    // If `sign == 1`, then the tree is an up-tree;
    // if `sign == -1`, then the tree is a down-tree.
    template<int sign>
        requires sign_integral<decltype(sign), sign>
    class banana_tree {

        public:
            static constexpr int sign_v = sign;
            using node_type = banana_tree_node<sign>;
            using node_ptr_type = banana_tree_node<sign>*;
            using const_node_ptr_type = const banana_tree_node<sign>*;
            using node_pool_type = recycling_object_pool<node_type>;

            using tree_type = banana_tree<sign>;

            class walk_iterator_pair;
            class string_iterator_pair;

            // Construct an empty banana tree using `node_pool` to allocate nodes.
            banana_tree(node_pool_type &node_pool);
            // Construct a banana tree for the interval between `left_endpoint` and `right_endpoint`
            // using `node_pool` to allocate nodes.
            banana_tree(node_pool_type &node_pool,
                        list_item* left_endpoint,
                        list_item* right_endpoint);

            banana_tree(const banana_tree &other) = delete;

            banana_tree(banana_tree &&other);

            ~banana_tree();

            // Initialize this tree to be an empty tree ready for cutting.
            // If `left == true`, then the special root is set to `-inf` and the right hook is used as dummy.
            // Otherwise, the special root stays at `+inf` and the left hook is used as dummy.
            void initialize_empty_cut_tree(bool left);

            // Construct the banana tree for the interval between `left_endpoint` and `right_endpoint`.
            void construct(list_item* left_endpoint, list_item* right_endpoint);

            //
            // Local maintenance operations
            //

            // Fix the banana tree after the value of `item` has been increased
            void on_increase_value_of_maximum(list_item* item);

            // Fix the banana tree after the value of `item` has been decreased
            void on_decrease_value_of_maximum(list_item* item);

            // Create a banana from the items given in `item_pair`, a pair of minimum and maximum.
            // The item `next_max` is the maximum closes to `item_pair.min`, such that
            // `item_pair.min` is between `next_max` and `item_pair.max`.
            // This is the item called `b` in the paper.
            void anticancel(list_item* next_max,
                            const list_item_pair &item_pair);

            // Update the banana tree after maximum `old_max` becomes non-critical
            // because its neighbor `new_max` becomes a maximum.
            void max_slide(list_item* old_max,
                           list_item* new_max);
            // Update the banana tree after minimum `old_min` becomes non-critical
            // because its neighbor `new_min` becomes a minimum.
            void min_slide(list_item* old_min,
                           list_item* new_min);

            // Cancel the maximum associated with `item` and its birth.
            // `item` has to be a maximum
            // and its node `n` has to satisfy `n->in == n->mid == n->get_birth()`.
            void cancel_maximum(list_item* item);

            // Cancel the maximum associated with `item` with its neighboring endpoint
            // given by `endpoint`.
            // Assumes that the node representing `item` can be used to represent `endpoint`
            // without requiring further adjustment outside this function.
            // This can be ensured by adjusting the value of `item` to be sufficiently close to that of `endpoint`.
            // Note: this assumption is not checked!
            void cancel_max_with_endpoint(list_item* item, list_item* endpoint);
            // Cancel the minimum associated with `item` with its neighboring endpoint
            // given by `endpoint`.
            // Assumes that the node representing `item` can be used to represent `endpoint`
            // without requiring further adjustment outside this function.
            // This can be ensured by adjusting the value of `item` to be sufficiently close to that of `endpoint`.
            // Note: this assumption is not checked!
            void cancel_min_with_endpoint(list_item* item, list_item* endpoint);

            // Replace the right endpoint by the given item.
            // Assumes that the new endpoint is "sufficiently" close to the old endpoint.
            void replace_right_endpoint(list_item* new_endpoint);

            // Replace the left endpoint by the given item.
            // Assumes that the new endpoint is "sufficiently" close to the old endpoint.
            void replace_left_endpoint(list_item* new_endpoint);

            //
            // Topological maintenance operations
            //

            // Glue `right_tree` to the right of this `banana_tree`.
            // Assumes that all items in `right_tree` are strictly to the right of all items in `this`.
            void glue_to_right(tree_type &right_tree, signed_min_dictionary<sign> &min_dict);

            // Find the smallest banana that's contains `virtual_item` in its in- or mid-panel.
            internal::item_pair<sign> smallest_banana(const list_item& virtual_item,
                                                      signed_min_dictionary<sign> &min_dict,
                                                      signed_max_dictionary<sign> &max_dict) const;

            // Load the stacks `L_stack`, `M_stack`, `R_stack` with bananas cut by `virtual_item`,
            // such that
            //  - `L_stack` contains bananas `(p,q)` where `virtual_item` is in the in-panel, with `q < p < virtual_item`,
            //  - `M_stack` contains bananas `(p,q)` where `virtual_item` is in the mid-panel, with `virtual_item` in `[p,q]` or `[q,p]`,
            //  - `R_stack` contains bananas `(p,q)` where `virtual_item` is in the in-panel, with `virtual_item < p < q`.
            void load_stacks(const list_item& virtual_item,
                             const internal::item_pair<sign>& smallest_banana,
                             internal::banana_stack<sign> &L_stack,
                             internal::banana_stack<sign> &M_stack,
                             internal::banana_stack<sign> &R_stack) const;

            // Cut this tree at `cut_item`, given the stack loaded by `load_stacks`.
            // If the maximum `q` of the first banana on the stacks is such that `*q->item < cut_item`
            // then `other_tree` will contain the items to the right of `cut_item`,
            // otherwise it will contain the items to the left of `cut_item`.
            // The items `left_of_cut` and `right_of_cut` are the items immediately to the left and right of `cut_item`, respectively.
            // Returns `true` if the tree is cut in the left spine, false otherwise.
            bool cut(list_item& cut_item,
                     list_item& left_of_cut,
                     list_item& right_of_cut,
                     tree_type &other_tree,
                     internal::banana_stack<sign> &L_stack,
                     internal::banana_stack<sign> &M_stack,
                     internal::banana_stack<sign> &R_stack,
                     internal::banana_stack<-sign> &L_inv_stack,
                     internal::banana_stack<-sign> &R_inv_stack);

            //
            // 
            //

            [[nodiscard]] inline const node_type* get_special_root() const { return special_root_item.get_node<sign>(); }
            [[nodiscard]] inline node_type* get_special_root() { return special_root_item.get_node<sign>(); }
            [[nodiscard]] inline const node_type* get_left_hook() const { return left_hook_item.get_node<sign>(); }
            [[nodiscard]] inline const node_type* get_right_hook() const { return right_hook_item.get_node<sign>(); }

            [[nodiscard]] inline list_item* get_global_max() const { return global_max; }

            [[nodiscard]] inline list_item* get_left_endpoint() const { return left_endpoint; }
            [[nodiscard]] inline list_item* get_right_endpoint() const { return right_endpoint; }

            //
            // Iterators over banana trees
            //

            // Obtain a pair of iterators to recursively list the bananas of this banana tree.
            // This implements the `Walk` function described in the paper for extracting the persistence diagram.
            [[nodiscard]] walk_iterator_pair walk() const;

            [[nodiscard]] string_iterator_pair string() const;

            //
            // Output
            //

            void print(std::ostream &stream) const;

            //
            // Utilities
            //

            // Swap trees `tree_a` and `tree_b` by swapping the nodes associated with special roots and the hooks,
            // and swapping values/orders of special roots and hooks.
            // If `tree_a` and `tree_b` use different node pools, then there will be horrific memory issues,
            // so this is forbidden.
            friend void swap(banana_tree& tree_a, banana_tree& tree_b) {
                massert(&tree_a.node_pool == &tree_b.node_pool,
                        "Cannot swap trees that use a different node pool.");
                tree_a.special_root_item.swap_node_with_item<sign>(tree_b.special_root_item);
                tree_a.left_hook_item.swap_node_with_item<sign>(tree_b.left_hook_item);
                tree_a.right_hook_item.swap_node_with_item<sign>(tree_b.right_hook_item);
                tree_a.special_root_item.swap_order_and_value(tree_b.special_root_item);
                tree_a.left_hook_item.swap_order_and_value(tree_b.left_hook_item);
                tree_a.right_hook_item.swap_order_and_value(tree_b.right_hook_item);
                std::swap(tree_a.left_endpoint, tree_b.left_endpoint);
                std::swap(tree_a.right_endpoint, tree_b.right_endpoint);
            }

        private:
            // A memory pool from which to allocated nodes
            node_pool_type &node_pool;

            // An item for the hook on the left end of the interval
            // This is assigned a value, but may not be represented by a node.
            list_item left_hook_item;
            // An item for the hook on the right end of the interval.
            // Same as `left_hook_item` this may not be represented by a node.
            list_item right_hook_item;
            // An item representing the special root of the tree.
            // Its value is infinity. Note that this is different from the global maximum.
            list_item special_root_item;
            // The item with largest function value (smallest if `sign == -1`).
            list_item* global_max = nullptr;

            // Left endpoint of the interval represented by this banana tree
            list_item* left_endpoint;
            // Right endpoint of the interval represented by this banana tree
            list_item* right_endpoint;

            friend class persistence_data_structure;

            

            // Allocate a new node for the given `list_item`.
            node_ptr_type allocate_node(list_item* item);
            // Free the node associated with `item`.
            // This also resets the node-pointer of `item`.
            void free_node(list_item* item);
            // Free `node` and reset the node-pointer of the associated `list_item`.
            void free_node(node_ptr_type node);

            // Assign order and value to the hook based on the value of `endpoint`.
            // If `left == true` updates the left hook, else updates the right hook.
            template<bool left>
            void assign_hook_value_and_order(list_item* endpoint);
            // Assign order and value to the hook based on the value of its death.
            // If `left == true` updates the left hook, else updates the right hook.
            template<bool left>
            void assign_hook_value_and_order();

            //
            // Private methods related to local maintenance operations
            //

            // Fix the banana tree after the value of `item` has been increased.
            // `callback` is a function object which is called on each interchange of maxima.
            // Its purpose is to notify the caller of the interchanges that are performed,
            // such that interchanges of minima can be performed in parallel.
            // The signature should be equivalent to
            // ```
            // void cb(node_ptr_type a, node_ptr_type b);
            // ```
            // `a` and `b` are the interchanged nodes, where `a->up == b` before the interchange,
            // i.e., the interchange occurs because `a` has greater value than `b`.
            //
            // The `callback` is also called when the item becomes the new global maximum,
            // even though the interchange with the special root is not actually executed.
            // In this case, `b` is the node corresponding to the old global maximum.
            template<typename T>
            void on_increase_value_of_maximum(list_item* item, const T &callback);

            // Fix the banana tree after the value of `item` has been decreased.
            // `callback` is a function object which is called on each interchange of maxima.
            // Its purpose is to notify the caller of the interchanges that are performed,
            // such that interchanges of minima can be performed in parallel.
            // The signature should be equivalent to
            // ```
            // void cb(node_ptr_type a, node_ptr_type b);
            // ```
            // `a` and `b` are the interchanged nodes, where `a->up == b` before the interchange,
            // i.e., the interchange occurs because `a` has greater value than `b`.
            //
            // The `callback` is also called when the item becomes the new global maximum,
            // even though the interchange with the special root is not actually executed.
            // In this case, `b` is the node corresponding to the old global maximum.
            template<typename T>
            void on_decrease_value_of_maximum(list_item* item, const T &callback);

            // Set the global max to `item` if `item`'s value is greater than the current max's value.
            void update_global_max(list_item* item);
            // Update the global max based on `in` and `mid` pointers of the special root.
            void update_global_max();

            //
            // Private methods related to topological maintenance operations
            // Implemented in `banana_tree_topological_operations.cpp`
            //

            // Reverse of `do_injury`.
            // In the paper `max_node` is $q$, `other_min_node` is $\alpha$
            void undo_injury(node_ptr_type max_node, node_ptr_type other_min_node);
            void undo_fatality(node_ptr_type min_node, node_ptr_type max_node, node_ptr_type other_min_node);
            // `min_node` is the node whose value is adjusted (i.e., the hook)
            void undo_scare(node_ptr_type min_node);

            // Swap the trails going down from `top_of_in` and `top_of_mid` with the leaf `node`.
            // The trail from `top_of_in->low` to `top_of_in` becomes a mid-trail in the process;
            // the trail from `top_of_mid->low` to `top_of_mid` becomes an in-trail.
            // `low` pointers on the sections of trails that haven't been moved are updated accordingly.
            //
            // For details see the description of fatalities in the paper.
            //
            // This happens: s = `top_of_in`, t = `top_of_mid`, a = `node`, `i = a->in`, `m = a->mid`
            //       q           b                  q          b
            //     / |         / |                 /|         / |
            //    s  t        .  .                |/        i  m
            //    .  .        .  .      ==>       a          .  . 
            //    .  .        i  m                           s  t
            //    | /         | /                            | /
            //    p           a                              p    
            //
            // There may be nodes between `s` and `q` and between `t` and `q`.
            void fatality_swap(node_ptr_type top_of_in,
                               node_ptr_type top_of_mid,
                               node_ptr_type node);

            // Prepare `this` and `right_tree` by removing nodes that will become non-critical
            // when `right_tree` is glued to the right of `this`.
            // This is the pre-processing step to establish the assumption for the gluing algorithm
            // as described in the paper.
            // Returns a pair of `banana_tree_node` pointers, where
            //  - `first` represents the rightmost critical item of the left tree, i.e., of `this`
            //  - `second` represents the leftmost critical item of the right tree.
            //
            // Afterwards, one interval ends in an up-type item, the other in a down-type item
            // and the up-type item will have lower value that the down-type item.
            std::pair<node_ptr_type, node_ptr_type> prepare_gluing_to_right(tree_type &right_tree,
                                                                            signed_min_dictionary<sign> &min_dict);

            // Remove the node `node_with_hook` from the banana tree, along with the hook it's paired with.
            // The node `node_with_hook` should be internal and not a special root.
            // This is intended as a subroutine for `prepare_gluing_to_right`.
            // Expects that both the in- and mid-trail are empty.
            void remove_node_with_hook(node_ptr_type node_with_hook);

            // Helper function for `prepare_gluing_to_right`.
            // Make `new_hook_node`, which is so far not a hook, into a hook.
            // The template parameter specifies whether the node becomes the left hook or the right hook.
            // If it is true, make `new_hook_node` the left hook;
            // if it is false, make `new_hook_node` the right hook.
            // Returns the node that represents the now outermost proper critical item.
            template<bool left>
            node_ptr_type turn_node_into_hook(node_ptr_type new_hook_node);

            // Helper function for `glue_to_right`.
            // Used to ensure that `this->get_special_root()`
            // becomes the special root of the tree rooted at `other_special_root`.
            void ensure_glued_tree_is_this(node_ptr_type other_special_root, node_ptr_type dummy_node);

            // Helper function for `cut` that executes the main loop of the cutting algorithm.
            // For details on arguments see `cut()`.
            // `dummy_node` points to the dummy node defined in the "other tree".
            void cut_loop(list_item& cut_item,
                          node_ptr_type dummy_node,
                          internal::banana_stack<sign> &L_stack,
                          internal::banana_stack<sign> &M_stack,
                          internal::banana_stack<sign> &R_stack,
                          internal::banana_stack<-sign> &L_inv_stack,
                          internal::banana_stack<-sign> &R_inv_stack);

            // Ensure that this tree's special root satisfies the convention that the special root is at infinity.
            // Update the in- and mid-trail accordingly.
            // `cuts_left == true` indicates that this tree was cut off from the left spine.
            void fix_special_root_after_cut(bool cuts_left);

            // Reassign hooks of `this` and `other_tree`.
            void update_hooks_after_cut(banana_tree& other_tree,
                                        list_item& left_of_cut,
                                        list_item& right_of_cut,
                                        node_ptr_type dummy_node,
                                        bool cuts_left);

            // Reverse of `undo_injury`. Moves the nodes `j` on the in-trail between `max_node->get_birth()` and `max_node`,
            // onto the mid-trail above `dummy_node`,
            // where `list_item::lies_between(cut_item, *j->item, *min_node->item) == true`, where `min_node == max_node->get_birth()`.
            // Returns `true` if a global minimum was found, false otherwise.
            void do_injury(list_item& cut_item,
                           node_ptr_type max_node,
                           node_ptr_type dummy_node);
            // Reverse of `undo_fatality`. Swaps items that are on the other side of `cut_item` relative to `max_node` with `dummy_node`.
            // See fatality_swap for details of the reverse.
            void do_fatality(list_item& cut_item, node_ptr_type min_node, node_ptr_type max_node, node_ptr_type dummy_node);
            // Reverse of `undo_scare`. Sets the value of `dummy_node` to be above that of `min_node`,
            // and performs the necessary interchange.
            void do_scare(node_ptr_type min_node, node_ptr_type dummy_node);

            void update_dummy_position_in_cut(node_ptr_type dummy_node);

        //
        // Iterators and helpers
        //
        public:
            // Helper class for walk iterators
            class walk_iterator_pair {
                public:
                    using iterator_type = walk_iterator<node_type>;

                    // Construct a pair of iterators starting with the banana associated with `initial_max`
                    walk_iterator_pair(banana_tree<sign>::node_ptr_type initial_max);

                    iterator_type begin();
                    iterator_type end();

                private:
                    node_ptr_type initial_max;
            };

            // Helper class for string iterators
            class string_iterator_pair {
                public:
                    using iterator_type = string_iterator<node_type>;

                    // Construct a pair of iterators over the banana tree with `special_root` as the special root.
                    string_iterator_pair(node_ptr_type left_node, node_ptr_type special_root);

                    iterator_type begin();
                    iterator_type end();

                private:
                    node_ptr_type left_node;
                    node_ptr_type special_root;

            };
            
        //
        // Construction algorithm and related data structures
        // The notation is largely as in the paper; see there for details.
        //
        private:
            // A wrapper around an item that adds pointers for the linked list of critical items
            struct construction_item {
                // The previous critical item on the left
                construction_item* prev = nullptr;
                // The previous critical item on the right
                construction_item* next = nullptr;
                // The actual `list_item` that this `construction_item` refers to
                list_item* stored_item = nullptr;

                [[nodiscard]] function_value_type get_value() const {
                    return stored_item->value<sign>();        
                }
            };
        
            // The actual construction of the banana tree,
            // using the linear-time stack-based algorithm described in the paper.
            void construct_impl(list_item* left_endpoint,
                                list_item* right_endpoint);
            
            // The `attach j below b on the left` function. See the paper for details.
            void attach_below_on_left(construction_item* j, construction_item* b);
            // The `attach b below j on the right` function. See the paper for details.
            void attach_below_on_right(construction_item* b, construction_item* j);
            // Fix the `low` and `death` pointers along the trails of the banana
            // spanned by minimum `a` and maximum `b`.
            void fix_banana(construction_item* a, construction_item* b);

            // Set the labels of nodes on splines appropriately.
            void initialize_spline_labels();

    };

    //
    // Declarations of iterators
    //

    // An iterator that recursively walks through the bananas.
    // Dereferencing yields a pair of `banana_tree_node<sign>`s,
    // with the minimum first and the maximum second.
    // Bananas are visited in the order of the `Walk` function given in the paper.
    // Warning: this iterator is expensive to copy. Avoid that at all costs.
    template<typename node_type>
    class walk_iterator {

        public:
            using node_ptr_type = node_type*;
            using value_type = std::pair<node_ptr_type, node_ptr_type>;
            using reference = value_type&;
            using pointer = value_type*; 
            using iterator_category = std::forward_iterator_tag;

        protected:
            walk_iterator(node_ptr_type at_node);
            walk_iterator(const value_type &initial_banana);

        public:
            walk_iterator& operator++();
            reference operator*(); // TODO: I don't like that these operators are non-const
            pointer operator->();

        private:
            std::vector<value_type> banana_stack;
            node_ptr_type current_node;

            friend class banana_tree<1>::walk_iterator_pair;
            friend class banana_tree< -1>::walk_iterator_pair;

            template<typename N>
            friend bool operator!=(const walk_iterator<N> &a, const walk_iterator<N> &b);
        
    };

    // An iterator that visists nodes in a banana tree
    // in the order in which their associated items appear along the interval.
    // The special root itself appears at the beginning.
    template<typename node_type>
    class string_iterator {
        public:
            using node_ptr_type = node_type*;
            using const_node_ptr_type = const node_type*;
            using value_type = const_node_ptr_type;
            using reference = value_type&;
            using pointer = value_type*;
            using iterator_category = std::forward_iterator_tag;

        protected:
            string_iterator(const_node_ptr_type previous_node, const_node_ptr_type start);
        
        public:
            string_iterator& operator++();
            reference operator*();
            pointer operator->();
        
        private:
            const_node_ptr_type previous_node;
            const_node_ptr_type current_node;

            friend class banana_tree<1>::string_iterator_pair;
            friend class banana_tree< -1>::string_iterator_pair;

            template<typename N>
            friend bool operator==(const string_iterator<N> &a, const string_iterator<N> &b);
            template<typename N>
            friend bool operator!=(const string_iterator<N> &a, const string_iterator<N> &b);

    };

    //
    // Comparison operators for iterators
    //

    // Inequality operator for `walk_iterator` instances
    template<typename node_type>
    bool operator!=(const walk_iterator<node_type> &a,
                    const walk_iterator<node_type> &b);

    // Equality operator for `string_iterator` instances
    template<typename node_type>
    bool operator==(const string_iterator<node_type> &a,
                    const string_iterator<node_type> &b);
    // Inequality operator for `string_iterator` instances
    template<typename node_type>
    bool operator!=(const string_iterator<node_type> &a,
                    const string_iterator<node_type> &b);

    // A wrapper around a pair of banana trees, i.e., an up-tree and a down-tree.
    // This manages the pair of up-tree and down-tree
    class persistence_data_structure {

        public:
            persistence_data_structure(recycling_object_pool<up_tree_node> &up_tree_node_pool,
                                       recycling_object_pool<down_tree_node> &down_tree_node_pool);
            persistence_data_structure(recycling_object_pool<up_tree_node> &up_tree_node_pool,
                                       recycling_object_pool<down_tree_node> &down_tree_node_pool,
                                       list_item* left_endpoint,
                                       list_item* right_endpoint);

            void construct(list_item* left_endpoint, list_item* right_endpoint);

            //
            // Local maintenance operations
            //

            // Update the data structure after `max_item`'s value increased.
            void on_increase_value_of_maximum(list_item* max_item);
            // Update the data structure after `max_item`'s value decreased.
            // If `cancel_after_decrease` is `true`, then `max_item` is cancelled with its birth.
            void on_decrease_value_of_maximum(list_item* max_item);
            // Update the data structure after `min_item`'s value increased.
            // If `cancel_after_increase` is `true`, then `min_item` is cancelled with its death.
            void on_increase_value_of_minimum(list_item* min_item);
            // Update the data structure after `min_item`'s value decreased.
            void on_decrease_value_of_minimum(list_item* min_item);

            // Anticancellation of the minimum and maximum given in `new_items`,
            // which are assumed to be at sufficiently small distance in value
            // such that they are paired in the banana tree.
            // These two items have to be neighbors.
            void anticancel(min_dictionary &min_dict,
                            max_dictionary &max_dict,
                            const list_item_pair &new_items);

            // Update the data structure after `min_item` and `max_item` become non-critical
            // Remove `min_item` and `max_item` from the datastructure,
            // where `min_item` and `max_item` are paired and their banana is empty.
            void cancel(list_item* min_item, list_item* max_item);
            // Update the data structure after maximum `item` becomes non-critical
            // and changes the criticality of its neighbor `endpoint`.
            // Expects that `item` is represented by an internal node and
            // `endpoint` is a neighbor of `item` and an endpoint.
            void cancel_max_with_endpoint(list_item* item, list_item* endpoint);
            // Update the data structure after minimum `item` becomes non-critical
            // and changes the criticality of its neighbor `endpoint`.
            // Expects that `item` is represented by an internal node and
            // `endpoint` is a neighbor of `item` and an endpoint.
            void cancel_min_with_endpoint(list_item* item, list_item* endpoint);

            // Update the data structure after `new_max`'s value decreases below that of `old_max`,
            // where `old_max` was a maximum before the decrease and `new_max` was non-critical.
            void max_slide(list_item* old_max, list_item* new_max);
            // Update the data structure after `new_min`'s value decreases below that of `old_min`,
            // where `old_min` was a minimum before the decrease and `new_min` was non-critical.
            void min_slide(list_item* old_min, list_item* new_min);

            // Process a change of the given endpoint from a down-type item to an up-type item.
            void change_down_to_up(list_item* endpoint, list_item* neighbor);
            // Process a change of the given endpoint from an up-type item to a down-type item.
            void change_up_to_down(list_item* endpoint, list_item* neighbor);

            // Replace the right endpoint by the given item.
            // Assumes that the new endpoint is sufficiently close to the old endpoint
            // in terms of value, such that the structure of the trees does not change.
            void replace_right_endpoint(list_item* new_endpoint);
            // Replace the left endpoint by the given item.
            // Assumes that the new endpoint is sufficiently close to the old endpoint
            // in terms of value, such that the structure of the trees does not change.
            void replace_left_endpoint(list_item* new_endpoint);

            //
            // Topological maintenance operations
            //

            // Glue the banana trees of `right_persistence` to this `persistence_data_structure`'s banana trees.
            // `min_dict` and `max_dict` are dictionaries containing the minima and maxima, respectively.
            void glue_to_right(persistence_data_structure &right_persistence,
                               min_dictionary &min_dict,
                               max_dictionary &max_dict);

            // Cut the banana trees of `this` between `left_of_cut` and `right_of_cut`.
            // Returns a `persistence_data_structure` for the part that's cut off:
            // if the cut is on the left spine of the trees of `this`, then the returned PDS stores the items up to `left_of_cut`;
            // otherwise, the returned PDS stores the items to the right of `right_of_cut` (including `right_of_cut`).
            // `left_of_cut` and `right_of_cut` are expected to be a minimum and maximum that are not represented in the trees so far,
            // and are expected to be close enough in value such that they form a window.
            // The items `left_of_cut` and `right_of_cut` should be neighbors at the call,
            // i.e., `left_of_cut->right_neighbor() == right_of_cut`.
            // They are cut during the call, i.e., after `cut` returns
            // `left_of_cut->right_neighbor() == nullptr` and
            // `right_of_cut->left_neighbor() == nullptr`.
            // 
            // Before cutting, an anti-cancellation between `left_of_cut` and `right_of_cut` is executed,
            // and the resulting banana is used as `smallest_banana`.
            persistence_data_structure cut(list_item& left_of_cut,
                                           list_item& right_of_cut,
                                           min_dictionary &min_dict,
                                           max_dictionary &max_dict);

            //
            // 
            //

            void extract_persistence_diagram(persistence_diagram &dgm) const;

            [[nodiscard]] const banana_tree<1>& get_up_tree() const;
            [[nodiscard]] const banana_tree< -1>& get_down_tree() const;

            [[nodiscard]] const up_tree_node* get_up_tree_special_root() const;
            [[nodiscard]] const down_tree_node* get_down_tree_special_root() const;

            [[nodiscard]] const list_item* get_global_max() const;
            [[nodiscard]] const list_item* get_global_min() const;

        private:
            banana_tree<1> up_tree;
            banana_tree< -1> down_tree;
    };

}
