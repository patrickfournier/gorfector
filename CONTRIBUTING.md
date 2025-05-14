# Contributing

If you want to contribute code, be it a new feature or a bug fix, please fork the repository and submit a
pull request. Discuss proposed changes in an issue on [GitHub](https://github.com/patrickfournier/gorfector/issues)
before starting work. This will help avoid duplicate work and ensure that the changes are in line with the 
goals of the project.

## Formatting code

**Gorfector** uses [clang-format](https://clang.llvm.org/docs/ClangFormat.html) to format the code. You will need
to install it. On Ubuntu, you can install it with the following command:

```bash
  sudo apt install clang-format
```

`git` will check the code formatting of each staged file before each commit. If the formatting is not correct, 
the commit will be aborted. You can run the following command to manually format the staged files:

```bash
    .githooks/clang_format_files.sh
```
