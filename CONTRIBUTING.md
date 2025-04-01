# Contributing to IncludeBuild

Thank you for considering contributing to IncludeBuild! This document outlines the process for contributing to the project.

## Code of Conduct

Please be respectful and considerate of others when contributing to this project. We welcome contributions from everyone who wishes to improve IncludeBuild.

## How to Contribute

### Reporting Bugs

1. Check if the bug has already been reported in the [issue tracker](https://github.com/includebuild/includebuild/issues).
2. If not, create a new issue with a clear title and description.
3. Include steps to reproduce the bug, expected behavior, and actual behavior.
4. Include your operating system, compiler version, and any other relevant environment information.

### Feature Requests

1. Check if the feature has already been requested in the [issue tracker](https://github.com/includebuild/includebuild/issues).
2. If not, create a new issue with a clear title and description of the feature.
3. Explain why the feature would be useful to users of IncludeBuild.

### Pull Requests

1. Fork the repository and create a new branch from `main`.
2. Make your changes in the new branch.
3. Add or update tests as necessary.
4. Ensure your code follows the existing style.
5. Submit a pull request to the `main` branch.

## Development Guidelines

### Code Style

- Use 4 spaces for indentation, not tabs.
- Follow the existing naming conventions:
  - `snake_case` for functions and variables
  - `ALL_CAPS` for macros and constants
  - Prefix all public functions and types with `ib_`
- Keep functions small and focused on a single task.
- Comment your code, especially complex parts.

### Testing

- Add tests for new features.
- Ensure all tests pass before submitting a pull request.
- Test your changes on multiple platforms if possible.

### Documentation

- Update documentation for any changed functionality.
- Document new functions, types, and macros.
- Use clear, concise language in documentation.

## Development Setup

1. Clone the repository:
   ```bash
   git clone https://github.com/includebuild/includebuild.git
   cd includebuild
   ```

2. Create a branch for your changes:
   ```bash
   git checkout -b feature/your-feature-name
   ```

3. Make your changes and test them.

4. Commit your changes:
   ```bash
   git commit -m "Add your meaningful commit message here"
   ```

5. Push your branch to your fork:
   ```bash
   git push origin feature/your-feature-name
   ```

6. Create a pull request on GitHub.

## License

By contributing to IncludeBuild, you agree that your contributions will be licensed under the project's [MIT License](LICENSE). 