// SEMANTIC ERROR (c): type mismatches inside expressions
int a = 5;
bool f = true;
int b = a + f;          // int + bool -> invalid
int c = "text" + 3;     // string + int -> invalid
bool g = a && f;        // int && bool -> && needs two bools
int d = 10 % 2.5;       // % needs int operands
