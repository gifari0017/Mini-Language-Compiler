// LEXICAL ERRORS: illegal characters
int x = 5;
int y = x @ 3;          // '@' is illegal
int z = x # y;          // '#' is illegal
print z $;              // '$' is illegal
