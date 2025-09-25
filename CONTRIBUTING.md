# Contributing to funnel Development

## Development Setup

1. **Clone the repository**

   ```bash
   git clone https://github.com/lbl-srg/funnel.git
   cd funnel
   ```

2. **Install development dependencies**

   ```bash
   pip install -e ".[dev,publish]"
   ```

3. **Install system dependencies** (for building from source)

   ```bash
   # Ubuntu/Debian
   sudo apt-get install cmake build-essential

   # macOS
   brew install cmake

   # Windows
   # Install Visual Studio Build Tools and CMake
   ```

## Commit Guidelines

We use [Commitizen](https://commitizen-tools.github.io/commitizen/) for consistent commit messages and automated versioning and changelog.

Important commits that should be considered for semantic versioning and that should appear in the changelog must include a proper commit type, such as:

- `feat`: New features
- `fix`: Bug fixes
- `refactor`: Code refactoring

A major version bump is triggered by the detection of `BREAKING CHANGE` in the commit body.
Multiple breaking changes are allowed in a single commit but they must be separate, as opposed to bullet points under a unique `BREAKING CHANGE`.

### Example

```
refactor: Modernize packaging and refactor entry point

BREAKING CHANGE: Python 2 is no longer supported.

BREAKING CHANGE: The standalone CLI script has been renamed from pyfunnel/pyfunnel.py to pyfunnel/cli.py.
```

## Development Workflow

1. **Open an issue**

1. **Create a feature branch**

   ```bash
   git checkout -b issueXX_description
   ```

1. **Make your changes** and commit using the above guidelines

1. **Build the project and test your changes**

   ```bash
   mkdir -p build && cd build
   cmake ..  # Add `-A x64` on Windows to compile in 64 bits
   cmake --build . --target install --config Release
   ctest -C Release --verbose
   ```

1. **Submit a pull request** to the `master` branch

## Release Process

Releases are automated using our CI/CD pipeline:

1. **Staging**: Changes are tested on the `staging` branch — which can be checked out and updated with new commits if needed.
2. **Master**: Production-ready releases are tagged and published to PyPI.

Contributors don't need to manage versions manually - Commitizen handles this automatically based on commit messages.
