# Geometry Painter

| Class name      | `oly::rendering::GeometryPainter`   |
|-----------------|-------------------------------------|
| Header file     | `graphics/shapes/GeometryPainter.h` |
| Base classes    | -                                   |
| Derived classes | -                                   |

## Description

## Public inner classes

* [`PaintSupport`](PaintSupport.md)

## Public typedefs

* `using PaintFunction = std::function<void(PaintSupport&)>`

## Public methods

### `GeometryPainter()`

??? note "`GeometryPainter(const PaintFunction& paint_fn)`"
    !!! info "Description"
        Constructor.
    !!! abstract "Arguments"
        * `const PaintFunction& paint_fn`: Paint callback

??? note "`GeometryPainter(const PaintFunction& paint_fn, SpriteBatch* batch)`"
    !!! info "Description"
        Constructor.
    !!! abstract "Arguments"
        * `const PaintFunction& paint_fn`: Paint callback
        * `SpriteBatch* batch`: Sets which sprite batch to use for drawing the painted texture. Can pass `nullptr` to avoid initializing on a sprite batch.

??? note "`GeometryPainter(const GeometryPainter&)`"
    !!! info "Description"
        Copy constructor.

??? note "`GeometryPainter(GeometryPainter&&) noexcept`"
    !!! info "Description"
        Move constructor.

??? note "`~GeometryPainter()`"
    !!! info "Description"
        Destructor.

??? note "`GeometryPainter& operator=(const GeometryPainter&)`"
    !!! info "Description"
        Copy assignment operator.

??? note "`GeometryPainter& operator=(GeometryPainter&&) noexcept`"
    !!! info "Description"
        Move assignment operator.

### `draw()`

### `flag_dirty()`

### `get_ellipse_batch()`

### `get_polygon_batch()`

### `get_sprite_batch()`

### `regen_to_current_resolution()`

### `set_ellipse_batch()`

### `set_polygon_batch()`

### `set_sprite_batch()`

## Public data members

| Data member              | Default value         | Description                                                                                                                                                                                                    |
|--------------------------|-----------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `PaintFunction paint_fn` | `[](PaintSupport) {}` | Callback that's executed when writing to the `GeometryPainter`'s texture. Use it to draw geometric renderables. Also use the `PaintSupport` (see [`PaintSupport`](PaintSupport.md)) for proper batch flushing. |
