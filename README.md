**MoCaChem: Monte Carlo Chemical Potential Estimation**

MoCaChem is a program that uses the Monte Carlo method to estimate the chemical potential of a fluid governed by Lennard-Jones potential interactions. By simulating particle fluctuations in defined sub-regions of the system's volume, it accurately calculates the thermodynamic properties of the system, providing insight into phase equilibria and molecular dynamics.

### Building the Code

This project uses [xmake](https://xmake.io/#/) as its build system. If you do not have xmake installed take a look at the [installation section](https://xmake.io/#/guide/installation) in the guide.

To build the project, simply run:

```sh
xmake build
```

### Running the Program

Once the code is built, you can run the compiled program with:

```bash
xmake run
```

Alternatively, if you want to pass arguments to the program, you can use:

```bash
xmake run MoCaChem [arguments]
```

For a list of available `[arguments]` run `xmake run MoCaChem --help`.

---

**DISCLAMER:** This project is still in development and thus neither feature complete nor tested for correctness. If you are interested there will soon be a alpha release.
