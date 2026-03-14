# Samplers

All sampler classes derive from `Sampler1D`, `Sampler2D`, `Sampler3D`, or `Sampler4D`, located in `graphics/particles/Samplers.h`.

## Math foundation

Note that in the PDF ($f$), CDF ($F$), and Quantile ($F^{\leftarrow}$) definitions the domain is always assumed to be $[0, 1]$. The relationships between these 3 functions are
listed below for each dimension case.

### 1D functions

$$F(x)=\int_0^xf(t)dt$$

$$f(x)=\frac{d}{dx}F(x)$$

$$F^{\leftarrow}(x)=F^{-1}(x)$$

$$F(x)=(F^{\leftarrow})^{-1}(x)$$

### 2D functions

$$F(x,y)=\int_0^x\int_0^yf(u,v)dvdu$$

$$F(x,y)=\int_0^xf_1(u)du\int_0^yf_2(v)dv\qquad\text{(if $f(x,y)=f_1(x)f_2(y)$ is separable)}$$

$$f(x,y)=\frac{\partial^2}{\partial x\partial y}F(x,y)$$

$$f(x,y)=\frac{\partial}{\partial x}F_1(x)\frac{\partial}{\partial y}F_2(y)\qquad\text{(if $F(x,y)=F_1(x)F_2(y)$ is separable)}$$

$$F^{\leftarrow}(x,y)=(F_1^{-1}(x),F_2^{-1}(y))\qquad\text{(if $F(x,y)=F_1(x)F_2(y)$ is separable)}$$

## UniformSampler1D

| PDF      | CDF      | Quantile              |
|----------|----------|-----------------------|
| $f(x)=1$ | $F(x)=x$ | $F^{\leftarrow}(x)=x$ |

## TiltedSampler1D

Represents a uniform distribution tilted to favor one endpoint.

!!! abstract "Parameters"
    * Shape $k$
    * Direction: Left, Right, None (equivalent to [`UniformSampler1D`](#uniformsampler1d)).

Left-tilted:

| PDF                             | CDF                     | Quantile                                 |
|---------------------------------|-------------------------|------------------------------------------|
| $f(x)=\frac{k(k+1)}{(k+1-x)^2}$ | $F(x)=\frac{kx}{k+1-x}$ | $F^{\leftarrow}(x)=\frac{(k+1)x}{(k+x)}$ |

Right-tilted:

| PDF                           | CDF                         | Quantile                             |
|-------------------------------|-----------------------------|--------------------------------------|
| $f(x)=\frac{k(k+1)}{(k+x)^2}$ | $F(x)=\frac{(k+1)x}{(k+x)}$ | $F^{\leftarrow}(x)=\frac{kx}{k+1-x}$ |

## UniformSampler2D

| PDF        | CDF         | Quantile                    |
|------------|-------------|-----------------------------|
| $f(x,y)=1$ | $F(x,y)=xy$ | $F^{\leftarrow}(x,y)=(x,y)$ |
