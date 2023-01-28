/* This is the contribution to the `default_compilers' array in gcc.c
   for the Algol 60 language.  */

{".A60",   "@a60-cpp-input", 0, 0, 0},
{".ALG",   "@a60-cpp-input", 0, 0, 0},
{"@a60-cpp-input",
  "cc1 -E -traditional-cpp -D_ALGOL60_ %(cpp_options) \
      %{E|M|MM:%(cpp_debug_options)}\
      %{!M:%{!MM:%{!E: -o %|.a60 |\n\
    algol601 -fpreprocessed %|.a60 %(cc1_options) %{!fsyntax-only:%(invoke_as)}}}}", 0, 0, 0},

{".a60",   "@a60", 0, 0, 0},
{".alg",   "@a60", 0, 0, 0},
{"@a60",   "%{!E:%{!MM:%{!M:algol601 %i %(cc1_options) %{I*}\
             %{!fsyntax-only:%(invoke_as)}}}}\n", 0, 0, 0},
