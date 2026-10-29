// VALID: nested scopes and shadowing
int x = 1;              // global x
print x;                // 1

if (x == 1) {
    int x = 2;          // inner x shadows global (legal, different scope)
    int y = x + 10;     // uses inner x -> 12
    print y;
}

print x;                // global x still 1 (inner x died)

do {
    int counter = 0;    // scoped to the loop body
    counter = counter + 1;
    x = x + 1;
} while (x < 3);
