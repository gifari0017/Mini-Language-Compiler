// SYNTAX ERRORS: missing semicolons / malformed statements
// Panic-mode should report several, not stop at the first.
int a = 5               // missing ;
int b = 10;
print b
int c = 20;             // the previous print missed its ;
int d = ;               // missing expression
