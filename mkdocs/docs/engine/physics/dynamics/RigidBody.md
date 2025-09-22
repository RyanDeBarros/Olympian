# Rigid Body

| Class name      | `oly::physics::RigidBody`                                                                         |
|-----------------|---------------------------------------------------------------------------------------------------|
| Header file     | `physics/dynamics/bodies/RigidBody.h`                                                             |
| Base classes    | -                                                                                                 |
| Derived classes | [`StaticBody`](StaticBody.md), [`LinearBody`](LinearBody.md), [`KinematicBody`](KinematicBody.md) |

## Description

## Public methods

### `add_collider()`

### `clear_colliders()`

### `collider()`

??? note "`const col2d::Collider& collider(size_t i = 0) const`"
    !!! info "Description"
        Gets a reference to the specified collider.
    !!! abstract "Arguments"
        * `size_t i = 0`: Index of the collider
    !!! abstract "Return Value"
        `const col2d::Collider&` Reference to the collider

??? note "`col2d::Collider& collider(size_t i = 0)`"
    !!! info "Description"
        Gets a reference to the specified collider.
    !!! abstract "Arguments"
        * `size_t i = 0`: Index of the collider
    !!! abstract "Return Value"
        `col2d::Collider&` Reference to the collider

### `collider_index()`

### `collision_view()`

### `erase_collider()`

### `get_local()`

### `get_transformer()`

### `is_colliding()`

### `num_colliders()`

### `set_local()`

### `set_transformer()`

### `state()`

### `update_view()`
