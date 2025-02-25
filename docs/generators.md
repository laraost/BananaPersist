# Synthetic Data Generators

## Gaussian Random Walks

Generate a random walk where $f(x_i) = f(x_{i-1}) + \cal{N}(\mu, \sigma)$.

Pass to `--gen_args` as
```
    grw:<mean>;<sd>
```
`<mean>` is the mean of the normal distribution, `<sd>` its standard deviation.
All parameters (`<...>`) are converted to a number by `std::stod(...)`.
Parameter `<sd>` is optional.

## Linear Case for Local Maintenance

Pass to `--gen_args` as `local-wc`.

## Quasi-Periodic Functions

### Method 1 -- Sine Wave + Random Walk

$f(x) = a \cdot sin(\omega x) + R(x; \mu, \sigma)$, where $R(x; \mu, \sigma)$ is a Gaussian random walk.

Parameters:
- $a$: amplitude of the sine wave
- $\omega$: angular frequency in items; $\omega = 1/(2\pi \cdot b)$ has period $b$
- $\mu$: mean of the normal distribution of the random walk
- $\sigma$: the standard deviation of the normal distribution of the random walk

Pass to `--gen_args` as
```
    sqp:<period>;<amplitude>;<mean>;<sd>
```
All parameters (`<...>`) are converted to a number by `std::stod(...)`.
All are optional, but must be specified in the given order,
i.e., if `<mean>` is given, then `<period>` and `<amplitude>` cannot be omitted. 

### Method 2 -- Sine Wave Modulating a Random Walk

$f(x) = R(x; a \cdot sin(wx), \sigma)$, where $R(x; \mu, \sigma)$ is a Gaussian random walk.

Parameters:
- $a$: amplitude of the sine wave
- $\omega$: angular frequency in items; $\omega = 1/(2\pi \cdot b)$ has period $b$
- $\sigma$: mean of the normal distribution of the random walk as above

Pass to `--gen_args` as
```
    mqp:<number of periods>;<amplitude>;<sd>
```
All parameters (`<...>`) are converted to a number by `std::stod(...)`.
All are optional, but must be specified in the given order,
i.e., if `<sd>` is given, then `<number of periods>` and `<amplitude>` cannot be omitted. 
