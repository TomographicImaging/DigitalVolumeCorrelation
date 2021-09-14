# GitHub Actions

## Building the Conda Package: [conda_build_and_publish](https://github.com/vais-ral/CCPi-DVC/blob/master/.github/workflows/conda_build_and_publish.yml)
This github action builds and tests the conda package, by using the [conda-package-publish-action](https://github.com/paskino/conda-package-publish-action)

When pushing to master *all* variants are built and tested.

When making an [annotated](https://git-scm.com/book/en/v2/Git-Basics-Tagging) tag, *all* variants are built, tested and published to the [ccpi conda channel for ccpi-dvc](https://anaconda.org/ccpi/ccpi-dvc/files). This is only the linux version which is built with the github action. The macOS and windows versions currently have to built locally and published manually.

When opening or modifying a pull request to master, a single variant is built and tested, but not published. This variant is `python=3.7` and `numpy=1.18`.
