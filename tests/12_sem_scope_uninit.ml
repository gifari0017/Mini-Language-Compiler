// SEMANTIC ERRORS (e, f bonus): scope violation + uninitialized use
int total;              // declared, NOT initialized
int next = total + 1;   // (f) uses 'total' before it was initialized

if (next > 0) {
    int temp = 100;     // 'temp' lives only inside this block
    print temp;
}
print temp;             // (e) scope violation: 'temp' not visible here
