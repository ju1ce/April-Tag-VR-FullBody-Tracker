.clang-format
```yaml
---
BasedOnStyle: LLVM
UseTab: Never
IndentWidth: 4
AllowShortFunctionsOnASingleLine: All
AllowShortIfStatementsOnASingleLine: AllIfsAndElse
AllowShortBlocksOnASingleLine: Always
AllowShortLambdasOnASingleLine: All
AllowShortLoopsOnASingleLine: true
BreakBeforeBraces: Allman
IndentCaseLabels: false
ColumnLimit: 0
AccessModifierOffset: -4
NamespaceIndentation: None
AlwaysBreakTemplateDeclarations: Yes
PointerAlignment: Left
ReferenceAlignment: Left
AlignAfterOpenBracket: DontAlign
ForEachMacros: ["TEST_CASE"]
SortIncludes: true
IncludeBlocks: Regroup
IncludeCategories:
  # Headers in <> without extension.
  - Regex:           '<([[:alnum:]\Q/-_\E])+>'
    Priority:        4
  # Headers in <> from specific external libraries.
  - Regex:           '<(opencv2|wx|apriltag)\/'
    Priority:        3
  # Headers in <> with extension.
  - Regex:           '<([[:alnum:].\Q/-_\E])+>'
    Priority:        2
  # Headers in "" with extension.
  - Regex:           '"([[:alnum:].\Q/-_\E])+"'
    Priority:        1
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
