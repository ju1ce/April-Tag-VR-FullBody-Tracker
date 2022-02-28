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
        bugprone-*,
        concurrency-*,
        performance-*,
        portability-*,
        readability-*,
        modernize-*,
        cppcoreguidelines-*,
        clang-analyzer-*,
        clang-diagnostic-*,
        misc-*,
    ]
Remove:
    [
        readability-braces-around-statements,
        readability-identifier-length,
        readability-magic-numbers,
        modernize-use-trailing-return-type,
        cppcoreguidelines-non-private-member-variables-in-classes,
        cppcoreguidelines-special-member-functions,
        cppcoreguidelines-macro-usage,
        cppcoreguidelines-avoid-magic-numbers,
        misc-non-private-member-variables-in-classes,
    ]
CheckOptions:
    readability-identifier-naming.GlobalConstantCase: UPPER_CASE
    readability-identifier-naming.ClassCase: CamelCase
    readability-identifier-naming.ClassConstantCase: UPPER_CASE
    readability-identifier-naming.ConstantMemberCase: lower_case
    readability-identifier-naming.StructCase: CamelCase
    readability-identifier-naming.EnumCase: CamelCase
    readability-identifier-naming.FunctionCase: CamelCase
    readability-identifier-naming.MemberCase: lower_case
    readability-identifier-naming.ParameterCase: lower_case
    readability-identifier-naming.UnionCase: CamelCase
    readability-identifier-naming.VariableCase: lower_case
    readability-identifier-naming.GlobalVariableCase: UPPER_CASE
    readability-identifier-naming.ConstantCase: CamelCase
    readability-identifier-naming.MethodCase: CamelCase
    readability-identifier-naming.MethodIgnoredRegexp: get.*|set.*
```