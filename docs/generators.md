# Synthetic Data Generators

The general format of the string passed to `--gen-args` is `<gen>:<list-of-params>`,
where `<gen>` is the name of the generator and `<list-of-params>` is a semicolon-separated list of values.
Parameters have to be provided in the given order.
Some may be optional, but if one optional parameter is omitted, all subsequent ones must be omitted, too.
Numerical parameters are converted to a number by `std::stod(...)`.

## Gaussian Random Walks

Generate a random walk where $R(x_i;\mu,\sigma) = R(x_{i-1};\mu,\sigma) + \cal{N}(\mu, \sigma)$,
with $R(0;\mu,\sigma) = 0$.
The term $\cal{N}(\mu,\sigma)$ denotes a Gaussian random variable with mean $\mu$ and standard deviation $\sigma$.

Pass to `--gen_args` as
```
    grw:<mean>;<sd>
```
`<mean>` is the mean of the normal distribution, `<sd>` its standard deviation.
Parameter `<sd>` is optional with default `1`.

## Linear Case for Local Maintenance and Topological Maintenance

Pass to `--gen_args` as
```
    <gen>:<noise>;<mean>;<sd>
```

Here, `<gen>` is either `local-wc` or `cut-wc`.
`<noise>` is the amount of noise mixed into the signal: 0 for no noise, 1 for only noise.
The noise is a Gaussian random walk with mean `<mean>` and standard deviation `<sd>`.
All parameters (except `<gen>`) are optional.

Default values are:
- `<noise>`: `0` 
- `<mean>`: `0`
- `<sd>`: `1`

The executable `generate_data` also accepts `glue-wc` for `<gen>`; this produces the same series as `cut-wc`.

## Quasi-Periodic Functions

### Method 1 -- Sine Wave + Random Walk

$f(x) = a \cdot sin(\omega x) + R(x; \mu, \sigma)$, where $R(x; \mu, \sigma)$ is a Gaussian random walk.
Items are generated for $x=0,1,\dots$.

Parameters:
- $a$: amplitude of the sine wave
- $\omega$: angular frequency in items; $\omega = 1/(2\pi \cdot b)$ has period $b$
- $\mu$: mean of the normal distribution of the random walk
- $\sigma$: the standard deviation of the normal distribution of the random walk

Pass to `--gen_args` as
```
    sqp:<period>;<amplitude>;<mean>;<sd>
```
All parameters are optional, but must be specified in the given order,
e.g., if `<mean>` is given, then `<period>` and `<amplitude>` cannot be omitted. 

Default values are:
- `<period>`: `100` 
- `<amplitude>`: `1`
- `<mean>`: `0`
- `<sd>`: `1`

### Method 2 -- Sine Wave Modulating a Random Walk

$f(x) = R(x; a \cdot sin(wx), \sigma)$, where $R(x; \mu, \sigma)$ is a Gaussian random walk.
Items are generated for $x=0,1,\dots$.

Parameters:
- $a$: amplitude of the sine wave
- $\omega$: angular frequency in items; $\omega = 1/(2\pi \cdot b)$ has period $b$
- $\sigma$: mean of the normal distribution of the random walk as above

Pass to `--gen_args` as
```
    mqp:<number of periods>;<amplitude>;<sd>
```
All are optional, but must be specified in the given order,
e.g., if `<sd>` is given, then `<number of periods>` and `<amplitude>` cannot be omitted. 

Default values are:
- `<number of periods>`: `5.5`
- `<amplitude>`: `1`
- `<sd>`: `1`

### A Note
Both of these methods do essentially the same thing.
For "historical" reasons `sqp` takes the period of the sine wave as input, while in `mqp` you select the number of periods over the whole input.
Furthermore, `sqp` allows to bias the random walk, which `mqp` does not.
