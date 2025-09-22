# Paint Support

| Class name      | `oly::rendering::GeometryPainter::PaintSupport` |
|-----------------|-------------------------------------------------|
| Header file     | `graphics/shapes/GeometryPainter.h`             |
| Base classes    | -                                               |
| Derived classes | -                                               |

## Description
Also see `PaintSupport`'s outer class [`GeometryPainter`](GeometryPainter.md).

## Public methods

### `pre_polygon_draw()`

???+ note "`void pre_polygon_draw()`"
    !!! info "Description"
        Call before drawing a polygonal renderable to flush any pending non-polygonal batching.

### `pre_ellipse_draw()`

???+ note "`void pre_ellipse_draw()`"
    !!! info "Description"
        Call before drawing an ellipse renderable to flush any pending non-ellipse batching.
