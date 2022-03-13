.clang-format
```yaml
---
BasedOnStyle: LLVM
UseTab: Never
IndentWidth: 4
TabWidth: 4
BreakBeforeBraces: Allman
AllowShortFunctionsOnASingleLine: true
AllowShortIfStatementsOnASingleLine: true
IndentCaseLabels: false
ColumnLimit: 0
AccessModifierOffset: -4
NamespaceIndentation: None
FixNamespaceComments: false
AlwaysBreakTemplateDeclarations: Yes
PointerAlignment: Left
ReferenceAlignment: Left
```

options for clang-tidy
```yaml
---
Add:
    [
        clang-analyzer-*,
        clang-diagnostic-*,
        bugprone-*,
        concurrency-*,
        performance-*,
        portability-*,
        modernize-*,
    ]
Remove:
    [
        modernize-use-trailing-return-type,
    ]
CheckOptions:
    readability-identifier-naming.GlobalConstantCase: UPPER_CASE
    readability-identifier-naming.VariableCase: camelBack
    readability-identifier-naming.FunctionCase: CamelCase
    readability-identifier-naming.ParameterCase: camelBack
    readability-identifier-naming.ClassCase: CamelCase
    readability-identifier-naming.MemberCase: camelBack
    readability-identifier-naming.EnumCase: CamelCase
```
