# IncludeBuild Tests

This directory contains tests for the IncludeBuild system. These tests help verify that IncludeBuild functions correctly and can be used to detect regressions when making changes to the code.

## Running the Tests

You can run all tests with:

```bash
make test
```

Or run an individual test directly:

```bash
gcc -o test_basic tests/test_basic.c
./test_basic
```

## Test Files

- `test_basic.c` - Basic tests for IncludeBuild functionality
- `test_logging.c` - Tests for logging and error handling

## Adding New Tests

When adding new functionality to IncludeBuild, please also add tests to verify it works correctly. Tests should:

1. Be easy to run
2. Verify expected behavior
3. Include clear error messages when tests fail
4. Test both normal and error cases

Each test should use the `TEST_ASSERT` macro to verify conditions and fail with a helpful message if the condition is not met.

## Test Coverage

The current tests cover:

- Basic API functionality
- Configuration management
- Target handling

More test coverage is welcome! Please consider contributing tests for:

- Command line processing
- Dependency graph generation
- Platform-specific functionality
- Error handling 