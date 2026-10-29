// VALID: if/else, do/while, nesting
int n = 5;
int fact = 1;
int i = 1;

do {
    fact = fact * i;
    i = i + 1;
} while (i <= n);

if (fact > 100) {
    print "big factorial";
} else {
    if (fact == 120) {
        print "it is 120";
    } else {
        print "small";
    }
}
print fact;
