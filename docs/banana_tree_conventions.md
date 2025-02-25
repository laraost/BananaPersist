# Banana Trees

This document describes conventions used in the implementation. See the paper for a full description of banana trees.

## Nodes in Banana Trees

A node in a banana tree has between two and four neighbors:

- Internal nodes have neighbors on the same trail (pointed to by `up` and `down`) and the first node(s) on their in- and mid-trail (pointed to by `in` and `mid`).
- The special root has the first nodes on its in- and mid-trail as its only neighbors (pointed to by `in` and `mid`).
- Leaves have the first nodes on their in- and mid-trails as neighbors (pointed to by `in` and `mid`).

The `up` and `down` pointers of leaves and the special root are `null`.
All nodes have non-null `low` pointers; only leaves have non-null `death` pointers.

|              | up | down | in | mid | low | death |
|--------------|----|------|----|-----|-----|-------|
|internal node | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :x: |
|leaf          | :x: | :x: | :heavy_check_mark: | :heavy_check_mark: | `= this` | :heavy_check_mark: |
|special root  | :x: | :x: | :heavy_check_mark: | :heavy_check_mark: | `= in->low` | :x: |

### `low` Pointers

`low` pointers generally point to the descendant with the lowest function value.
For internal nodes this is the lower end of the trail containing the node.
The `low`-pointers of leaves point to themselves and the `low`-pointer of the special root points to the birth of the global window, i.e., to the lower end of the trails starting at the special root.

### `death` Pointers

`death` pointers are only not `null` at leaves, where they point to the node upper end of the trails starting at the leaf.

## Construction

Which of the trails beginning at the special root is the in-trail and which is the mid-trail is not specified in the paper. However, the construction algorithm always ends up with the left trail being the in-trail and the right trail being the mid-trail. 

For a node on the special banana, we say that it is on the in-trail if its item is left of the item represented by `low` of the special root. Otherwise, it is on the mid-trail.