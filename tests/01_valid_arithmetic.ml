// VALID: declarations, arithmetic, operator precedence, print
int a = 2 + 3 * 4;        // 14, not 20 (precedence)
int b = (2 + 3) * 4;      // 20
float r = 10.0 / 4.0;
int m = 17 % 5;
int neg = -a;
bool t = (a < b) && (m == 2);
print a;
print b;
print r;
print t;
