# BrainBox

BrainBox is a C++ quiz app built with Qt 6 Widgets. It includes single-answer
and multiple-answer C++ quizzes.

## Requirements

- CMake 3.16 or newer
- A C++17 compiler
- Qt 6 with the Widgets component

## Build and run on Windows

From the project directory, configure and build with CMake. If Qt is installed
in the MSYS2 UCRT64 environment at `C:\msys64\ucrt64`, pass that location as
the Qt prefix path:

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH=C:/msys64/ucrt64
cmake --build build
cd build
.\BrainBox.exe
```

If CMake can already find your Qt installation, you can omit
`-DCMAKE_PREFIX_PATH=...`.

The CMake configure step copies the question JSON files into the build
directory, where the app loads them at runtime.

## Adding questions

Edit `questions_single.json` or `questions_multi.json` in the project root.
Each file contains a `questions` array. You can add as many question objects
as you like.

Single-answer questions use an `answer` field with the 1-based option number:

```json
{
  "question": "Which keyword declares a class in C++?",
  "options": ["function", "class", "object"],
  "answer": 2
}
```

Multiple-answer questions use an `answers` array of 1-based option numbers:

```json
{
  "question": "Which are C++ fundamental types?",
  "options": ["int", "class", "double"],
  "answers": [1, 3]
}
```

Keep the JSON valid and make sure each answer number refers to an existing
option. Re-run the CMake configure command after changing the question files
so they are copied into `build`.
